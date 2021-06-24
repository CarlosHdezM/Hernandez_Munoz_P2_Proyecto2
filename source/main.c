#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_uart.h"
#include "stdio.h"		//Desactivar despues
#include "funciones_serial.h"


/* UART instance and clock */
#define DEMO_UART_CLKSRC     UART0_CLK_SRC
#define DEMO_UART_CLK_FREQ   CLOCK_GetFreq(UART0_CLK_SRC)
#define DEMO_UART_IRQn       UART0_RX_TX_IRQn
#define DEMO_UART_IRQHandler UART0_RX_TX_IRQHandler





#define NUM_CHARS_FECHA_HORA 		8
#define NUM_CHARS_OPT_MENU			1
#define RX_BUFFER_SIZE 				20

volatile uint8_t rx_buff_ndx = 		0;
uint8_t rx_buffer[RX_BUFFER_SIZE];

bool debo_imprimir = true;


uint8_t str_menu[5][110] = {"\r\n\r\n Opciones:\t \r\n1) Configurar Hora\r\n2) Configurar Fecha \r\n3) Leer Hora \r\n4)Leer Fecha\r\nIntroduce opcion:\t",
					 	 	"\r \n \r \nIntroducir hora en formato hh:mm:ss \t",
							"\r \n \r \nIntroducir fecha en formato dd:mm:aa \t",
							"\r \n \r \nLa hora actual es \t",
							"\r \n \r \nLa fecha actual es \t"
							};
uint8_t str_fecha_configurada_exito[] = "\r \n \r \n Se ha configurado la fecha exitosamente... \t";
uint8_t str_hora_configurada_exito[] = "\r \n \r \n Se ha configurado la hora exitosamente... \t";
uint8_t str_opc_incorrecta[] = "\r \n \r \n OPCION NO VALIDA... \t";




void DEMO_UART_IRQHandler(void)
{
	uint8_t data;
    if ((kUART_RxDataRegFullFlag | kUART_RxOverrunFlag) & UART_GetStatusFlags(UART0))		/* If new data arrived. */
    {
        data = UART_ReadByte(UART0);
		UART_WriteBlocking(UART0, &data, sizeof(data));			//Yep, efectivamente esta linea aquí es horrible, ya que estoy agregando una función bloqueante dentro de una interrupción, chale.
        if (rx_buff_ndx < RX_BUFFER_SIZE-1){
        	rx_buffer[rx_buff_ndx] = data;
        	rx_buff_ndx++;
        }
    }
    SDK_ISR_EXIT_BARRIER;
}


void reset_uart_vars(void);


int main(void)
{
    uart_config_t config;

    BOARD_InitPins();
    BOARD_BootClockRUN();

    UART_GetDefaultConfig(&config);
    config.baudRate_Bps = BOARD_DEBUG_UART_BAUDRATE;
    config.enableTx     = true;
    config.enableRx     = true;

    UART_Init(UART0, &config, DEMO_UART_CLK_FREQ);


    /* Enable RX interrupt. */
    UART_EnableInterrupts(UART0, kUART_RxDataRegFullInterruptEnable | kUART_RxOverrunInterruptEnable);
    EnableIRQ(DEMO_UART_IRQn);

    //-----Variables locales para maquina de estados-----
	typedef enum {menu, config_hora, config_fecha, leer_hora, leer_fecha} estados_maquina;
	estados_maquina estado_maquina = menu;

    struct UART_separado datoUART;




    while (1)
    {

		if (debo_imprimir){
			UART_WriteBlocking(UART0, &str_menu[estado_maquina][0], sizeof(str_menu[estado_maquina]) / sizeof(str_menu[estado_maquina][0]));
			debo_imprimir = false;
		}

        switch(estado_maquina){
			case menu:
			{
				if (rx_buff_ndx >= NUM_CHARS_OPT_MENU){
					uint8_t opt_selected = rx_buffer[0] - 0x30;
					if (opt_selected >= config_hora && opt_selected <= leer_fecha){					//Comprobación de que se introdujo un valor válido
						estado_maquina = opt_selected;
					}
					else{
						UART_WriteBlocking(UART0, str_opc_incorrecta, sizeof(str_opc_incorrecta) / sizeof(str_opc_incorrecta[0]));
					}
					debo_imprimir = true;
					reset_uart_vars();
				}
			}
			break;


			case config_hora:
				if (rx_buff_ndx >= NUM_CHARS_FECHA_HORA){
					datoUART = separar_UART(rx_buffer, NUM_CHARS_FECHA_HORA, ':');
					if (datoUART.numFields == 3){
						//TODO: Agregar funcion para verificar que la hora ingresada sea valida.
						UART_WriteBlocking(UART0, str_hora_configurada_exito, sizeof(str_hora_configurada_exito) / sizeof(str_hora_configurada_exito[0]));
						//TODO: Agregar funcion para setear hora por I2C en el modulo.
					}
					else {
						//TODO: Enviar por UART mensaje de que la hora ingresada fue incorrecta.
					}
					estado_maquina = menu;
					debo_imprimir = true;
					reset_uart_vars();
				}
			break;


			case config_fecha:
				if (rx_buff_ndx >= NUM_CHARS_FECHA_HORA){
					datoUART = separar_UART(rx_buffer, NUM_CHARS_FECHA_HORA, '/');
					if (datoUART.numFields == 3){
						//TODO: Agregar funcion para verificar que la fecha ingresada sea valida.
						UART_WriteBlocking(UART0, str_fecha_configurada_exito, sizeof(str_fecha_configurada_exito) / sizeof(str_fecha_configurada_exito[0]));
						//TODO: Agregar funcion para setear hora por I2C en el modulo.
					}
					else {
						//TODO: Enviar por UART mensaje de que la hora ingresada fue incorrecta.
					}
					estado_maquina = menu;
					debo_imprimir = true;
					reset_uart_vars();
				}
			break;


			case leer_hora:
				//TODO: Función para consultar hora mediante I2C
				estado_maquina = menu;
				debo_imprimir = true;
				reset_uart_vars();
			break;


			case leer_fecha:
				//TODO: Función para consultar fecha mediante I2C
				estado_maquina = menu;
				debo_imprimir = true;
				reset_uart_vars();
			break;


			default:
				UART_WriteBlocking(UART0, str_opc_incorrecta, sizeof(str_opc_incorrecta) / sizeof(str_opc_incorrecta[0]));
				estado_maquina = menu;
				debo_imprimir = true;
				reset_uart_vars();
			break;


			}
    }//-----Cierre de while infinito
}//-----Cierre de main



void reset_uart_vars(void){
	for(uint8_t i = 0; i<RX_BUFFER_SIZE; i++){
		rx_buffer[i] = '\0';
	}
	rx_buff_ndx=0;
}


