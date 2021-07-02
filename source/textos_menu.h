#ifndef TEXTOS_MENU_H_
#define TEXTOS_MENU_H_




uint8_t str_menu[5][110] = {"\r\n\r\n Opciones:\t \r\n1) Configurar Hora\r\n2) Configurar Fecha \r\n3) Leer Hora \r\n4)Leer Fecha\r\nIntroduce opcion:\t",
					 	 	"\r \n \r \nIntroducir hora en formato hh:mm:ss \t",
							"\r \n \r \nIntroducir fecha en formato dd/mm/aa \t",
							"\r \n \r \nLa hora actual es \t",
							"\r \n \r \nLa fecha actual es \t"
							};
/*
 * Hay mucho espacio de memoria desaprovechado en este arreglo de dos dimensiones, pero decidí dejarlo de esta forma ya que me pareció interesante tener
 * los textos del menu en un solo arreglo y que se impriman en funcion del estado de la maquina de estados.
*/

uint8_t str_fecha_configurada_exito[] = "\r \n \r \nSe ha configurado la fecha exitosamente... \t";
uint8_t str_hora_configurada_exito[] = "\r \n \r \nSe ha configurado la hora exitosamente... \t";
uint8_t str_entrada_incorrecta[] = "\r \n \r \nEntrada incorrecta. Por favor ingresa la informacion en el formato correcto y rango apropiado.";
uint8_t str_opc_incorrecta[] = "\r \n \r \nOPCION NO VALIDA... \t";
uint8_t str_comm_failed[] = "\r \n \r \nFallo en la operacion. Revisa que el modulo este conectado. \t";




#endif /* TEXTOS_MENU_H_ */
