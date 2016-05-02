#define _GNU_SOURCE

#include<stdio.h>
#include<fcntl.h>
#include <unistd.h>
#include<string.h>
#include<dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "pico_stack.h"
#include "pico_dev_vde.h"
#include "pico_socket.h"
#include "counter.h"
#include <pthread.h>

#include "libpicotcp-wrapper.h"

#define INBUFSIZE 1024
#define BUFSIZE 1024

struct pico_socket *pico_socket_accept(struct pico_socket *s, void *orig, uint16_t *port);

struct client {
	size_t inbufpos;
	size_t outbufpos;
	size_t outbuflen;
	size_t outheaderlen;
	char inbuf[INBUFSIZE];
	char *header;
	char *outbuf;
};

struct args {
    int argc;
    char **argv;
};

static char *ishtml="Content-Type: text/html\r\n";

static char *headmsg=
"HTTP/1.1 %d OK\r\n"
"%s"
"Content-Length: %d\r\n\r\n";

static char *messagein= "<HTML>\n<HEAD>\n<TITLE>PICOWEBSERVER NETJECT</TITLE>\n</HEAD>\n<BODY>\n<H1>%s</H1>\n";
static char *messageout= "</BODY></HTML>\n";

int phttps_senddir(FILE *f, char *path, char *webpath) {
	int i,n;
	struct dirent **namelist;
	//printf("phttps_senddir %s %s\n", path, webpath);
	fprintf(f,messagein,(*webpath)?webpath:"/ (wrappedroot)");
	if (path == webpath) {
		char *lastslash=strrchr(webpath,'/');
		if (lastslash) {
			*lastslash=0;
			fprintf(f,"<p><a href=\"/%s\"> .. </a></p>\n",webpath);
			*lastslash='/';
		} else {
			fprintf(f,"<p><a href=\"/\"> .. </a></p>\n");
		}
	}
	n=scandir(path, &namelist, NULL, alphasort);
	for (i=0; i<n;i++) {
		if (namelist[i]->d_name[0] != '.') {
			fprintf(f,"<p><a href=\"%s/%s\"> %s </a></p>\n",
					webpath,namelist[i]->d_name,namelist[i]->d_name);
		}
	}
	fprintf(f,"%s",messageout);
	return 200;
}

int phttps_sendfile(FILE *f, char *path) {
	//printf("phttps_sendfile %s\n", path);
	char buf[BUFSIZE];
	int infd=open(path,O_RDONLY);
	if (infd > 0) {
		int n;
		struct stat statbuf;
		fstat(infd,&statbuf);
		while ((n=read(infd,buf,BUFSIZE))>0)
			fwrite(buf,n,1,f);
		return 200;
	}
	return 500;
}

int phttps_senderr(FILE *f)
{
	char *msg,*header;
	size_t size;
	fprintf(f,messagein,"404: File not Found");
	fprintf(f,"%s",messageout);
	return 404;
}

int httpparse(struct client *cl) {
	char *req=cl->inbuf;
	char *end;
	if (strncmp(req,"GET /",5)==0 && (end=strchr(req+4,' ')) != NULL) { 
		char *html=ishtml;
		int code;
		struct stat sbuf;
		FILE *f=open_memstream(&cl->outbuf,&cl->outbuflen);
		*end=0;
		if (req[5]==0)
			code=phttps_senddir(f,".",req+5);
		else if (strstr(req+4,"/../") == NULL && stat(req+5,&sbuf)==0) {
			if (S_ISREG(sbuf.st_mode)) {
				code=phttps_sendfile(f,req+5);
				if (strlen(req)<=5 || strcmp(req+(strlen(req)-5),".html") != 0)
					html="";
			} else if (S_ISDIR(sbuf.st_mode))
				code=phttps_senddir(f,req+5,req+5);
		} else
			phttps_senderr(f);
		fclose(f);
		asprintf(&cl->header,headmsg,code,html,cl->outbuflen);
	} else
		return 1;
	return 0;
}

