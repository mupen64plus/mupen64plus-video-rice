/* OGLExtensions.cpp
Copyright (C) 2009 Richard Goedeken

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/* This source file contains code for assigning function pointers to some OpenGL functions */
/* This is only necessary because Windows does not contain development support for OpenGL versions beyond 1.1 */

#include "osal_opengl.h"
#include "OGLExtensions.h"
#include "Video.h"

static void APIENTRY EmptyFunc(void) { return; }

PFUNCGLACTIVETEXTUREPROC             pglActiveTexture = (PFUNCGLACTIVETEXTUREPROC) EmptyFunc;
PFUNCGLPROGRAMSTRINGARBPROC          pglProgramStringARB = (PFUNCGLPROGRAMSTRINGARBPROC) EmptyFunc;
PFUNCGLBINDPROGRAMARBPROC            pglBindProgramARB = (PFUNCGLBINDPROGRAMARBPROC) EmptyFunc;
PFUNCGLGENPROGRAMSARBPROC            pglGenProgramsARB = (PFUNCGLGENPROGRAMSARBPROC) EmptyFunc;
PFUNCGLPROGRAMENVPARAMETER4FVARBPROC pglProgramEnvParameter4fvARB = (PFUNCGLPROGRAMENVPARAMETER4FVARBPROC) EmptyFunc;
PFUNCGLFOGCOORDPOINTERPROC           pglFogCoordPointer = (PFUNCGLFOGCOORDPOINTERPROC) EmptyFunc;
PFUNCGLCLIENTACTIVETEXTUREPROC       pglClientActiveTexture = (PFUNCGLCLIENTACTIVETEXTUREPROC) EmptyFunc;
PFUNCGLCREATESHADERPROC              pglCreateShader = (PFUNCGLCREATESHADERPROC) EmptyFunc;
PFUNCGLSHADERSOURCEPROC              pglShaderSource = (PFUNCGLSHADERSOURCEPROC) EmptyFunc;
PFUNCGLCOMPILESHADERPROC             pglCompileShader = (PFUNCGLCOMPILESHADERPROC) EmptyFunc;
PFUNCGLGETSHADERIVPROC               pglGetShaderiv = (PFUNCGLGETSHADERIVPROC) EmptyFunc;
PFUNCGLGETSHADERINFOLOGPROC          pglGetShaderInfoLog = (PFUNCGLGETSHADERINFOLOGPROC) EmptyFunc;
PFUNCGLCREATEPROGRAMPROC             pglCreateProgram = (PFUNCGLCREATEPROGRAMPROC) EmptyFunc;
PFUNCGLATTACHSHADERPROC              pglAttachShader = (PFUNCGLATTACHSHADERPROC) EmptyFunc;
PFUNCGLBINDATTRIBLOCATIONPROC        pglBindAttribLocation = (PFUNCGLBINDATTRIBLOCATIONPROC) EmptyFunc;
PFUNCGLLINKPROGRAMPROC               pglLinkProgram = (PFUNCGLLINKPROGRAMPROC) EmptyFunc;
PFUNCGLGETPROGRAMIVPROC              pglGetProgramiv = (PFUNCGLGETPROGRAMIVPROC) EmptyFunc;
PFUNCGLGETPROGRAMINFOLOGPROC         pglGetProgramInfoLog = (PFUNCGLGETPROGRAMINFOLOGPROC) EmptyFunc;
PFUNCGLDETACHSHADERPROC              pglDetachShader = (PFUNCGLDETACHSHADERPROC) EmptyFunc;
PFUNCGLGETUNIFORMLOCATIONPROC        pglGetUniformLocation = (PFUNCGLGETUNIFORMLOCATIONPROC) EmptyFunc;
PFUNCGLDELETESHADERPROC              pglDeleteShader = (PFUNCGLDELETESHADERPROC) EmptyFunc;
PFUNCGLDELETEPROGRAMPROC             pglDeleteProgram = (PFUNCGLDELETEPROGRAMPROC) EmptyFunc;
PFUNCGLISSHADERPROC                  pglIsShader = (PFUNCGLISSHADERPROC) EmptyFunc;
PFUNCGLISPROGRAMPROC                 pglIsProgram = (PFUNCGLISPROGRAMPROC) EmptyFunc;
PFUNCGLENABLEVERTEXATTRIBARRAYPROC   pglEnableVertexAttribArray = (PFUNCGLENABLEVERTEXATTRIBARRAYPROC) EmptyFunc;
PFUNCGLDISABLEVERTEXATTRIBARRAYPROC  pglDisableVertexAttribArray = (PFUNCGLDISABLEVERTEXATTRIBARRAYPROC) EmptyFunc;
PFUNCGLVERTEXATTRIBPOINTERPROC       pglVertexAttribPointer = (PFUNCGLVERTEXATTRIBPOINTERPROC) EmptyFunc;
PFUNCGLUNIFORM4FPROC                 pglUniform4f = (PFUNCGLUNIFORM4FPROC) EmptyFunc;
PFUNCGLUNIFORM3FPROC                 pglUniform3f = (PFUNCGLUNIFORM3FPROC) EmptyFunc;
PFUNCGLUNIFORM2FPROC                 pglUniform2f = (PFUNCGLUNIFORM2FPROC) EmptyFunc;
PFUNCGLUNIFORM1FPROC                 pglUniform1f = (PFUNCGLUNIFORM1FPROC) EmptyFunc;
PFUNCGLUNIFORM1IPROC                 pglUniform1i = (PFUNCGLUNIFORM1IPROC) EmptyFunc;
PFUNCGLUSEPROGRAMPROC                pglUseProgram = (PFUNCGLUSEPROGRAMPROC) EmptyFunc;

