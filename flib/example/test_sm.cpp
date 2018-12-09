#include "FPlugin.hpp"
#include "FFunc.hpp"
_FUsing(FStd)

typedef void*(*sys_malloc)(size_t);
typedef void(*sys_free)(void*);
static sys_malloc real_malloc = 0;
static sys_free real_free = 0;

static void __mtrace_init(void)
{
    char path[256] = {0};
    FSplitpath(__FILE__, path, NULL, NULL);
    std::string example_path = path;
    std::string dll_path = example_path + "/../libs/macosx/libsm.dylib";
    FPlugin plugin(dll_path.c_str(), false);
    real_malloc = plugin.Get<sys_malloc>("sys_malloc");
    real_free = plugin.Get<sys_free>("sys_free");
}

extern "C" void *malloc(size_t size)
{
    if(!real_malloc)
        __mtrace_init();
    return real_malloc(size);
}
extern "C" void free(void* ptr)
{
    if(!real_free)
        __mtrace_init();
    return real_free(ptr);
}

int main()
{
    char path[256] = {0};
    FSplitpath(__FILE__, path, NULL, NULL);
    __mtrace_init();
    char *ps = NULL;
    ps = (char *)malloc(100 * sizeof(char));
    strcpy(ps, path);

    int* p = (int*)(malloc(sizeof(int)));
    *p = 9;

    return 0;
}