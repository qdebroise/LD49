#ifndef AUDIO_H_
#define AUDIO_H_

// @Todo: can we find a way to determine a WAV file duration programatically and loop background
// musics ?

struct audio_system_o;

// @Note: the enum values must be sequential starting at 0 because they map to an array. This makes
// things quicker for me as it avoids creating an id to sound mapping.
enum AudioEntry
{
    AUDIO_ENTRY_EMIT_NEUTRON = 0,
    AUDIO_ENTRY_ATOM_STABLE,

    _AUDIO_ENTRY_COUNT, // This *MUST* appear last in the enum.
};

struct audio_system_o* audio_system_create(void);
void audio_system_destroy(struct audio_system_o*);
void audio_system_play_sound(const struct audio_system_o*, enum AudioEntry);

#endif // AUDIO_H_