#define INIT_ENTRY_POINT(type, funcname) \
  p##funcname = (type) CoreVideo_GL_GetProcAddress(#funcname); \
  if (p##funcname == NULL) { DebugMessage(M64MSG_WARNING, \
  "Couldn't get address of OpenGL function: '%s'", #funcname); p##funcname = (type) EmptyFunc; }

void OGLExtensions_Init(void)
{
    INIT_ENTRY_POINT(PFUNCGLACTIVETEXTUREPROC,             glActiveTexture);
    INIT_ENTRY_POINT(PFUNCGLPROGRAMSTRINGARBPROC,          glProgramStringARB);
    INIT_ENTRY_POINT(PFUNCGLBINDPROGRAMARBPROC,            glBindProgramARB);
    INIT_ENTRY_POINT(PFUNCGLGENPROGRAMSARBPROC,            glGenProgramsARB);
    INIT_ENTRY_POINT(PFUNCGLPROGRAMENVPARAMETER4FVARBPROC, glProgramEnvParameter4fvARB);
    INIT_ENTRY_POINT(PFUNCGLFOGCOORDPOINTERPROC,           glFogCoordPointer);
    INIT_ENTRY_POINT(PFUNCGLCLIENTACTIVETEXTUREPROC,       glClientActiveTexture);
    INIT_ENTRY_POINT(PFUNCGLCREATESHADERPROC,              glCreateShader);
    INIT_ENTRY_POINT(PFUNCGLSHADERSOURCEPROC,              glShaderSource);
    INIT_ENTRY_POINT(PFUNCGLCOMPILESHADERPROC,             glCompileShader);
    INIT_ENTRY_POINT(PFUNCGLGETSHADERIVPROC,               glGetShaderiv);
    INIT_ENTRY_POINT(PFUNCGLGETSHADERINFOLOGPROC,          glGetShaderInfoLog);
    INIT_ENTRY_POINT(PFUNCGLCREATEPROGRAMPROC,             glCreateProgram);
    INIT_ENTRY_POINT(PFUNCGLATTACHSHADERPROC,              glAttachShader);
    INIT_ENTRY_POINT(PFUNCGLBINDATTRIBLOCATIONPROC,        glBindAttribLocation);
    INIT_ENTRY_POINT(PFUNCGLLINKPROGRAMPROC,               glLinkProgram);
    INIT_ENTRY_POINT(PFUNCGLGETPROGRAMIVPROC,              glGetProgramiv);
    INIT_ENTRY_POINT(PFUNCGLGETPROGRAMINFOLOGPROC,         glGetProgramInfoLog);
    INIT_ENTRY_POINT(PFUNCGLDETACHSHADERPROC,              glDetachShader);
    INIT_ENTRY_POINT(PFUNCGLGETUNIFORMLOCATIONPROC,        glGetUniformLocation);
    INIT_ENTRY_POINT(PFUNCGLDELETESHADERPROC,              glDeleteShader);
    INIT_ENTRY_POINT(PFUNCGLDELETEPROGRAMPROC,             glDeleteProgram);
    INIT_ENTRY_POINT(PFUNCGLISSHADERPROC,                  glIsShader);
    INIT_ENTRY_POINT(PFUNCGLISPROGRAMPROC,                 glIsProgram);
    INIT_ENTRY_POINT(PFUNCGLENABLEVERTEXATTRIBARRAYPROC,   glEnableVertexAttribArray);
    INIT_ENTRY_POINT(PFUNCGLDISABLEVERTEXATTRIBARRAYPROC,  glDisableVertexAttribArray);
    INIT_ENTRY_POINT(PFUNCGLVERTEXATTRIBPOINTERPROC,       glVertexAttribPointer);
    INIT_ENTRY_POINT(PFUNCGLUNIFORM4FPROC,                 glUniform4f);
    INIT_ENTRY_POINT(PFUNCGLUNIFORM3FPROC,                 glUniform3f);
    INIT_ENTRY_POINT(PFUNCGLUNIFORM2FPROC,                 glUniform2f);
    INIT_ENTRY_POINT(PFUNCGLUNIFORM1FPROC,                 glUniform1f);
    INIT_ENTRY_POINT(PFUNCGLUNIFORM1IPROC,                 glUniform1i);
    INIT_ENTRY_POINT(PFUNCGLUSEPROGRAMPROC,                glUseProgram);
}


