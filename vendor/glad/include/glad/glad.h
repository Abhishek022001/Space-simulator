/**
 * glad.h — Minimal OpenGL 4.1 Core loader
 * Hand-crafted for SpaceSim project. Covers all GL functions used across all phases.
 * Backend: GLFW (gladLoadGLLoader via glfwGetProcAddress).
 */
#ifndef GLAD_H
#define GLAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <KHR/khrplatform.h>

/* APIENTRY must be defined before any PFNGL* typedefs that use it */
#ifndef APIENTRY
  #ifdef _WIN32
    #define APIENTRY __stdcall
  #else
    #define APIENTRY
  #endif
#endif

/* -------------------------------------------------------------------------
 * GL base types
 * ---------------------------------------------------------------------- */
typedef unsigned int  GLenum;
typedef unsigned char GLboolean;
typedef unsigned int  GLbitfield;
typedef void          GLvoid;
typedef khronos_int8_t   GLbyte;
typedef khronos_uint8_t  GLubyte;
typedef khronos_int16_t  GLshort;
typedef khronos_uint16_t GLushort;
typedef int           GLint;
typedef unsigned int  GLuint;
typedef khronos_int32_t  GLclampx;
typedef int           GLsizei;
typedef float         GLfloat;
typedef float         GLclampf;
typedef double        GLdouble;
typedef double        GLclampd;
typedef khronos_intptr_t  GLintptr;
typedef khronos_ssize_t   GLsizeiptr;
typedef khronos_int64_t   GLint64;
typedef khronos_uint64_t  GLuint64;
typedef char          GLchar;
typedef unsigned short GLhalf;

/* -------------------------------------------------------------------------
 * GL constants (subset used by this project)
 * ---------------------------------------------------------------------- */
#define GL_FALSE                          0
#define GL_TRUE                           1
#define GL_NONE                           0

/* Clears */
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_COLOR_BUFFER_BIT               0x00004000

/* Primitives */
#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005

/* Data types */
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406

/* Blend */
#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_FUNC_ADD                       0x8006

/* Enables */
#define GL_CULL_FACE                      0x0B44
#define GL_DEPTH_TEST                     0x0B71
#define GL_BLEND                          0x0BE2
#define GL_LINE_SMOOTH                    0x0B20
#define GL_PROGRAM_POINT_SIZE             0x8642

/* Depth */
#define GL_LESS                           0x0201
#define GL_LEQUAL                         0x0203

/* Textures */
#define GL_TEXTURE_2D                     0x0DE1
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803
#define GL_REPEAT                         0x2901
#define GL_LINEAR                         0x2601
#define GL_LINEAR_MIPMAP_LINEAR           0x2703
#define GL_CLAMP_TO_EDGE                  0x812F

/* Shaders */
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_INFO_LOG_LENGTH                0x8B84

/* Buffers */
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_UNIFORM_BUFFER                 0x8A11
#define GL_STATIC_DRAW                    0x88B4
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_STREAM_DRAW                    0x88E0

/* Framebuffers */
#define GL_FRAMEBUFFER                    0x8D40
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_DEPTH_ATTACHMENT               0x8D00

/* Misc */
#define GL_NO_ERROR                       0
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C

/* -------------------------------------------------------------------------
 * GL 1.x / 2.x functions available directly from opengl32.dll on Windows.
 * We still declare them here so callers include only this header.
 * ---------------------------------------------------------------------- */
extern void  glViewport(GLint x, GLint y, GLsizei w, GLsizei h);
extern void  glClear(GLbitfield mask);
extern void  glClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
extern void  glEnable(GLenum cap);
extern void  glDisable(GLenum cap);
extern void  glDepthFunc(GLenum func);
extern void  glBlendFunc(GLenum src, GLenum dst);
extern void  glBlendEquation(GLenum mode);
extern void  glLineWidth(GLfloat width);
extern void  glPointSize(GLfloat size);
extern void  glDrawArrays(GLenum mode, GLint first, GLsizei count);
extern void  glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices);
extern void  glGenTextures(GLsizei n, GLuint* textures);
extern void  glDeleteTextures(GLsizei n, const GLuint* textures);
extern void  glBindTexture(GLenum target, GLuint texture);
extern void  glTexImage2D(GLenum target, GLint level, GLint internalformat,
                          GLsizei width, GLsizei height, GLint border,
                          GLenum format, GLenum type, const void* data);
extern void  glTexParameteri(GLenum target, GLenum pname, GLint param);
extern const GLubyte* glGetString(GLenum name);
extern GLenum glGetError(void);

/* -------------------------------------------------------------------------
 * Modern GL function pointers (OpenGL 3.3+ Core — must be loaded at runtime)
 * ---------------------------------------------------------------------- */

