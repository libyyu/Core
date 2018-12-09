#include <stdio.h>
#include <stdlib.h>

extern void* sys_malloc(size_t size)
{
    return malloc(size);
}

extern void sys_free(void* ptr) 
{
    free(ptr);
}