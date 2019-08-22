#ifndef _MYWGL_H_
#define _MYWGL_H_

#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_PIXEL_TYPE_ARB 0x2013
#define WGL_TYPE_RGBA_ARB 0x202B
#define WGL_RED_BITS_ARB 0x2015
#define WGL_GREEN_BITS_ARB 0x2017
#define WGL_BLUE_BITS_ARB 0x2019
#define WGL_DEPTH_BITS_ARB 0x2022
#define WGL_DOUBLE_BUFFER_ARB 0x2011
#define WGL_COLOR_BITS_ARB 0x2014
#define WGL_STENCIL_BITS_ARB 0x2023
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_ACCELERATION_ARB 0x2003
#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_ALPHA_BITS_ARB 0x201B

/* --------------------- WGL_ARB_create_context_profile -------------------- */

#ifndef WGL_ARB_create_context_profile
#define WGL_ARB_create_context_profile 1

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126

#define WGLEW_ARB_create_context_profile WGLEW_GET_VAR(__WGLEW_ARB_create_context_profile)

#endif /* WGL_ARB_create_context_profile */

/* ------------------- WGL_EXT_create_context_es2_profile ------------------ */

#ifndef WGL_EXT_create_context_es2_profile
#define WGL_EXT_create_context_es2_profile 1

#define WGL_CONTEXT_ES2_PROFILE_BIT_EXT 0x00000004

#define WGLEW_EXT_create_context_es2_profile WGLEW_GET_VAR(__WGLEW_EXT_create_context_es2_profile)

#endif /* WGL_EXT_create_context_es2_profile */

/* ------------------- WGL_EXT_create_context_es_profile ------------------- */

#ifndef WGL_EXT_create_context_es_profile
#define WGL_EXT_create_context_es_profile 1

#define WGL_CONTEXT_ES_PROFILE_BIT_EXT 0x00000004

#define WGLEW_EXT_create_context_es_profile WGLEW_GET_VAR(__WGLEW_EXT_create_context_es_profile)

#endif /* WGL_EXT_create_context_es_profile */

#endif//_MYWGL_H_