/* Shaders */
typedef GLuint (APIENTRY* PFNGLCREATESHADERPROC)(GLenum type);
typedef void   (APIENTRY* PFNGLDELETESHADERPROC)(GLuint shader);
typedef void   (APIENTRY* PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar* const* str, const GLint* length);
typedef void   (APIENTRY* PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void   (APIENTRY* PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint* params);
typedef void   (APIENTRY* PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef GLuint (APIENTRY* PFNGLCREATEPROGRAMPROC)(void);
typedef void   (APIENTRY* PFNGLDELETEPROGRAMPROC)(GLuint program);
typedef void   (APIENTRY* PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void   (APIENTRY* PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void   (APIENTRY* PFNGLUSEPROGRAMPROC)(GLuint program);
typedef void   (APIENTRY* PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint* params);
typedef void   (APIENTRY* PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef GLint  (APIENTRY* PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar* name);

/* Uniforms */
typedef void (APIENTRY* PFNGLUNIFORM1IPROC)(GLint loc, GLint v0);
typedef void (APIENTRY* PFNGLUNIFORM1FPROC)(GLint loc, GLfloat v0);
typedef void (APIENTRY* PFNGLUNIFORM2FPROC)(GLint loc, GLfloat v0, GLfloat v1);
typedef void (APIENTRY* PFNGLUNIFORM2FVPROC)(GLint loc, GLsizei count, const GLfloat* val);
typedef void (APIENTRY* PFNGLUNIFORM3FPROC)(GLint loc, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void (APIENTRY* PFNGLUNIFORM3FVPROC)(GLint loc, GLsizei count, const GLfloat* val);
typedef void (APIENTRY* PFNGLUNIFORM4FPROC)(GLint loc, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void (APIENTRY* PFNGLUNIFORM4FVPROC)(GLint loc, GLsizei count, const GLfloat* val);
typedef void (APIENTRY* PFNGLUNIFORMMATRIX3FVPROC)(GLint loc, GLsizei count, GLboolean transpose, const GLfloat* val);
typedef void (APIENTRY* PFNGLUNIFORMMATRIX4FVPROC)(GLint loc, GLsizei count, GLboolean transpose, const GLfloat* val);

/* VAO / VBO */
typedef void (APIENTRY* PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint* arrays);
typedef void (APIENTRY* PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint* arrays);
typedef void (APIENTRY* PFNGLBINDVERTEXARRAYPROC)(GLuint array);
typedef void (APIENTRY* PFNGLGENBUFFERSPROC)(GLsizei n, GLuint* buffers);
typedef void (APIENTRY* PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint* buffers);
typedef void (APIENTRY* PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void (APIENTRY* PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
typedef void (APIENTRY* PFNGLBUFFERSUBDATAPROC)(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
typedef void (APIENTRY* PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
typedef void (APIENTRY* PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void (APIENTRY* PFNGLDISABLEVERTEXATTRIBARRAYPROC)(GLuint index);

/* Textures (modern) */
typedef void (APIENTRY* PFNGLACTIVETEXTUREPROC)(GLenum texture);
typedef void (APIENTRY* PFNGLGENERATEMIPMAPPROC)(GLenum target);

/* Framebuffers */
typedef void   (APIENTRY* PFNGLGENFRAMEBUFFERSPROC)(GLsizei n, GLuint* ids);
typedef void   (APIENTRY* PFNGLBINDFRAMEBUFFERPROC)(GLenum target, GLuint framebuffer);
typedef void   (APIENTRY* PFNGLFRAMEBUFFERTEXTURE2DPROC)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef GLenum (APIENTRY* PFNGLCHECKFRAMEBUFFERSTATUSPROC)(GLenum target);

/* Declare extern function pointer variables */
extern PFNGLCREATESHADERPROC            glCreateShader;
extern PFNGLDELETESHADERPROC            glDeleteShader;
extern PFNGLSHADERSOURCEPROC            glShaderSource;
extern PFNGLCOMPILESHADERPROC           glCompileShader;
extern PFNGLGETSHADERIVPROC             glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC        glGetShaderInfoLog;
extern PFNGLCREATEPROGRAMPROC           glCreateProgram;
extern PFNGLDELETEPROGRAMPROC           glDeleteProgram;
extern PFNGLATTACHSHADERPROC            glAttachShader;
extern PFNGLLINKPROGRAMPROC             glLinkProgram;
extern PFNGLUSEPROGRAMPROC              glUseProgram;
extern PFNGLGETPROGRAMIVPROC            glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC       glGetProgramInfoLog;
extern PFNGLGETUNIFORMLOCATIONPROC      glGetUniformLocation;

extern PFNGLUNIFORM1IPROC               glUniform1i;
extern PFNGLUNIFORM1FPROC               glUniform1f;
extern PFNGLUNIFORM2FPROC               glUniform2f;
extern PFNGLUNIFORM2FVPROC              glUniform2fv;
extern PFNGLUNIFORM3FPROC               glUniform3f;
extern PFNGLUNIFORM3FVPROC              glUniform3fv;
extern PFNGLUNIFORM4FPROC               glUniform4f;
extern PFNGLUNIFORM4FVPROC              glUniform4fv;
extern PFNGLUNIFORMMATRIX3FVPROC        glUniformMatrix3fv;
extern PFNGLUNIFORMMATRIX4FVPROC        glUniformMatrix4fv;

extern PFNGLGENVERTEXARRAYSPROC         glGenVertexArrays;
extern PFNGLDELETEVERTEXARRAYSPROC      glDeleteVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC         glBindVertexArray;
extern PFNGLGENBUFFERSPROC              glGenBuffers;
extern PFNGLDELETEBUFFERSPROC           glDeleteBuffers;
extern PFNGLBINDBUFFERPROC              glBindBuffer;
extern PFNGLBUFFERDATAPROC              glBufferData;
extern PFNGLBUFFERSUBDATAPROC           glBufferSubData;
extern PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;

extern PFNGLACTIVETEXTUREPROC           glActiveTexture;
extern PFNGLGENERATEMIPMAPPROC          glGenerateMipmap;

extern PFNGLGENFRAMEBUFFERSPROC         glGenFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC         glBindFramebuffer;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC    glFramebufferTexture2D;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC  glCheckFramebufferStatus;

/* -------------------------------------------------------------------------
 * Loader — call ONCE after glfwMakeContextCurrent()
 * Returns 1 on success, 0 on failure.
 * ---------------------------------------------------------------------- */
typedef void* (*GLADloadproc)(const char* name);
int gladLoadGLLoader(GLADloadproc load);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* GLAD_H */
