#ifndef BUFFER
#define BUFFER

#include <stdbool.h>
#include <stdlib.h>

// Intel from: 
// https://embeddedartistry.com/blog/2017/4/6/circular-buffers-in-cc
// https://en.wikipedia.org/wiki/Circular_buffer

/** 
 * Simple ring buffer implementation
 * Allows for a 'sliding window' in buffer
 **/

typedef struct {
    void* data;       //all data
    size_t start;     //start of valid data to read
    size_t end;       //end of valid data to read
    size_t elem_size; //size of one element in buffer
    size_t max;       //amount of elements that could be stored
    bool full;
} buffer_t;

// Make a buffer with (elem_max+1)*elem_size bytes
// Returns true on success, false otherwise
bool buffer_init(buffer_t* const buf, size_t max_elems, size_t elem_size);

// Resets a buffer (making it empty)
void buffer_reset(buffer_t* const buf);

// Adds an element 'data' to the buffer (data must have correct size!)
// Returns true on success, false otherwise
bool buffer_add(buffer_t* const buf, void* data, bool override);

// Returns pointer to element at specified index
void* buffer_read(const buffer_t* const buf, size_t index);

// Returns one element from the ringbuffer, then removes element
void* buffer_get(buffer_t* const buf);

// Returns amount of elements used in buffer
size_t buffer_used_size(const buffer_t* const buf);

// Returns current amount (of elements) of free space
size_t buffer_free_size(const buffer_t* const buf);

// Returns true if buffer is empty, false otherwise
bool buffer_empty(const buffer_t* const buf);

// Returns true if buffer is full, false otherwise
bool buffer_full(const buffer_t* const buf);

// Returns maximum capacity buffer can hold
size_t buffer_capacity(const buffer_t* const buf);

void buffer_free(const buffer_t* const buf);

#endif