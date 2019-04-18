#ifndef CONFIG_H
#define CONFIG_H

#include <unistd.h>

struct sound_device_config_t;

enum sample_format {
    signed_16bit_little_endian
};

struct sound_device_config_t *config_allocate();

void config_free(struct sound_device_config_t*);

void set_rate(struct sound_device_config_t *config, unsigned int rate);

void set_periods(struct sound_device_config_t *config, unsigned int periods);

void set_channels(struct sound_device_config_t *config, unsigned int channels);

void set_period_size(struct sound_device_config_t *config, size_t period_size);

void set_sample_format(struct sound_device_config_t *config, enum sample_format sample_format);

void set_device_name(struct sound_device_config_t *config, const char *device_name);

ssize_t get_frame_size(struct sound_device_config_t *);

size_t get_period_size(struct sound_device_config_t *config);

#endif //CONFIG_H