#include <stdbool.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "buffer.h"

// Helper func: Move pointer 1 place right, wrapping around end of buffer
void buf_move_ptr_right(buffer* const buf) {
    if(buf->full)
        buf->start = (buf->start + 1) % buf->max;
    buf->end = (buf->end + 1) % buf->max;

    buf->full = (buf->start == buf->end);
}
// Helper func: Move pointer 1 place left, wrapping around end of buffer
// Expects to be called for use: removing one item
void buf_move_ptr_left(buffer* const buf) {
    buf->full = false;
    buf->end = (buf->end + 1) % buf->max;
}

bool buf_init(buffer* const buf, size_t max_elems, size_t elem_size) {
    void* loc = malloc(max_elems*elem_size);
    if (loc == NULL || errno == ENOMEM)
        return false;
    buf->data = loc;
    buf->elem_size = elem_size;
    buf->max = max_elems;
    buf_reset(buf);
    return true;
}

void buf_reset(buffer* const buf) {
    buf->start = 0;
    buf->end = 0;
    buf->full = false;
}


bool buf_add(buffer* const buf, void* data, bool override) {
    if (!override && buf->full)
        return false;
    void* location = (uint8_t*)buf->data + buf->end * buf->elem_size;
    memcpy(location, data, buf->elem_size);
    buf_move_ptr_right(buf);
    return true;
}

void* buf_read(const buffer* const buf, size_t index) {
    size_t index_to_move = index % buf_used_size(buf);
    if (buf->start+index_to_move < buf->max) {
        uint8_t* start = (uint8_t*)buf->data + buf->start * buf->elem_size;
        return start + index_to_move*buf->elem_size;
    }
    else {
        size_t right_side = buf->max - (buf->start);
        return (uint8_t*)buf->data + (index_to_move-right_side)*buf->elem_size;
    }
}

size_t buf_used_size(const buffer* const buf) {
    if (buf->full)
        return buf->max;
    if (buf->start < buf->end)
        return buf->end - buf->start;
    else
        return buf->start - buf->end;
}

size_t buf_free_size(const buffer* const buf) {
    return buf->max - buf_used_size(buf);
}

bool buf_empty(const buffer* const buf) {
    return !buf->full && buf->start == buf->end;
}

bool buf_full(const buffer* const buf) {
    return buf->full;
}

size_t buf_capacity(const buffer* const buf) {
    return buf->max;
}

void buf_free(const buffer* buf) {
    free(buf->data);
}