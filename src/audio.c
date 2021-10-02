#include "audio.h"

#include <SDL2/SDL.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static const char* audio_files[_AUDIO_ENTRY_COUNT] = {
    "assets/sfx/laser.wav",
    "assets/sfx/music.wav",
    "assets/sfx/explosion.wav",
};

typedef struct audio_sample_t audio_sample_t;

struct audio_sample_t
{
    SDL_AudioSpec wav_spec;
    SDL_AudioDeviceID device_handle;
    uint32_t wav_length;
    uint8_t* wav_buffer;
    bool loaded;
};

struct audio_system_o
{
    audio_sample_t samples[_AUDIO_ENTRY_COUNT];
};

struct audio_system_o* audio_system_create(void)
{
    // @Note @Todo: see later about custom allocators.
    struct audio_system_o* system = malloc(sizeof(struct audio_system_o));

    for (uint32_t i = 0; i < (uint32_t)_AUDIO_ENTRY_COUNT; ++i)
    {
        audio_sample_t* sample = &system->samples[i];

        sample->loaded = true;
        if (!SDL_LoadWAV(audio_files[i], &sample->wav_spec, &sample->wav_buffer, &sample->wav_length))
        {
            fprintf(stderr, "Couldn't load '%s': %s\n", audio_files[i], SDL_GetError());
            sample->loaded = false;
        }
        else
        {
            sample->device_handle = SDL_OpenAudioDevice(NULL, 0, &sample->wav_spec, NULL, 0);
            if (sample->device_handle <= 0)
            {
                fprintf(stderr, "Couldn't open audio device: %s\n", SDL_GetError());
            }
        }
    }

    return system;
}

void audio_system_destroy(struct audio_system_o* audio)
{
    assert(audio);

    for (uint32_t i = 0; i < _AUDIO_ENTRY_COUNT; ++i)
    {
        SDL_CloseAudioDevice(audio->samples[i].device_handle);
        SDL_FreeWAV(audio->samples[i].wav_buffer);
    }
}

void audio_play_sound(const struct audio_system_o* audio, enum AudioEntry entry)
{
    assert(audio);
    assert((uint32_t)entry >= 0 && (uint32_t)entry < _AUDIO_ENTRY_COUNT);

    const audio_sample_t* sample = &audio->samples[(uint32_t)entry];

    if (!sample->loaded) fprintf(stderr, "Can't play sound.\n");

    SDL_ClearQueuedAudio(sample->device_handle);
    SDL_QueueAudio(sample->device_handle, sample->wav_buffer, sample->wav_length);
    SDL_PauseAudioDevice(sample->device_handle, 0);
}
