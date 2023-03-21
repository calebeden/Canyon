#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H

#include "core.h"

Type SerializableT = {"Serializable", 0};

struct _Serializable_VTBL {
    // TODO: Add file stream
    void (*serialize)(void *serializable);
    void *(*deserialize)();
};

struct _Serializable {
    struct _Serializable_VTBL *vptr;
};

typedef struct _Serializable *Serializable;


#endif