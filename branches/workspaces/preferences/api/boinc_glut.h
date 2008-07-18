// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef H_BOINC_GLUT
#define H_BOINC_GLUT


#if defined(_WIN32)
#if defined(HAVE_GL_GLUT_H)
#  include <GL/glut.h>
#else 
#  include<glut.h>
#endif

#elif defined(__APPLE_CC__)

#  include "GLUT/glut.h"

#else // !_WIN32, !__APPLE_CC__

#include "config.h"

#  if defined(HAVE_GLUT_H)
#    include "glut.h"
#  elif defined(HAVE_GL_GLUT_H)
#    include <GL/glut.h>
#  elif defined(HAVE_OPENGL_GLUT_H)
#    include <OpenGL/glut.h>
#  elif defined(HAVE_GLUT_GLUT_H)
#    include <GLUT/glut.h>
#  endif

#endif // _WIN32

#endif
