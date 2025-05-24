/****************************************************************************
 * WiiMC
 * Tantric 2009-2012
 *
 * mem2_manager.h
 *
 * MEM2 allocator
 ****************************************************************************/

#ifndef _MEM2MANAGER_H_
#define _MEM2MANAGER_H_
#include <ogc/system.h>

//1: critical errors     2: detailed info
#undef DEBUG_MEM2_LEVEL  // to get info about used mem, it's an approximation because of memory fragmentation
#undef DONT_USE_MEM2

enum mem2_areas_enum
{
	MEM2_VIDEO,
	MEM2_BROWSER,
	MEM2_GUI,
	MEM2_OTHER,
	MEM2_PICTURE,
	MEM2_INDEX,
	MEM2_EXTFONT,
	MEM2_MAX
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG_MEM2_LEVEL
#define mem2_memalign(a,b,c) _mem2_memalign(a,b,c,__FILE__, __LINE__)
#define mem2_malloc(a,b) _mem2_malloc(a,b,__FILE__, __LINE__)
#define mem2_free(a,b) _mem2_free(a,b,__FILE__, __LINE__)
#define mem2_calloc(a,b,c) _mem2_calloc(a,b,c,__FILE__, __LINE__)
#define mem2_realloc(a,b,c) _mem2_realloc(a,b,c,__FILE__, __LINE__)
#define mem2_strdup(a,b) _mem2_strdup(a,b,__FILE__, __LINE__)
#define mem2_strndup(a,b,c) _mem2_strndup(a,b,c,__FILE__, __LINE__)

#elif defined DONT_USE_MEM2
#define mem2_memalign(a,b,c) memalign(a,b)
#define mem2_malloc(a,b) malloc(a)
#define mem2_free(a,b) free(a)
#define mem2_calloc(a,b,c) calloc(a,b)
#define mem2_realloc(a,b,c) realloc(a,b)
#define mem2_strdup(a,b) strdup(a)
#define mem2_strndup(a,b,c) strndup(a,b)

#else
#define mem2_memalign(a,b,c) _mem2_memalign(a,b,c,NULL,0)
#define mem2_malloc(a,b) _mem2_malloc(a,b,NULL,0)
#define mem2_free(a,b) _mem2_free(a,b,NULL,0)
#define mem2_calloc(a,b,c) _mem2_calloc(a,b,c,NULL,0)
#define mem2_realloc(a,b,c) _mem2_realloc(a,b,c,NULL,0)
#define mem2_strdup(a,b) _mem2_strdup(a,b,NULL,0)
#define mem2_strndup(a,b,c) _mem2_strndup(a,b,c,NULL,0)
#endif

bool AddMem2Area(u32 size, int index);
bool RemoveMem2Area(int area);
void ClearMem2Area(int area);

void* _mem2_memalign(u8 align, u32 size, int area, const char* file, int line);
void* _mem2_malloc(u32 size, int area, const char* file, int line);
void _mem2_free(void* ptr, int area, const char* file, int line);
void* _mem2_calloc(u32 num, u32 size, int area, const char* file, int line);
void* _mem2_realloc(void* ptr, u32 newsize, int area, const char* file, int line);
char* _mem2_strdup(const char* s, int area, const char* file, int line);
char* _mem2_strndup(const char* s, size_t n, int area, const char* file, int line);

u32 mem2_size(int i);

void ShowAreaInfo(int area); //if area == -1 print all areas info

#ifdef __cplusplus
}
#endif

#endif
