#include "clock_config.h"
#include "board.h"
#include "fsl_uart.h"
#include "funciones_serial.h"
#include "ds1307.h"
#include "vali_date_hour.h"
#include "textos_menu.h"


#define NUM_CHARS_FECHA_HORA 		8
#define NUM_CHARS_OPT_MENU			1
#define RX_BUFFER_SIZE 				20


//-----Variables Globales-----
volatile uint8_t rx_buff_ndx = 		0;
uint8_t rx_buffer[RX_BUFFER_SIZE];		//Buffer intermedio de recepcion UART.


void reset_uart_vars(void);


void UART0_RX_TX_IRQHandler(void)
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



int main(void)
{
	//Arrancamos con la configuracion de relojes del sistema.
    BOARD_BootClockRUN();

    //Configuracion de periferico UART
    inicia_UART();

    //Inicio de I2C Y módulo ds1307
    init_I2C0();			//Arrancamos I2C
    init_RTC();				//Arrancamos RTC, encendiéndolo en caso de que estuviera apagado.

    //-----Variables locales -----
	typedef enum {menu, config_hora, config_fecha, leer_hora, leer_fecha} estados_maquina;
	estados_maquina estado_maquina = menu;
    struct UART_separado datoUART;
    bool debo_imprimir = true;

    //Activacion de interrupcion de recepcion UART
    UART_EnableInterrupts(UART0, kUART_RxDataRegFullInterruptEnable | kUART_RxOverrunInterruptEnable);
    EnableIRQ(UART0_RX_TX_IRQn);


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
			{
				if (rx_buff_ndx >= NUM_CHARS_FECHA_HORA){
					datoUART = separar_UART(rx_buffer, NUM_CHARS_FECHA_HORA, ':');
					if (datoUART.numFields == 3 && is_hour_valid(datoUART.field[0], datoUART.field[1],datoUART.field[2])){
						uint8_t hr  = 0B00000000 | (datoUART.field_digit[1] & 0B00001111) | ((datoUART.field_digit[0] & 0B00000011) << 4);
						uint8_t min = 0B00000000 | (datoUART.field_digit[3] & 0B00001111) | ((datoUART.field_digit[2] & 0B00000111) << 4);
						uint8_t sec = 0B00000000 | (datoUART.field_digit[5] & 0B00001111) | ((datoUART.field_digit[4] & 0B00000111) << 4);
						set_hour_date_ds1307_i2c(hr , min , sec, HOUR);
						UART_WriteBlocking(UART0, str_hora_configurada_exito, sizeof(str_hora_configurada_exito) / sizeof(str_hora_configurada_exito[0]));
					}
					else {
						UART_WriteBlocking(UART0, str_entrada_incorrecta, sizeof(str_entrada_incorrecta) / sizeof(str_entrada_incorrecta[0]));
					}
					estado_maquina = menu;
					debo_imprimir = true;
					reset_uart_vars();
				}
			}
			break;


			case config_fecha:
				if (rx_buff_ndx >= NUM_CHARS_FECHA_HORA){
					datoUART = separar_UART(rx_buffer, NUM_CHARS_FECHA_HORA, '/');
					if (datoUART.numFields == 3 && is_date_valid(datoUART.field[0], datoUART.field[1],datoUART.field[2])){
						uint8_t day   = 0B00000000 | (datoUART.field_digit[1] & 0B00001111) | ((datoUART.field_digit[0] & 0B00000011) << 4);
						uint8_t month = 0B00000000 | (datoUART.field_digit[3] & 0B00001111) | ((datoUART.field_digit[2] & 0B00000001) << 4);
						uint8_t year  = 0B00000000 | (datoUART.field_digit[5] & 0B00001111) | ((datoUART.field_digit[4] & 0B00001111) << 4);
						set_hour_date_ds1307_i2c(year, month , day , DATE);
						UART_WriteBlocking(UART0, str_fecha_configurada_exito, sizeof(str_fecha_configurada_exito) / sizeof(str_fecha_configurada_exito[0]));
					}
					else {
						UART_WriteBlocking(UART0, str_entrada_incorrecta, sizeof(str_entrada_incorrecta) / sizeof(str_entrada_incorrecta[0]));
					}
					estado_maquina = menu;
					debo_imprimir = true;
					reset_uart_vars();
				}
			break;


			case leer_hora:
			{
				uint8_t str_hour[NUM_CHARS_FECHA_HORA + 1] = {'\0'};			// + 1 para resevar espacio para el NULL TERMINATOR (fin de string)
				if (read_hour_ds1307_i2c(str_hour) ){
					UART_WriteBlocking(UART0, str_hour, sizeof(str_hour) / sizeof(str_hour[0]));
				}
				else {
					UART_WriteBlocking(UART0, str_comm_failed, sizeof(str_comm_failed) / sizeof(str_comm_failed[0]));
				}
				estado_maquina = menu;
				debo_imprimir = true;
				reset_uart_vars();
			}
			break;


			case leer_fecha:
			{
				uint8_t str_date[NUM_CHARS_FECHA_HORA + 1] = {'\0'};			// + 1 para resevar espacio para el NULL TERMINATOR (fin de string)
				if (read_date_ds1307_i2c(str_date) ){
					UART_WriteBlocking(UART0, str_date, sizeof(str_date) / sizeof(str_date[0]));
				}
				else {
					UART_WriteBlocking(UART0, str_comm_failed, sizeof(str_comm_failed) / sizeof(str_comm_failed[0]));
				}
				estado_maquina = menu;
				debo_imprimir = true;
				reset_uart_vars();
			}
			break;


			default:
				UART_WriteBlocking(UART0, str_opc_incorrecta, sizeof(str_opc_incorrecta) / sizeof(str_opc_incorrecta[0]));
				estado_maquina = menu;
				debo_imprimir = true;
				reset_uart_vars();
			break;


			}//-----Cierre de case maquina de estados
    }//-----Cierre de while infinito
}//-----Cierre de main



void reset_uart_vars(void){
	for(uint8_t i = 0; i<RX_BUFFER_SIZE; i++){
		rx_buffer[i] = '\0';
	}
	rx_buff_ndx=0;
}
