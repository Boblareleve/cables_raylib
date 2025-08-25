
#define PRINT_IMPLEMENTATION
#define STRING_IMPLEMENTATION
#define STRING_FILEIO
#define SET_IMPLEMENTATION
#define ALF_IMPLEMENTATION
// #define RAYGUI_IMPLEMENTATION
#define JSON_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION


#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <inttypes.h>

void *align_alloc(size_t size, size_t align)
{
    void *initial_alloc = malloc(sizeof(void*) + size + align);

    void *res = (void*)(((uintptr_t)initial_alloc & ~((uintptr_t)align - 1)) + align);
    assert(((uintptr_t)res & ((uintptr_t)align - 1)) == 0);
    assert((uintptr_t)((void**)res - 1) >= (uintptr_t)initial_alloc);

    ((void**)res)[-1] = initial_alloc;

    return res;
}
void align_free(void *ptr)
{
    assert(ptr - 0xffff <= (((void**)ptr)[-1]) && (((void**)ptr)[-1]) <= ptr + 0xffff);
    free(((void**)ptr)[-1]);
}

#define ALF_ALLOCATOR(size, align) align_alloc(size, align)
#define ALF_FREE(ptr) align_free(ptr)

#include "../engin/engin.h"


#include "alf.h"
#include "sets.h"
#define STRING_FILEIO
#include "Str.h"
#include "print.h"
#include "sa.h"
#include "json.h"
#include "raylib.h"
// #include "raygui.h"
#include "stb_image.h"

