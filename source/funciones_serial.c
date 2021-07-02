#include "funciones_serial.h"
#include "fsl_uart.h"


#define PIN16_IDX                       16u
#define PIN17_IDX                       17u
#define SOPT5_UART0TXSRC_UART_TX      0x00u



struct UART_separado separar_UART(uint8_t *cadena, uint8_t num_chars, uint8_t delim){
    uint8_t ndx_field_digit = 0;
	struct UART_separado temp =
	{
		 1 ,	//numFields
		{0},	//Inicialización de todos los campos de field en 0.
		{0}		//Inicialización de todos los campos de field_digit en 0.
	};
	for(uint8_t i = 0; i<num_chars; i++){
		if(cadena[i]==delim) temp.numFields++;
		else if (cadena[i] >= '0' && cadena[i] <= '9' ){
			temp.field[temp.numFields-1] = temp.field[temp.numFields-1] * 10 + (cadena[i] - 0x30 ) ;
			temp.field_digit[ndx_field_digit] = cadena[i] - 0x30;
			ndx_field_digit++;
		}
	}
	return temp;

}


void config_UART(void){
	  CLOCK_EnableClock(kCLOCK_PortB);                           /* Port B Clock Gate Control: Clock enabled */

	  PORT_SetPinMux(PORTB, PIN16_IDX, kPORT_MuxAlt3);           /* PORTB16 (pin 62) is configured as UART0_RX */
	  PORT_SetPinMux(PORTB, PIN17_IDX, kPORT_MuxAlt3);           /* PORTB17 (pin 63) is configured as UART0_TX */

	  SIM->SOPT5 = ((SIM->SOPT5 &
	    (~(SIM_SOPT5_UART0TXSRC_MASK)))                          /* Mask bits to zero which are setting */
	      | SIM_SOPT5_UART0TXSRC(SOPT5_UART0TXSRC_UART_TX)       /* UART 0 transmit data source select: UART0_TX pin */
	    );
}


void inicia_UART(void){
    config_UART();
    uart_config_t config;
    UART_GetDefaultConfig(&config);
    config.baudRate_Bps = BOARD_DEBUG_UART_BAUDRATE;
    config.enableTx     = true;
    config.enableRx     = true;
    UART_Init(UART0, &config, CLOCK_GetFreq(UART0_CLK_SRC));
}
