#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "GL/wgl_core.h"

#if defined(__APPLE__)
#include <dlfcn.h>

static void* AppleGLGetProcAddress (const char *name)
{
	static void* image = NULL;
	
	if (NULL == image)
		image = dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_LAZY);

	return (image ? dlsym(image, name) : NULL);
}
#endif /* __APPLE__ */

#if defined(__sgi) || defined (__sun)
#include <dlfcn.h>
#include <stdio.h>

static void* SunGetProcAddress (const GLubyte* name)
{
  static void* h = NULL;
  static void* gpa;

  if (h == NULL)
  {
    if ((h = dlopen(NULL, RTLD_LAZY | RTLD_LOCAL)) == NULL) return NULL;
    gpa = dlsym(h, "glXGetProcAddress");
  }

  if (gpa != NULL)
    return ((void*(*)(const GLubyte*))gpa)(name);
  else
    return dlsym(h, (const char*)name);
}
#endif /* __sgi || __sun */

#if defined(_WIN32)

#ifdef _MSC_VER
#pragma warning(disable: 4055)
#pragma warning(disable: 4054)
#pragma warning(disable: 4996)
#endif

static int TestPointer(const PROC pTest)
{
	ptrdiff_t iTest;
	if(!pTest) return 0;
	iTest = (ptrdiff_t)pTest;
	
	if(iTest == 1 || iTest == 2 || iTest == 3 || iTest == -1) return 0;
	
	return 1;
}

static PROC WinGetProcAddress(const char *name)
{
	HMODULE glMod = NULL;
	PROC pFunc = wglGetProcAddress((LPCSTR)name);
	if(TestPointer(pFunc))
	{
		return pFunc;
	}
	glMod = GetModuleHandleA("OpenGL32.dll");
	return (PROC)GetProcAddress(glMod, (LPCSTR)name);
}
	
#define IntGetProcAddress(name) WinGetProcAddress(name)
#else
	#if defined(__APPLE__)
		#define IntGetProcAddress(name) AppleGLGetProcAddress(name)
	#else
		#if defined(__sgi) || defined(__sun)
			#define IntGetProcAddress(name) SunGetProcAddress(name)
		#else /* GLX */
		    #include <GL/glx.h>

			#define IntGetProcAddress(name) (*glXGetProcAddressARB)((const GLubyte*)name)
		#endif
	#endif
#endif

int wgl_ext_ARB_multisample = wgl_LOAD_FAILED;
int wgl_ext_ARB_extensions_string = wgl_LOAD_FAILED;
int wgl_ext_ARB_pixel_format = wgl_LOAD_FAILED;
int wgl_ext_ARB_pixel_format_float = wgl_LOAD_FAILED;
int wgl_ext_ARB_framebuffer_sRGB = wgl_LOAD_FAILED;
int wgl_ext_ARB_create_context = wgl_LOAD_FAILED;
int wgl_ext_ARB_create_context_profile = wgl_LOAD_FAILED;
int wgl_ext_ARB_create_context_robustness = wgl_LOAD_FAILED;
int wgl_ext_EXT_swap_control = wgl_LOAD_FAILED;
int wgl_ext_EXT_pixel_format_packed_float = wgl_LOAD_FAILED;
int wgl_ext_EXT_create_context_es2_profile = wgl_LOAD_FAILED;
int wgl_ext_EXT_swap_control_tear = wgl_LOAD_FAILED;
int wgl_ext_NV_swap_group = wgl_LOAD_FAILED;

const char * (CODEGEN_FUNCPTR *_ptrc_wglGetExtensionsStringARB)(HDC hdc) = NULL;

static int Load_ARB_extensions_string(void)
{
	int numFailed = 0;
	_ptrc_wglGetExtensionsStringARB = (const char * (CODEGEN_FUNCPTR *)(HDC))IntGetProcAddress("wglGetExtensionsStringARB");
	if(!_ptrc_wglGetExtensionsStringARB) numFailed++;
	return numFailed;
}

BOOL (CODEGEN_FUNCPTR *_ptrc_wglChoosePixelFormatARB)(HDC hdc, const int * piAttribIList, const FLOAT * pfAttribFList, UINT nMaxFormats, int * piFormats, UINT * nNumFormats) = NULL;
BOOL (CODEGEN_FUNCPTR *_ptrc_wglGetPixelFormatAttribfvARB)(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int * piAttributes, FLOAT * pfValues) = NULL;
BOOL (CODEGEN_FUNCPTR *_ptrc_wglGetPixelFormatAttribivARB)(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int * piAttributes, int * piValues) = NULL;

static int Load_ARB_pixel_format(void)
{
	int numFailed = 0;
	_ptrc_wglChoosePixelFormatARB = (BOOL (CODEGEN_FUNCPTR *)(HDC, const int *, const FLOAT *, UINT, int *, UINT *))IntGetProcAddress("wglChoosePixelFormatARB");
	if(!_ptrc_wglChoosePixelFormatARB) numFailed++;
	_ptrc_wglGetPixelFormatAttribfvARB = (BOOL (CODEGEN_FUNCPTR *)(HDC, int, int, UINT, const int *, FLOAT *))IntGetProcAddress("wglGetPixelFormatAttribfvARB");
	if(!_ptrc_wglGetPixelFormatAttribfvARB) numFailed++;
	_ptrc_wglGetPixelFormatAttribivARB = (BOOL (CODEGEN_FUNCPTR *)(HDC, int, int, UINT, const int *, int *))IntGetProcAddress("wglGetPixelFormatAttribivARB");
	if(!_ptrc_wglGetPixelFormatAttribivARB) numFailed++;
	return numFailed;
}

