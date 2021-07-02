

#ifndef DS1307_H_
#define DS1307_H_

#include "stdint.h"
#include "stdbool.h"

typedef enum {HOUR = 0x00, DATE = 0x04} i2c_subaddresses;

void init_I2C0(void);
void init_RTC(void);
void set_hour_date_ds1307_i2c(uint8_t hr_fn, uint8_t min_fn, uint8_t sec_fn, uint8_t i2C_subaddress);
bool read_hour_ds1307_i2c(uint8_t * full_hour);
bool read_date_ds1307_i2c(uint8_t * full_date);


#endif /* DS1307_H_ */
