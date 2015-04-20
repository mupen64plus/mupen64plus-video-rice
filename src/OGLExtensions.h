/* OGLExtensions.h
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

/* This header file contains function pointers to some OpenGL functions */
/* This is only necessary because Windows does not contain development support for OpenGL versions beyond 1.1 */

#if !defined(OGL_EXTENSIONS_H)
#define OGL_EXTENSIONS_H

#include "osal_opengl.h"

/* Just call this one function to load up the function pointers. */
void OGLExtensions_Init(void);

/* The function pointer types are defined here because as of 2009 some OpenGL drivers under Linux do 'incorrect' things which
   mess up the SDL_opengl.h header, resulting in no function pointer typedefs at all, and thus compilation errors.
*/
#ifndef USE_GLES
#if defined(WIN32)
typedef void      (APIENTRYP PFNGLACTIVETEXTUREPROC) (GLenum texture);
#endif
/*typedef GLuint    (APIENTRYP PFNGLCREATESHADERPROC) (GLenum type);
typedef void      (APIENTRYP PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar** strings, const GLint* lengths);
typedef void      (APIENTRYP PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef void      (APIENTRYP PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint* param);
typedef void      (APIENTRYP PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef GLuint    (APIENTRYP PFNGLCREATEPROGRAMPROC) (void);
typedef void      (APIENTRYP PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void      (APIENTRYP PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar* name);
typedef void      (APIENTRYP PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void      (APIENTRYP PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint* param);
typedef void      (APIENTRYP PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void      (APIENTRYP PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
typedef GLint     (APIENTRYP PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar* name);
typedef void      (APIENTRYP PFNGLDELETESHADERPROC) (GLuint shader);
typedef void      (APIENTRYP PFNGLDELETEPROGRAMPROC) (GLuint program);
typedef GLboolean (APIENTRYP PFNGLISSHADERPROC) (GLuint shader);
typedef GLboolean (APIENTRYP PFNGLISPROGRAMPROC) (GLuint program);
typedef void      (APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint);
typedef void      (APIENTRYP PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint);
typedef void      (APIENTRYP PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
typedef void      (APIENTRYP PFNGLUNIFORM4FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void      (APIENTRYP PFNGLUNIFORM3FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void      (APIENTRYP PFNGLUNIFORM2FPROC) (GLint location, GLfloat v0, GLfloat v1);
typedef void      (APIENTRYP PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
typedef void      (APIENTRYP PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
typedef void      (APIENTRYP PFNGLUSEPROGRAMPROC) (GLuint program);*/
#endif /* not USE_GLES */

#ifndef USE_GLES
#ifdef WIN32
extern PFNGLACTIVETEXTUREPROC             glActiveTexture;
#endif /* WIN32 */
extern PFNGLCREATESHADERPROC              glCreateShader;
extern PFNGLSHADERSOURCEPROC              glShaderSource;
extern PFNGLCOMPILESHADERPROC             glCompileShader;
extern PFNGLGETSHADERIVPROC               glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC          glGetShaderInfoLog;
extern PFNGLCREATEPROGRAMPROC             glCreateProgram;
extern PFNGLATTACHSHADERPROC              glAttachShader;
extern PFNGLBINDATTRIBLOCATIONPROC        glBindAttribLocation;
extern PFNGLLINKPROGRAMPROC               glLinkProgram;
extern PFNGLGETPROGRAMIVPROC              glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC         glGetProgramInfoLog;
extern PFNGLDETACHSHADERPROC              glDetachShader;
extern PFNGLGETUNIFORMLOCATIONPROC        glGetUniformLocation;
extern PFNGLDELETESHADERPROC              glDeleteShader;
extern PFNGLDELETEPROGRAMPROC             glDeleteProgram;
extern PFNGLISSHADERPROC                  glIsShader;
extern PFNGLISPROGRAMPROC                 glIsProgram;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC   glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC  glDisableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC       glVertexAttribPointer;
extern PFNGLUNIFORM4FPROC                 glUniform4f;
extern PFNGLUNIFORM3FPROC                 glUniform3f;
extern PFNGLUNIFORM2FPROC                 glUniform2f;
extern PFNGLUNIFORM1FPROC                 glUniform1f;
extern PFNGLUNIFORM1IPROC                 glUniform1i;
extern PFNGLUSEPROGRAMPROC                glUseProgram;
extern PFNGLUSEPROGRAMPROC                glUseProgram;
#endif /* not USE_GLES */


#endif  // OGL_EXTENSIONS_H

