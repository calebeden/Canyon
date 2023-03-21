#ifndef SERIALIZABLESTRING_H
#define SERIALIZABLESTRING_H

#include "core.h"
#include "Serializable.h"

Type SerializableStringT;

struct _SerializableString_VTBL {
    
};

#define SERIALIZABLESTRING_VTBL {}

struct _SerializableString {
    String super;
    struct _Serializable_VTBL *vptr;
};

typedef struct _SerializableString *SerializableString;

#endif