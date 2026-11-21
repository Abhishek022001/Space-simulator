/**
 * glad.c — Minimal OpenGL 4.1 Core function loader for SpaceSim.
 * Uses GLFW's glfwGetProcAddress as the proc loader backend.
 */
#include "glad/glad.h"
#include <stdio.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Function pointer definitions
 * ---------------------------------------------------------------------- */
PFNGLCREATESHADERPROC            glCreateShader            = NULL;
PFNGLDELETESHADERPROC            glDeleteShader            = NULL;
PFNGLSHADERSOURCEPROC            glShaderSource            = NULL;
PFNGLCOMPILESHADERPROC           glCompileShader           = NULL;
PFNGLGETSHADERIVPROC             glGetShaderiv             = NULL;
PFNGLGETSHADERINFOLOGPROC        glGetShaderInfoLog        = NULL;
PFNGLCREATEPROGRAMPROC           glCreateProgram           = NULL;
PFNGLDELETEPROGRAMPROC           glDeleteProgram           = NULL;
PFNGLATTACHSHADERPROC            glAttachShader            = NULL;
PFNGLLINKPROGRAMPROC             glLinkProgram             = NULL;
PFNGLUSEPROGRAMPROC              glUseProgram              = NULL;
PFNGLGETPROGRAMIVPROC            glGetProgramiv            = NULL;
PFNGLGETPROGRAMINFOLOGPROC       glGetProgramInfoLog       = NULL;
PFNGLGETUNIFORMLOCATIONPROC      glGetUniformLocation      = NULL;

PFNGLUNIFORM1IPROC               glUniform1i               = NULL;
PFNGLUNIFORM1FPROC               glUniform1f               = NULL;
PFNGLUNIFORM2FPROC               glUniform2f               = NULL;
PFNGLUNIFORM2FVPROC              glUniform2fv              = NULL;
PFNGLUNIFORM3FPROC               glUniform3f               = NULL;
PFNGLUNIFORM3FVPROC              glUniform3fv              = NULL;
PFNGLUNIFORM4FPROC               glUniform4f               = NULL;
PFNGLUNIFORM4FVPROC              glUniform4fv              = NULL;
PFNGLUNIFORMMATRIX3FVPROC        glUniformMatrix3fv        = NULL;
PFNGLUNIFORMMATRIX4FVPROC        glUniformMatrix4fv        = NULL;

PFNGLGENVERTEXARRAYSPROC         glGenVertexArrays         = NULL;
PFNGLDELETEVERTEXARRAYSPROC      glDeleteVertexArrays      = NULL;
PFNGLBINDVERTEXARRAYPROC         glBindVertexArray         = NULL;
PFNGLGENBUFFERSPROC              glGenBuffers              = NULL;
PFNGLDELETEBUFFERSPROC           glDeleteBuffers           = NULL;
PFNGLBINDBUFFERPROC              glBindBuffer              = NULL;
PFNGLBUFFERDATAPROC              glBufferData              = NULL;
PFNGLBUFFERSUBDATAPROC           glBufferSubData           = NULL;
PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer     = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = NULL;

PFNGLACTIVETEXTUREPROC           glActiveTexture           = NULL;
PFNGLGENERATEMIPMAPPROC          glGenerateMipmap          = NULL;

PFNGLGENFRAMEBUFFERSPROC         glGenFramebuffers         = NULL;
PFNGLBINDFRAMEBUFFERPROC         glBindFramebuffer         = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC    glFramebufferTexture2D    = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC  glCheckFramebufferStatus  = NULL;

/* -------------------------------------------------------------------------
 * Internal loader helper
 * ---------------------------------------------------------------------- */
static GLADloadproc s_loader = NULL;

static void* load_fn(const char* name) {
    void* fn = s_loader(name);
    if (!fn) {
        fprintf(stderr, "[GLAD] Failed to load: %s\n", name);
    }
    return fn;
}

/* -------------------------------------------------------------------------
 * gladLoadGLLoader — call once after glfwMakeContextCurrent()
 * ---------------------------------------------------------------------- */
