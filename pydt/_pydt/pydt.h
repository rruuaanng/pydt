#ifndef PYFDT_FDTMODULE_H_
#define PYFDT_FDTMODULE_H_

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#if PY_VERSION_HEX >= 0x03120000
# include <descrobject.h>
#endif

#if PY_VERSION_HEX >= 0x03090000 && PY_VERSION_HEX < 0x03120000
# include <structmember.h>
/** member type */
# ifndef Py_T_UINT
#  define Py_T_UINT T_UINT
# endif
/** flags */
# ifndef Py_READONLY
#  define Py_READONLY READONLY
# endif
#endif


/* include POSIX */
#include <fcntl.h>
#include <sys/stat.h>

/* include standard */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* */
#include <libfdt.h>


typedef struct {
    PyObject_HEAD

    /* Raw FDT */
    uint32_t magic;
    uint32_t version;
    const void *fdt;
}FDTObject;


#endif // PYFDT_FDTMODULE_H_