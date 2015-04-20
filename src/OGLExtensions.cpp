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

#ifndef USE_GLES
#ifdef WIN32
PFNGLACTIVETEXTUREPROC            glActiveTexture            = (PFNGLACTIVETEXTUREPROC) EmptyFunc;
#endif
PFNGLCREATESHADERPROC             glCreateShader             = (PFNGLCREATESHADERPROC) EmptyFunc;
PFNGLSHADERSOURCEPROC             glShaderSource             = (PFNGLSHADERSOURCEPROC) EmptyFunc;
PFNGLCOMPILESHADERPROC            glCompileShader            = (PFNGLCOMPILESHADERPROC) EmptyFunc;
PFNGLGETSHADERIVPROC              glGetShaderiv              = (PFNGLGETSHADERIVPROC) EmptyFunc;
PFNGLGETSHADERINFOLOGPROC         glGetShaderInfoLog         = (PFNGLGETSHADERINFOLOGPROC) EmptyFunc;
PFNGLCREATEPROGRAMPROC            glCreateProgram            = (PFNGLCREATEPROGRAMPROC) EmptyFunc;
PFNGLATTACHSHADERPROC             glAttachShader             = (PFNGLATTACHSHADERPROC) EmptyFunc;
PFNGLBINDATTRIBLOCATIONPROC       glBindAttribLocation       = (PFNGLBINDATTRIBLOCATIONPROC) EmptyFunc;
PFNGLLINKPROGRAMPROC              glLinkProgram              = (PFNGLLINKPROGRAMPROC) EmptyFunc;
PFNGLGETPROGRAMIVPROC             glGetProgramiv             = (PFNGLGETPROGRAMIVPROC) EmptyFunc;
PFNGLGETPROGRAMINFOLOGPROC        glGetProgramInfoLog        = (PFNGLGETPROGRAMINFOLOGPROC) EmptyFunc;
PFNGLDETACHSHADERPROC             glDetachShader             = (PFNGLDETACHSHADERPROC) EmptyFunc;
PFNGLGETUNIFORMLOCATIONPROC       glGetUniformLocation       = (PFNGLGETUNIFORMLOCATIONPROC) EmptyFunc;
PFNGLDELETESHADERPROC             glDeleteShader             = (PFNGLDELETESHADERPROC) EmptyFunc;
PFNGLDELETEPROGRAMPROC            glDeleteProgram            = (PFNGLDELETEPROGRAMPROC) EmptyFunc;
PFNGLISSHADERPROC                 glIsShader                 = (PFNGLISSHADERPROC) EmptyFunc;
PFNGLISPROGRAMPROC                glIsProgram                = (PFNGLISPROGRAMPROC) EmptyFunc;
PFNGLENABLEVERTEXATTRIBARRAYPROC  glEnableVertexAttribArray  = (PFNGLENABLEVERTEXATTRIBARRAYPROC) EmptyFunc;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) EmptyFunc;
PFNGLVERTEXATTRIBPOINTERPROC      glVertexAttribPointer      = (PFNGLVERTEXATTRIBPOINTERPROC) EmptyFunc;
PFNGLUNIFORM4FPROC                glUniform4f                = (PFNGLUNIFORM4FPROC) EmptyFunc;
PFNGLUNIFORM3FPROC                glUniform3f                = (PFNGLUNIFORM3FPROC) EmptyFunc;
PFNGLUNIFORM2FPROC                glUniform2f                = (PFNGLUNIFORM2FPROC) EmptyFunc;
PFNGLUNIFORM1FPROC                glUniform1f                = (PFNGLUNIFORM1FPROC) EmptyFunc;
PFNGLUNIFORM1IPROC                glUniform1i                = (PFNGLUNIFORM1IPROC) EmptyFunc;
PFNGLUSEPROGRAMPROC               glUseProgram               = (PFNGLUSEPROGRAMPROC) EmptyFunc;
#endif /* not USE_GLES */

