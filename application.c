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
#include <stdatomic.h>
#include <alsa/asoundlib.h> //TODO do we really need alsa dependency here?
#include <sys/fcntl.h>

#include "application.h"
#include "net.h"
#include "sound.h"
#include "util.h"

struct application_config{
    struct sound_device_config_t *snd_cfg_ptr;
    struct connection_config_t *conn_cfg_ptr;
    enum mode mode;
    const char *file_path;
};

//TODO: Currently support only one client
//TODO: Currently the only _Atomic is client_ptr. Consider declaring all fields either const or _Atomic
struct application{
    struct client_t * _Atomic client_ptr;
    struct server_t *server_ptr;
    //TODO: Consider moving it into sound.h
    enum mode mode;
    int signal_flush_fd;
    union {
        struct sound_device_input_t *input;
        struct sound_device_output_t *output;
    } sound;
};

application_config_t* application_config_allocate(){
    struct sound_device_config_t *snd_cfg_ptr = config_allocate(); //TODO: add snd_ prefix to the config_allocate function
    struct connection_config_t *conn_cfg_ptr = net_allocate_config();

    struct application_config *app_config_ptr = malloc(sizeof(*app_config_ptr));
    app_config_ptr -> snd_cfg_ptr = snd_cfg_ptr;
    app_config_ptr -> conn_cfg_ptr = conn_cfg_ptr;
    return app_config_ptr;
}

void application_config_release(application_config_t *config){
    config_free(config -> snd_cfg_ptr);
    net_release_config(config -> conn_cfg_ptr);
    free(config);
}

void application_set_snd_dev_name(application_config_t *config, const char *dev_name){
    set_device_name(config -> snd_cfg_ptr, dev_name);
}

void application_set_snd_mode(application_config_t *config, enum mode mode){
    config -> mode = mode;
}

void application_set_file_path(application_config_t *config, const char *file_path){
    config -> file_path = file_path;
}

void application_set_rate(application_config_t *config, uint16_t rate){
    set_rate(config -> snd_cfg_ptr, rate);
}

void application_set_channels(application_config_t *config, uint16_t channels){
    set_channels(config -> snd_cfg_ptr, channels);
}

int application_set_sample_format(application_config_t *cfg, enum app_snd_sample_rate app_sample_format){
    enum sample_format format;
    switch (app_sample_format) {
        case app_signed_16bit_little_endian:
            format = signed_16bit_little_endian;
            break;
        default:
            fprintf(stderr, "The %d sample format is not supported\n", app_sample_format);
            return -1;
    }
    set_sample_format(cfg -> snd_cfg_ptr, format);
    return 0;
}

void application_set_periods(application_config_t *config, uint16_t periods){
    set_periods(config -> snd_cfg_ptr, periods);
}

void application_set_perios_size(application_config_t *config, size_t frame_count){
    set_period_size(config -> snd_cfg_ptr, frame_count);
}

void application_set_local_communication(application_config_t *config, const char *local_address){
    net_set_local_communication(config -> conn_cfg_ptr, local_address);
}

void application_set_tcp_communication(application_config_t *config, const char *host_name, uint16_t port, int backlog){
    net_set_tcp_communication(config -> conn_cfg_ptr, host_name, port, backlog);
}

//TODO: Print error msg yeilds too much code duplication
void stop_application(struct application * _Atomic * atomic_app_ptr){
    printf("Stopping application... \n");
    struct application * app_ptr = atomic_load_explicit(atomic_app_ptr, memory_order_acquire);
    struct error_t *error_ptr = NULL;
    if(app_ptr == NULL){
        fprintf(stderr, "Stop is ignored. Application pointer is NULL.\n");
    } else {
        struct client_t *client_ptr = app_ptr -> client_ptr;
        struct server_t *server_ptr = app_ptr -> server_ptr;
        net_close_client(client_ptr, &error_ptr);
        if(error_ptr != NULL){
            print_error_msg(error_ptr, stderr);
        }
        server_ptr -> shutdown(server_ptr, &error_ptr);
        if(error_ptr != NULL){
            print_error_msg(error_ptr, stderr);
        }
        enum mode mode = app_ptr -> mode;
        switch (mode) {
            case play:
                close_output(app_ptr -> sound.output, &error_ptr);
                if(error_ptr != NULL){
                    print_error_msg(error_ptr, stderr);
                }
                break;
            case record:
                close_input(app_ptr -> sound.input, &error_ptr);
                if(error_ptr != NULL){
                    print_error_msg(error_ptr, stderr);
                }
                break;
            default:
                //TODO: Unknown mode is currently logged to stderr. Probaly we have to call abort here as well...
                fprintf(stderr, "Unknown mode %d. Unable to close\n", mode);
                break;
        }
        //TODO: Currently it is simply freed. Probably we need a dedicated 
        //TODO: functions to allocate and release memory pointed to by
        //TODO: running_app pointer
        free(app_ptr);
        atomic_store_explicit(atomic_app_ptr, NULL, memory_order_release);
    }
}

