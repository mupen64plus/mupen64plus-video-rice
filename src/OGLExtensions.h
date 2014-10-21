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

#include <SDL_opengl.h>

/* Just call this one function to load up the function pointers. */
void OGLExtensions_Init(void);

/* The function pointer types are defined here because as of 2009 some OpenGL drivers under Linux do 'incorrect' things which
   mess up the SDL_opengl.h header, resulting in no function pointer typedefs at all, and thus compilation errors.
*/
typedef void      (APIENTRYP PFUNCGLACTIVETEXTUREPROC) (GLenum texture);
typedef void      (APIENTRYP PFUNCGLDELETEPROGRAMSARBPROC) (GLsizei n, const GLuint *programs);
typedef void      (APIENTRYP PFUNCGLPROGRAMSTRINGARBPROC) (GLenum target, GLenum format, GLsizei len, const GLvoid *string);
typedef void      (APIENTRYP PFUNCGLBINDPROGRAMARBPROC) (GLenum target, GLuint program);
typedef void      (APIENTRYP PFUNCGLGENPROGRAMSARBPROC) (GLsizei n, GLuint *programs);
typedef void      (APIENTRYP PFUNCGLPROGRAMENVPARAMETER4FVARBPROC) (GLenum target, GLuint index, const GLfloat *params);
typedef void      (APIENTRYP PFUNCGLFOGCOORDPOINTERPROC) (GLenum type, GLsizei stride, const GLvoid *pointer);
typedef void      (APIENTRYP PFUNCGLCLIENTACTIVETEXTUREPROC) (GLenum texture);
typedef GLuint    (APIENTRYP PFUNCGLCREATESHADERPROC) (GLenum type);
typedef void      (APIENTRYP PFUNCGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar** strings, const GLint* lengths);
typedef void      (APIENTRYP PFUNCGLCOMPILESHADERPROC) (GLuint shader);
typedef void      (APIENTRYP PFUNCGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint* param);
typedef void      (APIENTRYP PFUNCGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef GLuint    (APIENTRYP PFUNCGLCREATEPROGRAMPROC) (void);
typedef void      (APIENTRYP PFUNCGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void      (APIENTRYP PFUNCGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar* name);
typedef void      (APIENTRYP PFUNCGLLINKPROGRAMPROC) (GLuint program);
typedef void      (APIENTRYP PFUNCGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint* param);
typedef void      (APIENTRYP PFUNCGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void      (APIENTRYP PFUNCGLDETACHSHADERPROC) (GLuint program, GLuint shader);
typedef GLint     (APIENTRYP PFUNCGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar* name);
typedef void      (APIENTRYP PFUNCGLDELETESHADERPROC) (GLuint shader);
typedef void      (APIENTRYP PFUNCGLDELETEPROGRAMPROC) (GLuint program);
typedef GLboolean (APIENTRYP PFUNCGLISSHADERPROC) (GLuint shader);
typedef GLboolean (APIENTRYP PFUNCGLISPROGRAMPROC) (GLuint program);
typedef void      (APIENTRYP PFUNCGLENABLEVERTEXATTRIBARRAYPROC) (GLuint);
typedef void      (APIENTRYP PFUNCGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint);
typedef void      (APIENTRYP PFUNCGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
typedef void      (APIENTRYP PFUNCGLUNIFORM4FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void      (APIENTRYP PFUNCGLUNIFORM3FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void      (APIENTRYP PFUNCGLUNIFORM2FPROC) (GLint location, GLfloat v0, GLfloat v1);
typedef void      (APIENTRYP PFUNCGLUNIFORM1FPROC) (GLint location, GLfloat v0);
typedef void      (APIENTRYP PFUNCGLUNIFORM1IPROC) (GLint location, GLint v0);
typedef void      (APIENTRYP PFUNCGLUSEPROGRAMPROC) (GLuint program);

extern PFUNCGLACTIVETEXTUREPROC             pglActiveTexture;
extern PFUNCGLPROGRAMSTRINGARBPROC          pglProgramStringARB;
extern PFUNCGLBINDPROGRAMARBPROC            pglBindProgramARB;
extern PFUNCGLGENPROGRAMSARBPROC            pglGenProgramsARB;
extern PFUNCGLPROGRAMENVPARAMETER4FVARBPROC pglProgramEnvParameter4fvARB;
extern PFUNCGLFOGCOORDPOINTERPROC           pglFogCoordPointer;
extern PFUNCGLCLIENTACTIVETEXTUREPROC       pglClientActiveTexture;
extern PFUNCGLCREATESHADERPROC              pglCreateShader;
extern PFUNCGLSHADERSOURCEPROC              pglShaderSource;
extern PFUNCGLCOMPILESHADERPROC             pglCompileShader;
extern PFUNCGLGETSHADERIVPROC               pglGetShaderiv;
extern PFUNCGLGETSHADERINFOLOGPROC          pglGetShaderInfoLog;
extern PFUNCGLCREATEPROGRAMPROC             pglCreateProgram;
extern PFUNCGLATTACHSHADERPROC              pglAttachShader;
extern PFUNCGLBINDATTRIBLOCATIONPROC        pglBindAttribLocation;
extern PFUNCGLLINKPROGRAMPROC               pglLinkProgram;
extern PFUNCGLGETPROGRAMIVPROC              pglGetProgramiv;
extern PFUNCGLGETPROGRAMINFOLOGPROC         pglGetProgramInfoLog;
extern PFUNCGLDETACHSHADERPROC              pglDetachShader;
extern PFUNCGLGETUNIFORMLOCATIONPROC        pglGetUniformLocation;
extern PFUNCGLDELETESHADERPROC              pglDeleteShader;
extern PFUNCGLDELETEPROGRAMPROC             pglDeleteProgram;
extern PFUNCGLISSHADERPROC                  pglIsShader;
extern PFUNCGLISPROGRAMPROC                 pglIsProgram;
extern PFUNCGLENABLEVERTEXATTRIBARRAYPROC   pglEnableVertexAttribArray;
extern PFUNCGLDISABLEVERTEXATTRIBARRAYPROC  pglDisableVertexAttribArray;
extern PFUNCGLVERTEXATTRIBPOINTERPROC       pglVertexAttribPointer;
extern PFUNCGLUNIFORM4FPROC                 pglUniform4f;
extern PFUNCGLUNIFORM3FPROC                 pglUniform3f;
extern PFUNCGLUNIFORM2FPROC                 pglUniform2f;
extern PFUNCGLUNIFORM1FPROC                 pglUniform1f;
extern PFUNCGLUNIFORM1IPROC                 pglUniform1i;
extern PFUNCGLUSEPROGRAMPROC                pglUseProgram;


#endif  // OGL_EXTENSIONS_H

