//for string.h::strdup to be available
#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <alsa/asoundlib.h>
#include <sys/fcntl.h>

#include "application.h"
#include "net.h"
#include "sound.h"
#include "util.h"

static const unsigned_frames_count period_size = 2048;
static const unsigned int channels = 2;
static void _install_signal_handlers();

int run_application(struct application_config_t app_config){
    _install_signal_handlers();
    struct error_t *error = NULL;

    const enum mode mode = app_config.mode;
    const char *const file_path = app_config.file_path;
    const char *const device = app_config.device_name;

    struct sound_device_config_t *const cfg_ptr = config_allocate();
    set_rate(cfg_ptr, 44100);
    set_channels(cfg_ptr, channels);
    set_sample_format(cfg_ptr, signed_16bit_little_endian);
    set_periods(cfg_ptr, 2);
    set_period_size(cfg_ptr, period_size);

    const size_t frame_size = get_frame_size(cfg_ptr);
    const size_t buffer_frames = period_size;
    char *const buffer = calloc(frame_size, buffer_frames); 
   
    switch(mode){
        case record: {
            struct server_endpoint_t *srv_endpoint_ptr = initialize_server_endpoint(app_config.conn_config_ptr, &error);
            if(error == NULL){
                while(1){
                    struct connection_t *connection_ptr = await_connection(srv_endpoint_ptr, &error);

                    struct sound_device_input_t *input = open_input(device, cfg_ptr, &error);
                    int fd = open(file_path, O_WRONLY | O_EXCL | O_CREAT, S_IRUSR);
                    //TODO: We should release all the resources properly in case the file cannot be opened
                    if(fd <= 0){
                        if(errno == EEXIST){
                            fprintf(stderr, "File %s already exists\n", file_path);
                        } else {
                            fprintf(stderr, "Error while opening file %s. Error code: %d, error details: %s\n", file_path, errno, strerror(errno));
                        }
                        return 1;
                    }
                    //TODO: Currently error is not set. This may cause application to work incorrectly
                    while(error == NULL){
                        unsigned_frames_count frames_captured = capture(input, buffer, period_size, &error);
                        printf("Captured %lu frames. Writing to file...\n", frames_captured);
                        printf("Data captured: %s\n", buffer);
                        send_data(connection_ptr, buffer, buffer_frames * frame_size, &error);
                        ssize_t written = write(fd, buffer, buffer_frames * frame_size);
                        printf("Written %ld bytes to file\n", written);
                    }
                    fprintf(stderr, "Error occured while transferring data\n");
                    print_error_msg(error, stderr);
                    close_input(input, &error);
                    close_client_endpoint(connection_ptr -> client_endpoint_ptr, &error); 
                    clear_error(&error);
                }
                break;
            } else {
                print_error_msg(error, stderr);
                return 1;
            }
        }
        case play: {
            int fd = open(file_path, O_RDONLY);
            if(fd <= 0){
                fprintf(stderr, "Error while opening %s. Error code: %d, details: %s\n", file_path, errno, strerror(errno));
                return 1;
            }
            struct sound_device_output_t *output = open_output(device, cfg_ptr, &error);
            ssize_t bytes_read = read(fd, buffer, buffer_frames * frame_size);
            while(bytes_read > 0){
                printf("Bytes read from %s: %ld\n", file_path, bytes_read);
                unsigned_frames_count frames_played_back = playback(output, buffer, buffer_frames, &error);
                printf("Frames played back: %lu\n", frames_played_back);
                bytes_read = read(fd, buffer, buffer_frames * frame_size);
            }
            close_output(output, &error);
        }
    }
    free(buffer);

    return 0;
}

static void _install_signal_handlers(){
    signal(SIGPIPE, SIG_IGN);
}