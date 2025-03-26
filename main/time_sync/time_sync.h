#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <lwip/apps/sntp.h>

void time_init_from_compile(void);
bool time_sync_with_ntp(const char *ntp_domain);
