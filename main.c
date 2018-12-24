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
#include <alsa/asoundlib.h>
#include <sys/fcntl.h>

#include "net.h"
#include "sound.h"
#include "application.h"

static const char *const PLAYBACK = "P";
static const char *const CAPTURE = "C";

static void _print_usage_info_and_exit(const char *);
//TODO: Compile command is not being used for vscode. Sometimes re-creating it by hands helps

//!!!!!TODO:!!!!! How to force a compiler to emit warning in case a enumeration constant of one enum type is assigned to another?

//TODO: Configure problem matcher. It does not navigate to the line with a problem in case of warning

//TODO: Configure refactoring

//TODO: CREATE A SHORTCUT FOR peek-definition COMMAND.
//TODO: DISPLAY warnings IN THE PROBLEMS PANEL

//TODO: Ctrl+B does not work properly when terminal focus is on
int main(int argc, char const *argv[])
{
    enum { record, play } mode;
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
                _print_usage_info_and_exit(argv[0]);
       }
    }

    if(device == NULL || file_path == NULL)
        _print_usage_info_and_exit(argv[0]);

    struct connection_config_t *conn_config_ptr = allocate_local("/tmp/test_local_addr");
    struct application_config_t app_config = {
        .conn_config_ptr = conn_config_ptr,
        .mode = mode,
        .device_name = device,
        .file_path = file_path
    };
    int exit_status = run_application(app_config);
    printf("Application exited. Exit code = %d\n", exit_status);
}

static void _print_usage_info_and_exit(const char * binary_name){
    fprintf(stderr, "Usage: %s [-d pcm_device_name] [-m [P]layback | [C]apture] [-f filepath]\n", binary_name);
    exit(1);
}