HGLRC (CODEGEN_FUNCPTR *_ptrc_wglCreateContextAttribsARB)(HDC hDC, HGLRC hShareContext, const int * attribList) = NULL;

static int Load_ARB_create_context(void)
{
	int numFailed = 0;
	_ptrc_wglCreateContextAttribsARB = (HGLRC (CODEGEN_FUNCPTR *)(HDC, HGLRC, const int *))IntGetProcAddress("wglCreateContextAttribsARB");
	if(!_ptrc_wglCreateContextAttribsARB) numFailed++;
	return numFailed;
}

int (CODEGEN_FUNCPTR *_ptrc_wglGetSwapIntervalEXT)(void) = NULL;
BOOL (CODEGEN_FUNCPTR *_ptrc_wglSwapIntervalEXT)(int interval) = NULL;

static int Load_EXT_swap_control(void)
{
	int numFailed = 0;
	_ptrc_wglGetSwapIntervalEXT = (int (CODEGEN_FUNCPTR *)(void))IntGetProcAddress("wglGetSwapIntervalEXT");
	if(!_ptrc_wglGetSwapIntervalEXT) numFailed++;
	_ptrc_wglSwapIntervalEXT = (BOOL (CODEGEN_FUNCPTR *)(int))IntGetProcAddress("wglSwapIntervalEXT");
	if(!_ptrc_wglSwapIntervalEXT) numFailed++;
	return numFailed;
}

BOOL (CODEGEN_FUNCPTR *_ptrc_wglBindSwapBarrierNV)(GLuint group, GLuint barrier) = NULL;
BOOL (CODEGEN_FUNCPTR *_ptrc_wglJoinSwapGroupNV)(HDC hDC, GLuint group) = NULL;
BOOL (CODEGEN_FUNCPTR *_ptrc_wglQueryFrameCountNV)(HDC hDC, GLuint * count) = NULL;
BOOL (CODEGEN_FUNCPTR *_ptrc_wglQueryMaxSwapGroupsNV)(HDC hDC, GLuint * maxGroups, GLuint * maxBarriers) = NULL;
BOOL (CODEGEN_FUNCPTR *_ptrc_wglQuerySwapGroupNV)(HDC hDC, GLuint * group, GLuint * barrier) = NULL;
BOOL (CODEGEN_FUNCPTR *_ptrc_wglResetFrameCountNV)(HDC hDC) = NULL;

static int Load_NV_swap_group(void)
{
	int numFailed = 0;
	_ptrc_wglBindSwapBarrierNV = (BOOL (CODEGEN_FUNCPTR *)(GLuint, GLuint))IntGetProcAddress("wglBindSwapBarrierNV");
	if(!_ptrc_wglBindSwapBarrierNV) numFailed++;
	_ptrc_wglJoinSwapGroupNV = (BOOL (CODEGEN_FUNCPTR *)(HDC, GLuint))IntGetProcAddress("wglJoinSwapGroupNV");
	if(!_ptrc_wglJoinSwapGroupNV) numFailed++;
	_ptrc_wglQueryFrameCountNV = (BOOL (CODEGEN_FUNCPTR *)(HDC, GLuint *))IntGetProcAddress("wglQueryFrameCountNV");
	if(!_ptrc_wglQueryFrameCountNV) numFailed++;
	_ptrc_wglQueryMaxSwapGroupsNV = (BOOL (CODEGEN_FUNCPTR *)(HDC, GLuint *, GLuint *))IntGetProcAddress("wglQueryMaxSwapGroupsNV");
	if(!_ptrc_wglQueryMaxSwapGroupsNV) numFailed++;
	_ptrc_wglQuerySwapGroupNV = (BOOL (CODEGEN_FUNCPTR *)(HDC, GLuint *, GLuint *))IntGetProcAddress("wglQuerySwapGroupNV");
	if(!_ptrc_wglQuerySwapGroupNV) numFailed++;
	_ptrc_wglResetFrameCountNV = (BOOL (CODEGEN_FUNCPTR *)(HDC))IntGetProcAddress("wglResetFrameCountNV");
	if(!_ptrc_wglResetFrameCountNV) numFailed++;
	return numFailed;
}

typedef int (*PFN_LOADFUNCPOINTERS)(void);
typedef struct wgl_StrToExtMap_s
{
	char *extensionName;
	int *extensionVariable;
	PFN_LOADFUNCPOINTERS LoadExtension;
} wgl_StrToExtMap;

