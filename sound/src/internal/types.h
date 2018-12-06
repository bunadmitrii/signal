#ifndef INTERNAL_TYPES_H
#define INTERNAL_TYPES_H

#include <alsa/asoundlib.h>

struct sound_device_t{
    const char * device_name;
};

struct sound_device_config_t{
    unsigned int rate;
    unsigned int periods;
    snd_pcm_uframes_t period_size;
    unsigned int channels;
    snd_pcm_format_t sample_format;
};
#endif //INTERNAL_TYPES_H