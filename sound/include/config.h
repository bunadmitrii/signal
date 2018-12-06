#ifndef CONFIG_H
#define CONFIG_H

#include <unistd.h>

enum sample_format {
    signed_16bit_little_endian
};
typedef struct sound_device_config_t sound_device_config_t;
typedef size_t unsigned_frames_count;
sound_device_config_t *config_allocate();
void config_free(sound_device_config_t*);

void set_rate(sound_device_config_t *config, unsigned int rate);

void set_periods(sound_device_config_t *config, unsigned int periods);

void set_channels(sound_device_config_t *config, unsigned int channels);

void set_period_size(sound_device_config_t *config, unsigned_frames_count period_size);

void set_sample_format(sound_device_config_t *config, enum sample_format sample_format);

ssize_t get_frame_size(sound_device_config_t *);

#endif //CONFIG_H