void wakeup(uint16_t ev, struct pico_socket *s)
{
	struct client *cl=(struct client *)(s->priv);
	if (ev & PICO_SOCK_EV_CONN) {
		struct pico_socket *cs;
		union pico_address claddr;
		uint16_t clport;
		cs=pico_socket_accept(s,&claddr,&clport);
		if (cs) {
			cl = calloc(1,sizeof(struct client));
			if (cl) 
				cs->priv=cl;
			else {
				pico_socket_close(cs);
				return;
			}
		}
	}
	if (ev & PICO_SOCK_EV_RD) {
		ssize_t n=INBUFSIZE-cl->inbufpos-1;
		if (n > 0) {
			n=pico_socket_read(s,cl->inbuf+cl->inbufpos,n);
			if (n<0) goto terminate;
			cl->inbufpos += n;
			cl->inbuf[cl->inbufpos]=0;
		} else
			goto terminate;
		if (strstr(cl->inbuf,"\r\n\r\n") != NULL) {
			if (httpparse(cl))
				goto terminate;
		}
	}
	if (ev & PICO_SOCK_EV_WR) {
		if (cl->header) {
			ssize_t n=strlen(cl->header);
			ssize_t m=pico_socket_write (s, cl->header, n);
			if (n != m) goto terminate;
			free(cl->header);
			cl->header=NULL;
		}
		if (cl->outbuflen > 0) {
			ssize_t n=cl->outbuflen - cl->outbufpos;
			if (n == 0) goto terminate;
			if (n > INBUFSIZE) n=INBUFSIZE;
			n = pico_socket_write (s, cl->outbuf+cl->outbufpos,n);
			if (n < 0) goto terminate;
			cl->outbufpos += n;
		}
	}
	if (ev & PICO_SOCK_EV_CLOSE) {
		goto terminate;
	}
	if (ev & PICO_SOCK_EV_FIN) {
		//printf("FIN %p %p\n",s,cl);
		if (cl) {
			if (cl->outbuf) free(cl->outbuf);
			if (cl->header) free(cl->header);
			free(cl);
			s->priv=NULL;
		}
	}
	return;
terminate:
	pico_socket_close(s);
}

void *pico_main(void *args)
{
	struct pico_device *pico_dev;
	struct pico_ip4 my_ip, netmask;
	uint16_t port=80;
        char *device;
        int argc, ret, x;
        char **argv;
        argv = ((struct args*) args)->argv;
        argc = ((struct args*) args)->argc;
	if (argc < 4) {
		fprintf(stderr, "usage: vdesocket address mask [port] [defroute]\n");
                ret = 1;
		pthread_exit(&ret);
	}
	if (argc > 4)
		port=atoi(argv[4]);

	port=short_be(port);

        x = count();
        asprintf(&device, "vd%i", x);

	unsigned char macaddr[6]={0x0, 0x0, 0x0, 0xa, 0xb, 0xc};
	macaddr[4] ^= (uint8_t)(pthread_self() >> 8);
	macaddr[5] ^= (uint8_t) (pthread_self() & 0xFF);

        add_picotcp_handle();

	pico_string_to_ipv4(argv[2], &my_ip.addr);
	pico_string_to_ipv4(argv[3], &netmask.addr);

	pico_stack_init();

	pico_dev = (struct pico_device *) pico_vde_create(argv[1], device, macaddr);

	pico_ipv4_link_add(pico_dev, my_ip, netmask);
	if (argc > 5) {
		struct pico_ip4 defaddr, gateway;
		pico_string_to_ipv4("0.0.0.0", &defaddr.addr);
		pico_string_to_ipv4(argv[5], &gateway.addr);
		pico_ipv4_route_add(defaddr,defaddr,gateway,1,NULL);
	}

	struct pico_socket *s;
	s = pico_socket_open(PICO_PROTO_IPV4, PICO_PROTO_TCP, wakeup);

	pico_socket_bind(s, &my_ip.addr, &port);
	pico_socket_listen(s, 3);

	while(1) {
		pico_stack_tick();
		usleep(2000);
	}
}

int parseargs(char *buf, char**args){
    int ret, i;
    char *aux, *arg;
    arg = aux = buf;
    ret = 0;
    while (*aux != 0 && *aux != '\n'){
        i = 0;
        while (*aux == ' ')
            aux++;
        arg = aux;

        while (*aux != ' ' && *aux != 0 && *aux != '\n'){
            i++;
            aux++;
        }

        if (args[ret] == NULL)
            args[ret] = (char*) malloc(sizeof(char)*(i+1));
        
        strncpy(args[ret], arg, i);
        args[ret][i] = 0;
        ret++;
    }
    args[ret] = NULL;
    return ret;
}


int main(int argc, char* argv[]){
    struct args* new_args;
    char buf[100];
    pthread_t thread;
    int i, n;
    new_args = (struct args*) malloc(sizeof(struct args));
    new_args->argv = (char**) malloc(sizeof(char*)*7);
    new_args->argv[0] = (char*) malloc(sizeof(char)*32);
    strcpy(new_args->argv[0], argv[0]);

    while ( i = read(STDIN_FILENO, buf, 100)) {
        buf[i] = 0;
        n = parseargs(buf, new_args->argv+1)+1;
        new_args->argc = n;
        pthread_create(&thread, NULL, pico_main, new_args);
    }
    return 0;
}
