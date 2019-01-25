#include <alsa/asoundlib.h>
#include <unistd.h>
#include <stdio.h>

#include "sound.h"
#include "internal/types.h"

enum mode {
    capture_,
    playback_
};

struct sound_device_input_t{
    snd_pcm_t *const pcm_handle;
};

struct sound_device_output_t{
    snd_pcm_t *const pcm_handle;
};

static void _configure_device(snd_pcm_t **pcm_handle_ptr, const char *sd, enum mode mode, sound_device_config_t* cfg);

//capture
struct sound_device_input_t *open_input(const char* sd, sound_device_config_t* sd_cfg){
    snd_pcm_t *pcm_handle;
    _configure_device(&pcm_handle, sd, capture_, sd_cfg);
    struct sound_device_input_t *sdi = malloc(sizeof(*sdi));
    struct sound_device_input_t tmp = {.pcm_handle = pcm_handle};
    memcpy(sdi, &tmp, sizeof(tmp));
    return sdi;
}

unsigned_frames_count capture(struct sound_device_input_t *sds, char * buffer, unsigned_frames_count frames){
    ssize_t pcm_capture_return;
    snd_pcm_t *pcm_handle = sds -> pcm_handle;
    //TODO: Do we need to clear the buffer in case of failure?
    while((pcm_capture_return = snd_pcm_readi(pcm_handle, buffer, frames)) < 0){
        fprintf(stderr, "Buffer overrun! Details: %s\n", snd_strerror(pcm_capture_return));
        snd_pcm_prepare(pcm_handle);
    }
    return pcm_capture_return;
}

void close_input(struct sound_device_input_t* sdi){
    snd_pcm_close(sdi -> pcm_handle);
    free(sdi);
}
//end capture

//playback
struct sound_device_output_t *open_output(const char* sd_name, sound_device_config_t* sd_cfg){
    snd_pcm_t *pcm_handle;
    _configure_device(&pcm_handle, sd_name, playback_, sd_cfg);
    struct sound_device_output_t *sdo = malloc(sizeof(sdo));
    struct sound_device_output_t tmp = {.pcm_handle = pcm_handle};
    memcpy(sdo, &tmp, sizeof(tmp));
    return sdo;
}

unsigned_frames_count playback(struct sound_device_output_t *sds, char * buffer, unsigned_frames_count frames){
    ssize_t pcm_return;
    snd_pcm_t *pcm_handle = sds -> pcm_handle;
    //TODO: Probably in case of failure we need to playback another buffer, not this one
    while((pcm_return = snd_pcm_writei(pcm_handle, buffer, frames)) < 0){
        fprintf(stderr, "Error while playing back. PCM device will be prepared. Error code = %ld, details = %s\n", pcm_return, snd_strerror(pcm_return));
        snd_pcm_prepare(pcm_handle);
    }
    return pcm_return;
}

void close_output(struct sound_device_output_t* sdo){
    snd_pcm_close(sdo -> pcm_handle);
    free(sdo);
}

//TODO: replace void with error-code return type
static void _configure_device(snd_pcm_t **pcm_handle_ptr, const char *sd_name, enum mode mode, sound_device_config_t* cfg){ 
    snd_pcm_hw_params_t *hwparams = NULL;
    snd_pcm_stream_t stream;
    switch(mode){
        case capture_:
            stream = SND_PCM_STREAM_CAPTURE;
            break;
        case playback_:
            stream = SND_PCM_STREAM_PLAYBACK;
            break;
        default:
            fprintf(stderr, "Unknown stream mode\n");
            return;
    }
    snd_pcm_hw_params_alloca(&hwparams);
    const char * device_name = sd_name;
    
    int error_code;

    if((error_code = snd_pcm_open(pcm_handle_ptr, device_name, stream, 0)) < 0){
        fprintf(stderr, "Cannot open pcm device with name %s. Details: %s\n", device_name, snd_strerror(error_code));
        return;
    } else {
        printf("Pcm device %s opened\n", device_name);
    }

    snd_pcm_t *pcm_handle = *pcm_handle_ptr;

    if((error_code = snd_pcm_hw_params_any(pcm_handle, hwparams)) < 0){
        fprintf(stderr, "Cannot initialize hwparams with full configuration space. Details: %s\n", snd_strerror(error_code));
    } else {
        printf("Params are initialized with full configuration space\n");
    }

    if((error_code = snd_pcm_hw_params_set_access(pcm_handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0){
       fprintf(stderr, "Error setting access. Details: %s\n", snd_strerror(error_code));
    } else {
        printf("Setting access to SND_PCM_ACCESS_RW_INTERLEAVED: %s\n", snd_strerror(error_code));
    }

    if((error_code = snd_pcm_hw_params_set_format(pcm_handle, hwparams, cfg -> sample_format)) < 0){
        fprintf(stderr, "Cannot set sample format. Details: %s\n", snd_strerror(error_code));
    } else {
        printf("Sample format is initialized to %d\n", cfg -> sample_format);
    }

    {
        int direction = 0;
        unsigned int exact_rate = cfg -> rate;
        if((error_code = snd_pcm_hw_params_set_rate_near(pcm_handle, hwparams, &exact_rate, &direction)) < 0){
            fprintf(stderr, "Cannot set sample rate to %d. Details: %s\n", cfg -> rate, snd_strerror(error_code));
            return;
        } else if(exact_rate != cfg -> rate) {
            printf("Exact rate %d differs from the rate configured %d\n", exact_rate, cfg -> rate);
        } else {
            printf("Rate set to %d\n", exact_rate);
        }
    }

    if((error_code = snd_pcm_hw_params_set_channels(pcm_handle, hwparams, cfg -> channels)) < 0){
        fprintf(stderr, "Cannot set the number of channels to %d. Details: %s", cfg -> channels, snd_strerror(error_code));
        return;
    } else {
        printf("Number of channels is set to %d\n", cfg -> channels);
    }

    if((error_code = snd_pcm_hw_params_set_periods(pcm_handle, hwparams, cfg -> periods, 0)) < 0){
         fprintf(stderr, "Cannot set periods to %d. Details: %s\n", cfg -> periods, snd_strerror(error_code));
         return;
    } else {
        printf("Periods is set to %d\n", cfg -> periods);
    }

    {
        const int buffer_size = cfg -> period_size * cfg -> periods;
        if((error_code = snd_pcm_hw_params_set_buffer_size(pcm_handle, hwparams, buffer_size)) < 0){
            fprintf(stderr, "Cannot set buffer size to %d frames. Details: %s\n", buffer_size, snd_strerror(error_code));
            return;
        } else {
            printf("Buffer size is set to %d periods\n", buffer_size);
        }
    }

    if((error_code = snd_pcm_hw_params(pcm_handle, hwparams)) < 0){
        fprintf(stderr, "Cannot set the configured params. Details: %s\n", snd_strerror(error_code));
        return;
    } else {
        printf("Params are successfully applied to the pcm device\n");
    }
}
