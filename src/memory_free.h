
#ifndef __DPL_MEMORY_FREE_H__
#define __DPL_MEMORY_FREE_H__

#include <memory>
#include <cstddef>

#define dpl_freep(p) \
    if (p) { \
        delete p; \
        p = NULL; \
    } \
    (void)0

#define dpl_freepa(pa) \
    if (pa) { \
        delete[] pa; \
        pa = NULL; \
    } \
    (void)0

#endif // __DPL_MEMORY_FREE_H__
