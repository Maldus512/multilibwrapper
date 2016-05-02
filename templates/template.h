$warning

/*
*   template.h: template file
*
*   Copyright 2016 Mattia Maldini 
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License along
*   with this program; if not, write to the Free Software Foundation, Inc.,
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


/************************
*   Specific functions  *
*************************/

#ifndef __${LIBNAME}_WRAPPER_H__
#define __${LIBNAME}_WRAPPER_H__

int add_${libname}_handle();
int current_${libname}_handle();
int switch_${libname}_handle(int ld);
int switch_${libname}_default_handle(int ld);

#endif
