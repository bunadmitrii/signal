#include "config.h"
#include "internal/types.h"

struct sound_device_config_t *config_allocate(){
    return malloc(sizeof(struct sound_device_config_t));
}

void config_free(struct sound_device_config_t* sound_device_config_ptr){
    free(sound_device_config_ptr);
}

void set_rate(struct sound_device_config_t *config, unsigned int rate){
    config -> rate = rate;
}

void set_periods(struct sound_device_config_t *config, unsigned int periods){
    config -> periods = periods;
}

void set_period_size(struct sound_device_config_t *config, unsigned_frames_count period_size){
    config -> period_size = period_size;
}

void set_channels(struct sound_device_config_t *config, unsigned int channels){
    config -> channels = channels;
}

void set_sample_format(struct sound_device_config_t *config, enum sample_format sample_format){
        switch(sample_format){
        case signed_16bit_little_endian:
            config -> sample_format = SND_PCM_FORMAT_S16_LE;
            break;
        default:
            fprintf(stderr, "Unknown sample format\n");
            return;
    }
}

ssize_t get_frame_size(struct sound_device_config_t * config){
    ssize_t frame_size;
    switch(config -> sample_format){
        case SND_PCM_FORMAT_S16_LE:
            frame_size = 2;
            break;
        default:
            fprintf(stderr, "Unknown sample format");
            frame_size = -1;
    }

    if(frame_size >= 0)
        return config -> channels * frame_size;
    else 
        return -1;
}