#include "vali_date_hour.h"



bool is_hour_valid(uint8_t hours, uint8_t mins, uint8_t secs ){
	bool valid = true;
	if (hours > 23 || mins > 59 || secs > 59 ) valid = false;
	return valid;
}



bool is_date_valid(uint16_t day, uint16_t month, uint16_t year){
	year +=1900;
    uint8_t days_of_the_month[] = {31, 28, 31, 30,31, 30, 31, 31, 30, 31, 30, 31};
    if((year % 4 == 0 && year % 100 != 0) || year % 400 == 0){
        days_of_the_month[1]= 29;
    }
    if(month < 1 || month > 12){
        return false;
    }
    if(day <= 0 || day > days_of_the_month[month-1]){
        return false;
    }
    return true;
}
