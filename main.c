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
#include <unistd.h>
#include <stdatomic.h>
#include <sys/fcntl.h>

#include "application.h"

static application * _Atomic app_ptr = NULL; //TODO: Atomic?

static const char *const PLAYBACK = "P";
static const char *const CAPTURE = "C";
static const size_t period_size = 2048;
static const uint16_t channels = 2;
static const uint16_t periods = 2;

static void print_usage_info_and_exit_(const char *);
//TODO: Compile command is not being used for vscode. Sometimes re-creating it by hands helps

//!!!!!TODO:!!!!! How to force a compiler to emit warning in case a enumeration constant of one enum type is assigned to another?

//TODO: Configure problem matcher. It does not navigate to the line with a problem in case of warning

//TODO: Configure refactoring

//TODO: CREATE A SHORTCUT FOR peek-definition COMMAND.
//TODO: DISPLAY warnings IN THE PROBLEMS PANEL

//TODO: Ctrl+B does not work properly when terminal focus is on

//TODO: In case we open a new editor group (e.g. with Ctrl+2) what shortcut closes the currently active group?
int main(int argc, char const *argv[])
{
    enum mode mode;
    const char *device = NULL;
    const char *file_path = NULL;
    int opt;
    while((opt = getopt(argc, argv, "d:m:f:")) != -1){
       switch(opt){
            case 'd':
                device = strdup(optarg);
                break;
            case 'm':
                if(strcmp(optarg, CAPTURE) == 0)
                    mode = record;
                else if(strcmp(optarg, PLAYBACK) == 0)
                    mode = play;
                else {
                    fprintf(stderr, "Unknown mode %s. Choose %s for playback or %s for capture\n", optarg, PLAYBACK, CAPTURE);
                    exit(1);
                }
                break;
            case 'f':
                file_path = strdup(optarg);
                break;
            default:
                print_usage_info_and_exit_(argv[0]);
       }
    }

    if(device == NULL || file_path == NULL)
        print_usage_info_and_exit_(argv[0]);

    struct application_config *config_ptr = application_config_allocate();
    application_set_tcp_communication(config_ptr, "127.0.0.1", 5432, 16);
    application_set_snd_dev_name(config_ptr, device);
    application_set_snd_mode(config_ptr, mode);
    application_set_file_path(config_ptr, file_path);
    application_set_rate(config_ptr, 44100);
    application_set_channels(config_ptr, channels);
    enum app_snd_sample_rate rate = app_signed_16bit_little_endian;
    if(application_set_sample_format(config_ptr, rate) == -1){
        fprintf(stderr, "Configuration error. %d sample format is not supported\n", rate);
        exit(EXIT_FAILURE);
    }
    application_set_periods(config_ptr, periods);
    application_set_perios_size(config_ptr, period_size);
    //TODO: How to pass atomic pointer to a function?
    int exit_status = run_application(config_ptr, &app_ptr);
    application_config_release(config_ptr);
    //TODO: Stopping application which might be already stopped
    stop_application(&app_ptr);
    //TODO: We called stop_application which releases the object 
    //TODO: pointed to by app_ptr so we need to set the pointer to NULL
    //TODO: This is extremely weird and must be refactored
    app_ptr = NULL;
    printf("Application exited. Exit code = %d\n", exit_status);
}

static void print_usage_info_and_exit_(const char * binary_name){
    fprintf(stderr, "Usage: %s [-d pcm_device_name] [-m [P]layback | [C]apture] [-f filepath]\n", binary_name);
    exit(1);
}