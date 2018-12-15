//for string.h::strdup to be available
#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif

#include "net.h"
#include "sound.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <sys/fcntl.h>

static const char *const PLAYBACK = "P";
static const char *const CAPTURE = "C";

static const unsigned_frames_count period_size = 2048;
static const unsigned int channels = 2;

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
    struct connection_config_t config_ptr = {.type = local, .port = -1, .host = "/tmp/test_local_addr"};
    connection * connection_ptr;
    await_connection(&connection_ptr, &config_ptr);
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


    sound_device_config_t *cfg_ptr = config_allocate();
    set_rate(cfg_ptr, 44100);
    set_channels(cfg_ptr, channels);
    set_sample_format(cfg_ptr, signed_16bit_little_endian);
    set_periods(cfg_ptr, 2);
    set_period_size(cfg_ptr, period_size);

    size_t frame_size = get_frame_size(cfg_ptr);
    size_t buffer_frames = period_size;
    char * buffer = calloc(frame_size, buffer_frames);

    switch(mode){
        case record: {
            sound_device_input_t *input = open_input(device, cfg_ptr);
            int fd = open(file_path, O_WRONLY | O_EXCL | O_CREAT, S_IRUSR);
            if(fd <= 0){
                if(errno == EEXIST){
                    fprintf(stderr, "File %s already exists\n", file_path);
                } else {
                    fprintf(stderr, "Error while opening file %s. Error code: %d, error details: %s\n", file_path, errno, strerror(errno));
                }
                exit(1);
            }
            while(1){
                unsigned_frames_count frames_captured = capture(input, buffer, period_size);
                printf("Captured %d frames. Writing to file...\n", frames_captured);
                printf("Data captured: %s\n", buffer);
                ssize_t written = write(fd, buffer, buffer_frames * frame_size);
                printf("Written %d bytes to file\n", written);
            }
            break;
        }
        case play: {
            int fd = open(file_path, O_RDONLY);
            if(fd <= 0){
                fprintf(stderr, "Error while opening %s. Error code: %d, details: %s\n", file_path, errno, strerror(errno));
                exit(1);
            }
            sound_device_output_t *output = open_output(device, cfg_ptr);
            ssize_t bytes_read = read(fd, buffer, buffer_frames * frame_size);
            while(bytes_read > 0){
                printf("Bytes read from %s: %d\n", file_path, bytes_read);
                unsigned_frames_count frames_played_back = playback(output, buffer, buffer_frames);
                printf("Frames played back: %d\n", frames_played_back);
                bytes_read = read(fd, buffer, buffer_frames * frame_size);
            }
            close_output(output);
        }
    }
    free(buffer);

    return 0;
}

static void _print_usage_info_and_exit(const char * binary_name){
    fprintf(stderr, "Usage: %s [-d pcm_device_name] [-m [P]layback | [C]apture] [-f filepath]\n", binary_name);
    exit(1);
}