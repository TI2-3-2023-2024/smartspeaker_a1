#ifndef NTP_H
#define NTP_H

#include <time.h>
#include "lwip/apps/sntp.h"

void ntp_set_timezone(const char *timezone);
void ntp_initialize(void);
void ntp_get_time(char *time_str, size_t max_len);
void ntp_test(void *pvParameters);

#endif /* NTP_H */
