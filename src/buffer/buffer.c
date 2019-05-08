#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "buffer.h"

// Helper func: Move pointer 1 place left, wrapping around end of buffer
// Expects to be called for use: removing one item
static void buffer_move_ptr_left(buffer_t* const buf) {
    buf->full = false;
    buf->end = (buf->end + 1) % buf->max;
}

// Helper func: Move pointer 1 place right, wrapping around end of buffer
static void buffer_move_ptr_right(buffer_t* const buf) {
    if(buf->full)
        buf->start = (buf->start + 1) % buf->max;
    buf->end = (buf->end + 1) % buf->max;

    buf->full = (buf->start == buf->end);
}


bool buffer_init(buffer_t* const buf, size_t max_elems, size_t elem_size) {
    void* loc = malloc(max_elems*elem_size);
    if (loc == NULL || errno == ENOMEM)
        return false;
    buf->data = loc;
    buf->elem_size = elem_size;
    buf->max = max_elems;
    buffer_reset(buf);
    return true;
}

void buffer_reset(buffer_t* const buf) {
    buf->start = 0;
    buf->end = 0;
    buf->full = false;
}


bool buffer_add(buffer_t* const buf, void* data, bool override) {
    if (!override && buf->full)
        return false;
    void* location = (uint8_t*)buf->data + buf->end * buf->elem_size;
    memcpy(location, data, buf->elem_size);
    buffer_move_ptr_right(buf);
    return true;
}

void* buffer_read(const buffer_t* const buf, size_t index) {
    size_t index_to_move = index % buffer_used_size(buf);
    if (buf->start+index_to_move < buf->max) {
        uint8_t* start = (uint8_t*)buf->data + buf->start * buf->elem_size;
        return start + index_to_move*buf->elem_size;
    }
    else {
        size_t right_side = buf->max - (buf->start);
        return (uint8_t*)buf->data + (index_to_move-right_side)*buf->elem_size;
    }
}

void* buffer_get(buffer_t* const buf) {
    if(buffer_empty(buf))
        return NULL;

    void* data = buffer_read(buf, buf->start);
    void* copy = malloc(buf->elem_size);
    memcpy(copy, data, buf->elem_size);
    buffer_move_ptr_left(buf);
    return copy;
}

size_t buffer_used_size(const buffer_t* const buf) {
    if (buf->full)
        return buf->max;
    if (buf->start < buf->end)
        return buf->end - buf->start;
    else
        return buf->start - buf->end;
}

size_t buffer_free_size(const buffer_t* const buf) {
    return buf->max - buffer_used_size(buf);
}

bool buffer_empty(const buffer_t* const buf) {
    return !buf->full && buf->start == buf->end;
}

bool buffer_full(const buffer_t* const buf) {
    return buf->full;
}

size_t buffer_capacity(const buffer_t* const buf) {
    return buf->max;
}

void buffer_free(const buffer_t* const buf) {
    free(buf->data);
}