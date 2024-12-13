/**
 * Copyright (c) James Roy <rruuaanng@outlook.com>
 */
#include "pydt.h"


/**
 * Module infomation
 */
#undef MODULE_NAME
#undef MODULE_VERSION
#define MODULE_NAME "_fdt"
#define MODULE_VERSION "0.0.1"
static PyTypeObject FDT_type;
static FDTObject;



/**
 * Get the offset of the specified path, setting a Python exception
 * and return -1 if not found.
 */
int _get_fdt_path_offset(const void *fdt, const char *path)
{
    int offset = 0;

    offset = fdt_path_offset(fdt, path);
    switch (offset) {
    case -FDT_ERR_NOTFOUND:
        PyErr_SetString(PyExc_ValueError, "path does not exist");
        return -1;
    case -FDT_ERR_BADPATH:
        PyErr_SetString(PyExc_ValueError, "invalid path");
        return -1;
    }

    return offset;
}

/** 
 * Traverse all properties under a specified path based on the given
 * offset and return a Python dict containing the properties and their values.
 */
static PyObject *_foreach_path_props_by_offset(const void *fdt, int offset)
{
    uint32_t ret;
    int props_offset, len;
    struct fdt_property *prop;
    const char *propname;
    PyObject *value;
    PyObject *props = PyDict_New();

    if (props == NULL) {
        PyErr_SetString(PyExc_RuntimeError,
                        "dict 'props' instantiation failed");
        return NULL;
    }

    /**  */
    fdt_for_each_property_offset(props_offset, fdt, offset) {
        prop = fdt_get_property_by_offset(fdt, props_offset, &len);
        propname = fdt_string(fdt, fdt32_to_cpu(prop->nameoff));

        if (len < 0 || prop->data == NULL) {
            value = Py_None;
            goto next;
        }

        /* compatible attribute */
        if (!strcmp(propname, "compatible")) {
            value = PyUnicode_FromStringAndSize(prop->data, len - 1);
            goto next;
        }
        if (len % 4 == 0) {
            value = PyList_New(len / 4);
            /* boolean attribute */
            if (len == 0) {
                PyList_Append(value, Py_True);
                goto next;
            }
            /* multi-value attribute */
            for (int i = 0; i < len / 4; i++) {
                ret = fdt32_to_cpu(((fdt32_t *)prop->data)[i]);
                PyList_SetItem(value, i,
                               PyUnicode_FromFormat("0x%x", ret));
            }
        }

    next:
        PyDict_SetItemString(props, propname, value);
    }

    Py_DECREF(value);
    return props;
}

const void *_read_dtb_file(const char *filename)
{
    struct stat s;
    char *fdt;
    FILE *fp;

    if ((fp = fopen(filename, "rb")) == NULL) {
        return NULL;
    }

    if (stat(filename, &s)) {
        return NULL;
    }

    fdt = PyMem_Malloc(s.st_size);
    fread(fdt, s.st_size, sizeof(char), fp);
    fclose(fp);
    return fdt;

error:
    fclose(fp);
    return NULL;
}

bool _is_dtb_file(const char *filename)
{
    size_t len = strlen(filename);
    const char *prefix = filename + len - 3;

    if (strcmp(prefix, "dtb")) {
        return false;
    }

    return true;
}

/**
 * FDT objects methods
 */
static PyObject *get_path_props(FDTObject *self, PyObject *args)
{
    int offset;
    const char *path;

    if (!PyArg_ParseTuple(args, "s", &path)) {
        return NULL;
    }

    offset = _get_fdt_path_offset(self->fdt, path);
    if (offset == -1) {
        return NULL;
    }

    return _foreach_path_props_by_offset(self->fdt, offset);
}

static PyObject *get_path_offset(FDTObject *self, PyObject *args)
{
    int offset;
    const char *path;

    if (!PyArg_ParseTuple(args, "s", &path)) {
        return NULL;
    }

    offset = _get_fdt_path_offset(self->fdt, path);
    if (offset == -1) {
        return NULL;
    }

    return PyLong_FromLong(offset);
}