static void install_signal_handlers_(){
    signal(SIGPIPE, SIG_IGN);
}

int run_application(const application_config_t *app_config, struct application * _Atomic * atomic_app_ptr){
    install_signal_handlers_();
    struct application *app_ptr = malloc(sizeof(struct application));
    app_ptr -> client_ptr = NULL;
    app_ptr -> server_ptr = NULL;
    app_ptr -> mode = app_config -> mode;
    const char *const file_path = app_config -> file_path;
    //TODO: Duplicate mode usage. Mode is used for opening sound_device_input/output
    switch (app_ptr -> mode)
    {
        case record:
            app_ptr -> signal_flush_fd = open(file_path, O_WRONLY | O_EXCL | O_CREAT, S_IRUSR);
            break;
        case play:
            app_ptr -> signal_flush_fd = open(file_path, O_RDONLY);
            break;
        default:
            fprintf(stderr, "Unknown mode: %d", app_ptr -> mode);
            return -1; //TODO
    }
    //TODO: This is a really weird way of error handling
    struct error_t *error_ptr = NULL;
    app_ptr -> server_ptr = net_initialize_server_endpoint(app_config -> conn_cfg_ptr, &error_ptr);
    if(error_ptr != NULL){
        print_error_msg(error_ptr, stderr);
        return -1;
    }

    atomic_store_explicit(atomic_app_ptr, app_ptr, memory_order_release);

    struct sound_device_config_t *snd_cfg_ptr = app_config -> snd_cfg_ptr;
    const size_t frames_count = get_frame_size(snd_cfg_ptr);
    const size_t frame_size = get_period_size(snd_cfg_ptr);
    const size_t period_size = frames_count * frame_size;
    char buffer[period_size];
    memset(buffer, '\0', sizeof(buffer));
    // char *const buffer = calloc(frame_size, buffer_frames);
   
    switch(app_ptr -> mode){
        case record: {
            if(error_ptr == NULL){
                while(1){
                    struct server_t *srv_ptr = app_ptr -> server_ptr;
                    //TODO: Probably we need some status to avoid stopping app/awaiting client race
                    struct client_t *client_ptr = srv_ptr -> await_client(srv_ptr, &error_ptr);

                    struct sound_device_input_t *input = open_input(snd_cfg_ptr, &error_ptr);
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
                    while(error_ptr == NULL){
                        unsigned_frames_count frames_captured = capture(input, buffer, period_size, &error_ptr);
                        printf("Captured %lu frames. Writing to file...\n", frames_captured);
                        printf("Data captured: %s\n", buffer);
                        net_send_data(client_ptr, buffer, period_size, &error_ptr);
                        ssize_t written = write(fd, buffer, period_size);
                        printf("Written %ld bytes to file\n", written);
                    }
                    fprintf(stderr, "Error occured while transferring data\n");
                    print_error_msg(error_ptr, stderr);
                    close_input(input, &error_ptr);
                    net_close_client(client_ptr, &error_ptr);
                    clear_error(&error_ptr);
                }
                break;
            } else {
                print_error_msg(error_ptr, stderr);
                return 1;
            }
        }
        case play: {
            int fd = open(file_path, O_RDONLY);
            if(fd <= 0){
                fprintf(stderr, "Error while opening %s. Error code: %d, details: %s\n", file_path, errno, strerror(errno));
                return 1;
            }
            struct sound_device_output_t *output = open_output(snd_cfg_ptr, &error_ptr);
            ssize_t bytes_read = read(fd, buffer, period_size);
            while(bytes_read > 0){
                printf("Bytes read from %s: %ld\n", file_path, bytes_read);
                unsigned_frames_count frames_played_back = playback(output, buffer, frames_count, &error_ptr);
                printf("Frames played back: %lu\n", frames_played_back);
                bytes_read = read(fd, buffer, period_size);
            }
            close_output(output, &error_ptr);
        }
    }
    free(buffer);

    return 0;
}