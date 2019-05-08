#include "buffer/buffer.h"
#include "player.h"

#define NUM_CHANNELS 2
#define SAMPLE_RATE 44100
#define BLOCK_SIZE 1024
#define FRAME_SIZE 4

// Returns whether 
static bool player_open_audio_device(player_t* const player) {
    int err = 0;
    if ((err = snd_pcm_open(&(player->snd_handle), "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf(stderr, "couldnt open audio device: %s\n", snd_strerror(err));
        return false;
    }

    // Configure parameters of PCM output
    err = snd_pcm_set_params(
         player->snd_handle,
         SND_PCM_FORMAT_S16_LE,
         SND_PCM_ACCESS_RW_INTERLEAVED,
         NUM_CHANNELS,
         SAMPLE_RATE,
         0,              // Allow software resampling
         500000);        // 0.5 seconds latency
    if (err < 0) {
        printf("couldnt configure audio device: %s\n", snd_strerror(err));
        return false;
    }
    return true;
}

void player_init(player_t* const player, const size_t max_elems, const size_t elem_size) {
    player->snd_handle = NULL;
    player->buffer = malloc(sizeof(buffer));
    buf_init(player->buffer, max_elems, elem_size);
}

void player_run(player_t* const player) {
    int i = 0;
    unsigned blocksize = 0;
    uint8_t* play_ptr;

    while (true) {
        sleep(1);
        if (i <= 0) {
            /* TODO: get sample */

            play_ptr = (uint8_t*) player->buffer;
            i = blocksize;
        }

        /* write frames to ALSA */
        snd_pcm_sframes_t frames = snd_pcm_writei(player->snd_handle, play_ptr, (blocksize - (*play_ptr - *(uint8_t*) player->buffer)) / FRAME_SIZE);

        /* Check for errors */
        int ret = 0;
        if (frames < 0)
            ret = snd_pcm_recover(player->snd_handle, frames, 0);
        if (ret < 0) {
            fprintf(stderr, "ERROR: Failed writing audio with snd_pcm_writei(): %i\n", ret);
            exit(EXIT_FAILURE);
        }
        if (frames > 0 && frames < (blocksize - (*play_ptr - *(uint8_t*) player->buffer)) / FRAME_SIZE)
            printf("Short write (expected %i, wrote %li)\n",
                (int) (blocksize - (*play_ptr - *(uint8_t*) player->buffer)) / FRAME_SIZE, frames);

        /* advance pointers accordingly */
        if (frames > 0) {
            play_ptr += frames * FRAME_SIZE;
            i -= frames * FRAME_SIZE;
        }

        if ((unsigned)(*play_ptr - *(uint8_t*) player->buffer) == blocksize)
            i = 0;


        /* TODO: try to receive a block from the server? */

    }
}

void player_free(player_t* const player) {
    snd_pcm_drain(player->snd_handle);
    snd_pcm_hw_free(player->snd_handle);
    snd_pcm_close(player->snd_handle);
    free(player->buffer);
}