static PyObject *get_alias(FDTObject *self, PyObject *args)
{
    const char *name;
    const char *alias;

    if (!PyArg_ParseTuple(args, "s", &name)) {
        return NULL;
    }

    alias = fdt_get_alias(self->fdt, name);
    if (alias == NULL) {
        return Py_None;
    }

    return PyUnicode_FromString(alias);
}

static PyMethodDef fdt_obj_methods[] = {
    {"get_path_props", (PyCFunction)get_path_props, METH_VARARGS,
    PyDoc_STR("")},
    {"get_path_offset", (PyCFunction)get_path_offset, METH_VARARGS,
    PyDoc_STR("")},
    {"get_alias", (PyCFunction)get_alias, METH_VARARGS,
    PyDoc_STR("")},
    {NULL},
};

static PyObject *fdt_obj_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    const char *filename;
    FDTObject *fdtobject;
    char *kwlist[] = {"filename", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s",
                                     kwlist, &filename)) {
        return NULL;
    }

    fdtobject = PyObject_New(FDTObject, &FDT_type);

    if (!_is_dtb_file(filename)) {
        PyErr_Format(PyExc_RuntimeError, "'%s' is not a valid DTB file",
                     filename);
        return NULL;
    }

    fdtobject->fdt = _read_dtb_file(filename);
    if (fdtobject->fdt == NULL) {
        PyErr_Format(PyExc_OSError, "failed to open file '%s'", filename);
        return NULL;
    }

    if (fdt_check_header(fdtobject->fdt)) {
        PyErr_Format(PyExc_ValueError, "'%s' is not a valid FDT file",
                     filename);
        return NULL;
    }
    fdtobject->magic = fdt_magic(fdtobject->fdt);
    fdtobject->version = fdt_version(fdtobject->fdt);
    fdtobject->totalsize = fdt_totalsize(fdtobject->fdt);

    return (PyObject *)fdtobject;
}

static void fdt_obj_free(void *args)
{
}

static PyMemberDef fdt_obj_members[] = {
    {"magic", Py_T_UINT, offsetof(FDTObject, magic), Py_READONLY,
    PyDoc_STR("")},
    {"version", Py_T_UINT, offsetof(FDTObject, version), Py_READONLY,
    PyDoc_STR("")},
    {"totalsize", Py_T_UINT, offsetof(FDTObject, totalsize), Py_READONLY,
    PyDoc_STR("")},
    {NULL},
};

PyDoc_STRVAR(fdt_obj_doc,
""
"");
static PyTypeObject FDT_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "_fdt.FDT",
    .tp_basicsize = sizeof(FDTObject),
    .tp_new = fdt_obj_new,
    .tp_free = fdt_obj_free,
    .tp_methods = fdt_obj_methods,
    .tp_members = fdt_obj_members,
    .tp_doc = fdt_obj_doc,
};

/**
 * module methods
 */
static PyMethodDef fdt_methods[] = {
    {NULL},
};

/**
 * module slots
 */
static int pydt_exec(PyObject *m)
{
    if (PyModule_AddStringConstant(m, "__version__", MODULE_VERSION) < 0) {
        return -1;
    }

    if (PyType_Ready(&FDT_type) < 0) {
        return -1;
    }
    if (PyModule_AddObjectRef(m, "FDT", (PyObject *)&FDT_type) < 0) {
        return -1;
    }

    return 0;
}

static PyModuleDef_Slot fdt_slots[] = {
    {Py_mod_exec, pydt_exec},
    {0, NULL},
};

/**
 * module
 */
PyDoc_STRVAR(fdt_doc,
""
"");
static PyModuleDef fdt_module = {
    .m_name = MODULE_NAME,
    .m_doc = fdt_doc,
    .m_methods = fdt_methods,
    .m_slots = fdt_slots,
};

PyMODINIT_FUNC PyInit__fdt(void)
{
    return PyModuleDef_Init(&fdt_module);
}
