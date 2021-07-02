#ifndef VALI_DATE_HOUR_H_
#define VALI_DATE_HOUR_H_

#include "stdint.h"
#include "stdbool.h"


bool is_hour_valid(uint8_t hours, uint8_t mins, uint8_t secs );
bool is_date_valid(uint16_t day, uint16_t month, uint16_t year);

#endif /* VALI_DATE_HOUR_H_ */
