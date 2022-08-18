#pragma once

#include "../../defines.h"
#include "../kmemory.h"

enum {
    DARRAY_CAPACITY,
    DARRAY_LENGTH,
    DARRAY_STRIDE,
    DARRAY_FIELD_LENGTH
};

#define DARRAY_DEFAULT_CAPACITY 1
#define DARRAY_RESIZE_FACTOR 2

#define darrayLength(array) \
    _darray_field_get(array, DARRAY_LENGTH)

    #define darray_stride(array) \
    _darray_field_get(array, DARRAY_STRIDE)

template<typename T, typename T2> KINLINE T * _darray_create(T length, T2 stride){
 
    u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 array_size = length * stride;
    //cast here might need to change to T *?
    u64* new_array = (u64 *)kallocate(header_size + array_size, MEMORY_TAG_DARRAY);
    ksetMemory(new_array, 0, header_size + array_size);
    new_array[DARRAY_CAPACITY] = length;
    new_array[DARRAY_LENGTH] = 0;
    new_array[DARRAY_STRIDE] = stride;
    return (T *)(new_array + DARRAY_FIELD_LENGTH);
};

KINLINE KAPI void _darray_destroy(void * array){

      u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 total_size = header_size + header[DARRAY_CAPACITY] * header[DARRAY_STRIDE];
    kfree(header, total_size, MEMORY_TAG_DARRAY);
};

KINLINE KAPI u64 _darray_field_get(void* array, u64 field){

      u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    return header[field];
}

#define darray_capacity(array) \
    _darray_field_get(array, DARRAY_CAPACITY)

KINLINE KAPI void _darray_field_set(void* array, u64 field, u64 value){

    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    header[field] = value;
}

template<class T> KINLINE T * _darray_resize(T* array){

      u64 length = darrayLength(array);
    u64 stride = darray_stride(array);
    T* temp = (T*)_darray_create(
        (DARRAY_RESIZE_FACTOR * darray_capacity(array)),
        stride);
    kcopyMemory(temp, array, length * stride);

    _darray_field_set(temp, DARRAY_LENGTH, length);
    _darray_destroy(array);
    return temp;
}

template<class T> KINLINE  T *  _darray_push(T* array, const void* value_ptr){

     u64 length = darrayLength(array);
    u64 stride = darray_stride(array);
    if (length >= darray_capacity(array)) {
        array = _darray_resize(array);
    }

    u64 addr = (u64)array;
    addr += (length * stride);
    kcopyMemory((void*)addr, value_ptr, stride);
    _darray_field_set(array, DARRAY_LENGTH, length + 1);
    return array;
}

KINLINE KAPI void _darray_pop(void* array, void* dest){

     u64 length = darrayLength(array);
    u64 stride = darray_stride(array);
    u64 addr = (u64)array;
    addr += ((length - 1) * stride);
    kcopyMemory(dest, (void*)addr, stride);
    _darray_field_set(array, DARRAY_LENGTH, length - 1);
}

 template<typename T, typename F> KINLINE T *  _darray_pop_at(T* array, u64 index, F* dest){

 u64 length = darrayLength(array);
    u64 stride = darray_stride(array);
    if (index >= length) {
        KERROR("Index outside the bounds of this array! Length: %i, index: %index", length, index);
        return array;
    }

    u64 addr = (u64)array;
    kcopyMemory(dest, (void*)(addr + (index * stride)), stride);

    // If not on the last element, snip out the entry and copy the rest inward.
    if (index != length - 1) {
        kcopyMemory(
            (void*)(addr + (index * stride)),
            (void*)(addr + ((index + 1) * stride)),
            stride * (length - index));
    }

    _darray_field_set(array, DARRAY_LENGTH, length - 1);
    return array;

};

template<class T> KINLINE T * _darray_insert_at(T* array, u64 index, T* value_ptr){

      u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    if (index >= length) {
        KERROR("Index outside the bounds of this array! Length: %i, index: %index", length, index);
        return array;
    }
    if (length >= darray_capacity(array)) {
        array = _darray_resize(array);
    }

    u64 addr = (u64)array;

    // If not on the last element, copy the rest outward.
    if (index != length - 1) {
        kcopyMemory(
            (void*)(addr + ((index + 1) * stride)),
            (void*)(addr + (index * stride)),
            stride * (length - index));
    }

    // Set the value at the index
    kcopyMemory((void*)(addr + (index * stride)), value_ptr, stride);

    _darray_field_set(array, DARRAY_LENGTH, length + 1);
    return array;
};

#define darrayCreate(type) \
    _darray_create(DARRAY_DEFAULT_CAPACITY, sizeof(type)) \

#define darray_reserve(type, capacity) \
    _darray_create(capacity, sizeof(type)) \

#define darray_destroy(array) _darray_destroy(array);

#define darray_push(array, value)           \
    {                                       \
        typeof(value) temp = value;         \
        array = _darray_push(array, &temp); \
    }
// NOTE: could use __auto_type for temp above, but intellisense
// for VSCode flags it as an unknown type. typeof() seems to
// work just fine, though. Both are GNU extensions.

#define darray_pop(array, value_ptr) \
    _darray_pop(array, value_ptr)

#define darray_insert_at(array, index, value)           \
    {                                                   \
        typeof(value) temp = value;                     \
        array = _darray_insert_at(array, index, &temp); \
    }

#define darray_pop_at(array, index, value_ptr) \
    _darray_pop_at(array, index, value_ptr)

#define darray_clear(array) \
    _darray_field_set(array, DARRAY_LENGTH, 0)




#define darray_length_set(array, value) \
    _darray_field_set(array, DARRAY_LENGTH, value)
