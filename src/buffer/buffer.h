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
} buffer;

// Make a buffer with (elem_max+1)*elem_size bytes
// Returns true on success, false otherwise
bool buf_init(buffer* const buf, size_t max_elems, size_t elem_size);

// Resets a buffer (making it empty)
void buf_reset(buffer* const buf);

// Adds an element 'data' to the buffer (data must have correct size!)
// Returns true on success, false otherwise
bool buf_add(buffer* const buf, void* data, bool override);

//Returns pointer to element at specified index
void* buf_read(const buffer* const buf, size_t index);

// Returns amount of elements used in buffer
size_t buf_used_size(const buffer* const buf);

// Returns current amount of free space
size_t buf_free_size(const buffer* const buf);

// Returns true if buffer is empty, false otherwise
bool buf_empty(const buffer* const buf);

// Returns true if buffer is full, false otherwise
bool buf_full(const buffer* const buf);

// Returns maximum capacity buffer can hold
size_t buf_capacity(const buffer* const buf);