#ifndef PLAYER
#define PLAYER

#include <alsa/asoundlib.h>
#include "buffer/buffer.h"

typedef struct {
    snd_pcm_t* snd_handle;
    buffer* buffer;
} player_t;

void player_init(player_t* const player, const size_t max_elems, const size_t elem_size);

void player_run(player_t* const player);

void player_free(player_t* const player);

#endif