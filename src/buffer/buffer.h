#include <stdbool.h>
#include <stdlib.h>

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

bool buf_init(buffer* buf, size_t max_elems, size_t elem_size);

void buf_reset(buffer* buf);

bool buf_add(buffer* buf, void* data, bool override);

void* buf_read(buffer* buf, size_t index);

size_t buf_used_size(const buffer* buf);

size_t buf_free_size(const buffer* buf);

bool buf_empty(const buffer* buf);

bool buf_full(const buffer* buf);