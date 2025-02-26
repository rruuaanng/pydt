#ifndef PYFDT_FDTMODULE_H_
#define PYFDT_FDTMODULE_H_

/* include Python.h */
#define PY_SSIZE_T_CLEAN
#include "pythoncapi_compat.h"

/* include standard */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* */
#include <libfdt.h>


#define MODULE_NAME    "_fdt"
#define MODULE_VERSION "0.0.1"


#ifndef FDT_FULL_PATH_LEN
# define FDT_FULL_PATH_LEN 0x100 /* 256 */
#endif

typedef struct {
    PyObject_HEAD

    /* Raw FDT */
    uint32_t magic;
    uint32_t version;
    uint32_t totalsize;
    size_t headersize;
    const void *fdt;
    int fdt_errno;
}FDTObject;


#endif // PYFDT_FDTMODULE_H_