#ifndef SOUND_H
#define SOUND_H

#include "config.h"
#include <unistd.h>

typedef size_t unsigned_frames_count;
typedef struct sound_device_input_t sound_device_input_t;
typedef struct sound_device_output_t sound_device_output_t;

//capture
sound_device_input_t *open_input(const char*, sound_device_config_t*);

unsigned_frames_count capture(sound_device_input_t *sds, char * buffer, unsigned_frames_count frames);

//TODO: make return type to be an error-code
void close_input(sound_device_input_t*);

//playback
sound_device_output_t *open_output(const char*, sound_device_config_t*);

unsigned_frames_count playback(sound_device_output_t *sds, char * buffer, unsigned_frames_count frames);

void close_output(sound_device_output_t*);

#endif //SOUND_H