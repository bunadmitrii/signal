#ifndef SOUND_H
#define SOUND_H

#include "config.h"
#include <unistd.h>

typedef size_t unsigned_frames_count;
struct sound_device_input_t;
struct sound_device_output_t;

//capture
struct sound_device_input_t *open_input(const char*, struct sound_device_config_t*);

unsigned_frames_count capture(struct sound_device_input_t *sds, char * buffer, unsigned_frames_count frames);

//TODO: make return type to be an error-code
void close_input(struct sound_device_input_t*);

//playback
struct sound_device_output_t *open_output(const char*, struct sound_device_config_t*);

unsigned_frames_count playback(struct sound_device_output_t *sds, char * buffer, unsigned_frames_count frames);

void close_output(struct sound_device_output_t*);

#endif //SOUND_H