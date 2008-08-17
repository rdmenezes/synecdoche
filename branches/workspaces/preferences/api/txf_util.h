// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2005 University of California
//
// Synecdoche is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Synecdoche is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License with Synecdoche.  If not, see <http://www.gnu.org/licenses/>.

// Interface functions for tex_info stuff.
// Contributed by Tolu Aina

#ifndef _TXF_UTIL_
#define _TXF_UTIL_

#ifdef __cplusplus
extern "C" {
#endif

extern void txf_load_fonts(char* dir);
extern void txf_render_string(
	float alpha_value,
        // reference value to which incoming alpha values are compared.
        // 0 through to 1
	double x, double y, double z, // text position
	float fscale,                 // scale factor
	GLfloat * col,                // colour 
	int i,                        // font index see texfont.h 
	char * s				  	  // string ptr
);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