static wgl_StrToExtMap ExtensionMap[13] = {
	{"WGL_ARB_multisample", &wgl_ext_ARB_multisample, NULL},
	{"WGL_ARB_extensions_string", &wgl_ext_ARB_extensions_string, Load_ARB_extensions_string},
	{"WGL_ARB_pixel_format", &wgl_ext_ARB_pixel_format, Load_ARB_pixel_format},
	{"WGL_ARB_pixel_format_float", &wgl_ext_ARB_pixel_format_float, NULL},
	{"WGL_ARB_framebuffer_sRGB", &wgl_ext_ARB_framebuffer_sRGB, NULL},
	{"WGL_ARB_create_context", &wgl_ext_ARB_create_context, Load_ARB_create_context},
	{"WGL_ARB_create_context_profile", &wgl_ext_ARB_create_context_profile, NULL},
	{"WGL_ARB_create_context_robustness", &wgl_ext_ARB_create_context_robustness, NULL},
	{"WGL_EXT_swap_control", &wgl_ext_EXT_swap_control, Load_EXT_swap_control},
	{"WGL_EXT_pixel_format_packed_float", &wgl_ext_EXT_pixel_format_packed_float, NULL},
	{"WGL_EXT_create_context_es2_profile", &wgl_ext_EXT_create_context_es2_profile, NULL},
	{"WGL_EXT_swap_control_tear", &wgl_ext_EXT_swap_control_tear, NULL},
	{"WGL_NV_swap_group", &wgl_ext_NV_swap_group, Load_NV_swap_group},
};

static int g_extensionMapSize = 13;

static wgl_StrToExtMap *FindExtEntry(const char *extensionName)
{
	int loop;
	wgl_StrToExtMap *currLoc = ExtensionMap;
	for(loop = 0; loop < g_extensionMapSize; ++loop, ++currLoc)
	{
		if(strcmp(extensionName, currLoc->extensionName) == 0)
			return currLoc;
	}
	
	return NULL;
}

static void ClearExtensionVars(void)
{
	wgl_ext_ARB_multisample = wgl_LOAD_FAILED;
	wgl_ext_ARB_extensions_string = wgl_LOAD_FAILED;
	wgl_ext_ARB_pixel_format = wgl_LOAD_FAILED;
	wgl_ext_ARB_pixel_format_float = wgl_LOAD_FAILED;
	wgl_ext_ARB_framebuffer_sRGB = wgl_LOAD_FAILED;
	wgl_ext_ARB_create_context = wgl_LOAD_FAILED;
	wgl_ext_ARB_create_context_profile = wgl_LOAD_FAILED;
	wgl_ext_ARB_create_context_robustness = wgl_LOAD_FAILED;
	wgl_ext_EXT_swap_control = wgl_LOAD_FAILED;
	wgl_ext_EXT_pixel_format_packed_float = wgl_LOAD_FAILED;
	wgl_ext_EXT_create_context_es2_profile = wgl_LOAD_FAILED;
	wgl_ext_EXT_swap_control_tear = wgl_LOAD_FAILED;
	wgl_ext_NV_swap_group = wgl_LOAD_FAILED;
}


static void LoadExtByName(const char *extensionName)
{
	wgl_StrToExtMap *entry = NULL;
	entry = FindExtEntry(extensionName);
	if(entry)
	{
		if(entry->LoadExtension)
		{
			int numFailed = entry->LoadExtension();
			if(numFailed == 0)
			{
				*(entry->extensionVariable) = wgl_LOAD_SUCCEEDED;
			}
			else
			{
				*(entry->extensionVariable) = wgl_LOAD_SUCCEEDED + numFailed;
			}
		}
		else
		{
			*(entry->extensionVariable) = wgl_LOAD_SUCCEEDED;
		}
	}
}


static void ProcExtsFromExtString(const char *strExtList)
{
	size_t iExtListLen = strlen(strExtList);
	const char *strExtListEnd = strExtList + iExtListLen;
	const char *strCurrPos = strExtList;
	char strWorkBuff[256];

	while(*strCurrPos)
	{
		/*Get the extension at our position.*/
		int iStrLen = 0;
		const char *strEndStr = strchr(strCurrPos, ' ');
		int iStop = 0;
		if(strEndStr == NULL)
		{
			strEndStr = strExtListEnd;
			iStop = 1;
		}

		iStrLen = (int)((ptrdiff_t)strEndStr - (ptrdiff_t)strCurrPos);

		if(iStrLen > 255)
			return;

		strncpy(strWorkBuff, strCurrPos, iStrLen);
		strWorkBuff[iStrLen] = '\0';

		LoadExtByName(strWorkBuff);

		strCurrPos = strEndStr + 1;
		if(iStop) break;
	}
}

int wgl_LoadFunctions(HDC hdc)
{
	ClearExtensionVars();
	
	_ptrc_wglGetExtensionsStringARB = (const char * (CODEGEN_FUNCPTR *)(HDC))IntGetProcAddress("wglGetExtensionsStringARB");
	if(!_ptrc_wglGetExtensionsStringARB) return wgl_LOAD_FAILED;
	
	ProcExtsFromExtString((const char *)_ptrc_wglGetExtensionsStringARB(hdc));
	return wgl_LOAD_SUCCEEDED;
}