int gladLoadGLLoader(GLADloadproc load) {
    s_loader = load;

    glCreateShader            = (PFNGLCREATESHADERPROC)           load_fn("glCreateShader");
    glDeleteShader            = (PFNGLDELETESHADERPROC)           load_fn("glDeleteShader");
    glShaderSource            = (PFNGLSHADERSOURCEPROC)           load_fn("glShaderSource");
    glCompileShader           = (PFNGLCOMPILESHADERPROC)          load_fn("glCompileShader");
    glGetShaderiv             = (PFNGLGETSHADERIVPROC)            load_fn("glGetShaderiv");
    glGetShaderInfoLog        = (PFNGLGETSHADERINFOLOGPROC)       load_fn("glGetShaderInfoLog");
    glCreateProgram           = (PFNGLCREATEPROGRAMPROC)          load_fn("glCreateProgram");
    glDeleteProgram           = (PFNGLDELETEPROGRAMPROC)          load_fn("glDeleteProgram");
    glAttachShader            = (PFNGLATTACHSHADERPROC)           load_fn("glAttachShader");
    glLinkProgram             = (PFNGLLINKPROGRAMPROC)            load_fn("glLinkProgram");
    glUseProgram              = (PFNGLUSEPROGRAMPROC)             load_fn("glUseProgram");
    glGetProgramiv            = (PFNGLGETPROGRAMIVPROC)           load_fn("glGetProgramiv");
    glGetProgramInfoLog       = (PFNGLGETPROGRAMINFOLOGPROC)      load_fn("glGetProgramInfoLog");
    glGetUniformLocation      = (PFNGLGETUNIFORMLOCATIONPROC)     load_fn("glGetUniformLocation");

    glUniform1i               = (PFNGLUNIFORM1IPROC)              load_fn("glUniform1i");
    glUniform1f               = (PFNGLUNIFORM1FPROC)              load_fn("glUniform1f");
    glUniform2f               = (PFNGLUNIFORM2FPROC)              load_fn("glUniform2f");
    glUniform2fv              = (PFNGLUNIFORM2FVPROC)             load_fn("glUniform2fv");
    glUniform3f               = (PFNGLUNIFORM3FPROC)              load_fn("glUniform3f");
    glUniform3fv              = (PFNGLUNIFORM3FVPROC)             load_fn("glUniform3fv");
    glUniform4f               = (PFNGLUNIFORM4FPROC)              load_fn("glUniform4f");
    glUniform4fv              = (PFNGLUNIFORM4FVPROC)             load_fn("glUniform4fv");
    glUniformMatrix3fv        = (PFNGLUNIFORMMATRIX3FVPROC)       load_fn("glUniformMatrix3fv");
    glUniformMatrix4fv        = (PFNGLUNIFORMMATRIX4FVPROC)       load_fn("glUniformMatrix4fv");

    glGenVertexArrays         = (PFNGLGENVERTEXARRAYSPROC)        load_fn("glGenVertexArrays");
    glDeleteVertexArrays      = (PFNGLDELETEVERTEXARRAYSPROC)     load_fn("glDeleteVertexArrays");
    glBindVertexArray         = (PFNGLBINDVERTEXARRAYPROC)        load_fn("glBindVertexArray");
    glGenBuffers              = (PFNGLGENBUFFERSPROC)             load_fn("glGenBuffers");
    glDeleteBuffers           = (PFNGLDELETEBUFFERSPROC)          load_fn("glDeleteBuffers");
    glBindBuffer              = (PFNGLBINDBUFFERPROC)             load_fn("glBindBuffer");
    glBufferData              = (PFNGLBUFFERDATAPROC)             load_fn("glBufferData");
    glBufferSubData           = (PFNGLBUFFERSUBDATAPROC)          load_fn("glBufferSubData");
    glVertexAttribPointer     = (PFNGLVERTEXATTRIBPOINTERPROC)    load_fn("glVertexAttribPointer");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)load_fn("glEnableVertexAttribArray");
    glDisableVertexAttribArray= (PFNGLDISABLEVERTEXATTRIBARRAYPROC)load_fn("glDisableVertexAttribArray");

    glActiveTexture           = (PFNGLACTIVETEXTUREPROC)          load_fn("glActiveTexture");
    glGenerateMipmap          = (PFNGLGENERATEMIPMAPPROC)         load_fn("glGenerateMipmap");

    glGenFramebuffers         = (PFNGLGENFRAMEBUFFERSPROC)        load_fn("glGenFramebuffers");
    glBindFramebuffer         = (PFNGLBINDFRAMEBUFFERPROC)        load_fn("glBindFramebuffer");
    glFramebufferTexture2D    = (PFNGLFRAMEBUFFERTEXTURE2DPROC)   load_fn("glFramebufferTexture2D");
    glCheckFramebufferStatus  = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) load_fn("glCheckFramebufferStatus");

    /* Critical functions check */
    if (!glCreateShader || !glGenVertexArrays || !glGenBuffers) {
        fprintf(stderr, "[GLAD] ERROR: Core OpenGL 3.3 functions not available.\n");
        fprintf(stderr, "[GLAD] Ensure your GPU and drivers support OpenGL 3.3+\n");
        return 0;
    }

    return 1;
}
