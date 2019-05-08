#ifndef PLAYER
#define PLAYER

#include <alsa/asoundlib.h>
#include "buffer/buffer.h"

typedef struct {
    snd_pcm_t* snd_handle;
    buffer_t* buffer;
} player_t;

// Initialize a player_t
void player_init(player_t* const player, const size_t max_elems, const size_t elem_size);

// Play some music before returning
// NOTE: function remains in control if music buffer is full
void player_play(player_t* const player);

// Free a player_t
void player_free(player_t* const player);

#endif