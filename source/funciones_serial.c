#include "funciones_serial.h"


struct UART_separado separar_UART(uint8_t *cadena, uint8_t num_chars, uint8_t delim){
    struct UART_separado temp;
    temp.numFields = 1;
    temp.field[0] = 0;
    temp.field[1] = 0;
    temp.field[2] = 0;
    temp.field[3] = 0;
    temp.field[4] = 0;
	for(uint8_t i = 0; i<num_chars; i++){
		if(cadena[i]==delim) temp.numFields++;
		else{
			temp.field[temp.numFields-1] = temp.field[temp.numFields-1] * 10 + (cadena[i] - 0x30 ) ;
		}

	}
	return temp;

}
