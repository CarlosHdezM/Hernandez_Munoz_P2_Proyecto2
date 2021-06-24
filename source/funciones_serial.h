#ifndef FUNCIONES_SERIAL_H_
#define FUNCIONES_SERIAL_H_

#include "board.h"


struct UART_separado{
    uint8_t   numFields;
    int32_t   field[5];
};

struct UART_separado separar_UART(uint8_t *cadena, uint8_t num_chars, uint8_t delim);



#endif /* FUNCIONES_SERIAL_H_ */