#define INIT_ENTRY_POINT(type, funcname) \
  p##funcname = (type) CoreVideo_GL_GetProcAddress(#funcname); \
  if (p##funcname == NULL) { DebugMessage(M64MSG_WARNING, \
  "Couldn't get address of OpenGL function: '%s'", #funcname); p##funcname = (type) EmptyFunc; }

void OGLExtensions_Init(void)
{
#ifndef USE_GLES
#ifdef WIN32
    glActiveTexture            = (PFNGLACTIVETEXTUREPROC)            CoreVideo_GL_GetProcAddress("glActiveTexture");
#endif
    glCreateShader             = (PFNGLCREATESHADERPROC)             CoreVideo_GL_GetProcAddress("glCreateShader");
    glShaderSource             = (PFNGLSHADERSOURCEPROC)             CoreVideo_GL_GetProcAddress("glShaderSource");
    glCompileShader            = (PFNGLCOMPILESHADERPROC)            CoreVideo_GL_GetProcAddress("glCompileShader");
    glGetShaderiv              = (PFNGLGETSHADERIVPROC)              CoreVideo_GL_GetProcAddress("glGetShaderiv");
    glGetShaderInfoLog         = (PFNGLGETSHADERINFOLOGPROC)         CoreVideo_GL_GetProcAddress("glGetShaderInfoLog");
    glCreateProgram            = (PFNGLCREATEPROGRAMPROC)            CoreVideo_GL_GetProcAddress("glCreateProgram");
    glAttachShader             = (PFNGLATTACHSHADERPROC)             CoreVideo_GL_GetProcAddress("glAttachShader");
    glBindAttribLocation       = (PFNGLBINDATTRIBLOCATIONPROC)       CoreVideo_GL_GetProcAddress("glBindAttribLocation");
    glLinkProgram              = (PFNGLLINKPROGRAMPROC)              CoreVideo_GL_GetProcAddress("glLinkProgram");
    glGetProgramiv             = (PFNGLGETPROGRAMIVPROC)             CoreVideo_GL_GetProcAddress("glGetProgramiv");
    glGetProgramInfoLog        = (PFNGLGETPROGRAMINFOLOGPROC)        CoreVideo_GL_GetProcAddress("glGetProgramInfoLog");
    glGetUniformLocation       = (PFNGLGETUNIFORMLOCATIONPROC)       CoreVideo_GL_GetProcAddress("glGetUniformLocation");
    glDetachShader             = (PFNGLDETACHSHADERPROC)             CoreVideo_GL_GetProcAddress("glDetachShader");
    glDeleteShader             = (PFNGLDELETESHADERPROC)             CoreVideo_GL_GetProcAddress("glDeleteShader");
    glDeleteProgram            = (PFNGLDELETEPROGRAMPROC)            CoreVideo_GL_GetProcAddress("glDeleteProgram");
    glIsShader                 = (PFNGLISSHADERPROC)                 CoreVideo_GL_GetProcAddress("glIsShader");
    glIsProgram                = (PFNGLISPROGRAMPROC)                CoreVideo_GL_GetProcAddress("glIsProgram");
    glEnableVertexAttribArray  = (PFNGLENABLEVERTEXATTRIBARRAYPROC)  CoreVideo_GL_GetProcAddress("glEnableVertexAttribArray");
    glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) CoreVideo_GL_GetProcAddress("glDisableVertexAttribArray");
    glVertexAttribPointer      = (PFNGLVERTEXATTRIBPOINTERPROC)      CoreVideo_GL_GetProcAddress("glVertexAttribPointer");
    glUniform4f                = (PFNGLUNIFORM4FPROC)                CoreVideo_GL_GetProcAddress("glUniform4f");
    glUniform3f                = (PFNGLUNIFORM3FPROC)                CoreVideo_GL_GetProcAddress("glUniform3f");
    glUniform2f                = (PFNGLUNIFORM2FPROC)                CoreVideo_GL_GetProcAddress("glUniform2f");
    glUniform1f                = (PFNGLUNIFORM1FPROC)                CoreVideo_GL_GetProcAddress("glUniform1f");
    glUniform1i                = (PFNGLUNIFORM1IPROC)                CoreVideo_GL_GetProcAddress("glUniform1i");
    glUseProgram               = (PFNGLUSEPROGRAMPROC)               CoreVideo_GL_GetProcAddress("glUseProgram");
#endif /* not USE_GLES */
}


