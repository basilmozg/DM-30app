
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

#define rtc_read_try_count      65

void RealClockInit(int16_t *modbus_reg_04_, void *EVENT_SEND_);

void RTC_millis_get(int16_t *millis);

void RTC_success();

bool RTC_valid();
