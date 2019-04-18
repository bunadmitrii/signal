#ifndef APPLICATION_H
#define APPLICATION_H

#include <stdint.h>

enum mode {
    record, 
    play 
};

enum app_snd_sample_rate{
    app_signed_16bit_little_endian
};

typedef struct application_config application_config_t;

application_config_t* application_config_allocate();
void application_config_release(application_config_t *config);

void application_set_snd_dev_name(application_config_t *config, const char *dev_name);
void application_set_snd_mode(application_config_t *config, enum mode mode);
void application_set_file_path(application_config_t *config, const char *file_path);
void application_set_rate(application_config_t *config, uint16_t rate);
void application_set_channels(application_config_t *config, uint16_t channels);
/**
 * Returns 0 if successfull, -1 if the sample format is not supported
 */
//TODO: Such signature is confusing and error-prone.
//TODO: How can we ever get an error for the predefined sample rates to set to
int application_set_sample_format(application_config_t *cfg, enum app_snd_sample_rate rate);
void application_set_periods(application_config_t *config, uint16_t periods);
void application_set_perios_size(application_config_t *config, size_t frame_count);


void application_set_local_communication(application_config_t *config, const char *local_address);
void application_set_tcp_communication(application_config_t *config, const char *host_name, uint16_t port, int backlog);

typedef struct application application;

int run_application(const application_config_t*, application * _Atomic * app);

void stop_application(application * _Atomic * app);

#endif //APPLICATION_H