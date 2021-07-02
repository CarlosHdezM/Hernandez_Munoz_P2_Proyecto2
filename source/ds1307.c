#include "ds1307.h"
#include "fsl_i2c.h"
#include "fsl_common.h"
#include "fsl_port.h"

#define PIN_SDA                       	2u
#define PIN_SCL                       	3u
#define I2C_MASTER_SLAVE_ADDR_7BIT 0x68U
#define I2C_BAUDRATE               100000U
#define I2C_DATA_LENGTH            33U



void init_I2C0(void){
	  CLOCK_EnableClock(kCLOCK_PortB);                           /* Port B Clock Gate Control: Clock enabled */
	  CLOCK_EnableClock(kCLOCK_PortE);                           /* Port E Clock Gate Control: Clock enabled */

	  const port_pin_config_t porte24_pin31_config = {
	    kPORT_PullUp,                                            /* Internal pull-up resistor is enabled */
	    kPORT_FastSlewRate,                                      /* Fast slew rate is configured */
	    kPORT_PassiveFilterDisable,                              /* Passive filter is disabled */
	    kPORT_OpenDrainEnable,                                   /* Open drain is enabled */
	    kPORT_LowDriveStrength,                                  /* Low drive strength is configured */
	    kPORT_MuxAlt2,                                           /* Pin is configured as I2C0_SCL */
	    kPORT_UnlockRegister                                     /* Pin Control Register fields [15:0] are not locked */
	  };
	  PORT_SetPinConfig(PORTB, PIN_SDA, &porte24_pin31_config); /* PORTE24 (pin 31) is configured as I2C0_SCL */
	  const port_pin_config_t porte25_pin32_config = {
	    kPORT_PullUp,                                            /* Internal pull-up resistor is enabled */
	    kPORT_FastSlewRate,                                      /* Fast slew rate is configured */
	    kPORT_PassiveFilterDisable,                              /* Passive filter is disabled */
	    kPORT_OpenDrainEnable,                                   /* Open drain is enabled */
	    kPORT_LowDriveStrength,                                  /* Low drive strength is configured */
	    kPORT_MuxAlt2,                                           /* Pin is configured as I2C0_SDA */
	    kPORT_UnlockRegister                                     /* Pin Control Register fields [15:0] are not locked */
	  };
	  PORT_SetPinConfig(PORTB, PIN_SCL, &porte25_pin32_config); /* PORTE25 (pin 32) is configured as I2C0_SDA */


    i2c_master_config_t masterConfig;
    uint32_t sourceClock;

    I2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Bps = I2C_BAUDRATE;

    sourceClock = CLOCK_GetFreq(I2C0_CLK_SRC);
    I2C_MasterInit(I2C0, &masterConfig, sourceClock);
}



void init_RTC(){
	i2c_master_transfer_t masterXfer;
	uint8_t rxBuff[1];

	//Leemos el registro de segundos (00h) para recuperar su valor y modificar unicamente el bit 7 (CH).
	//-----Transmision i2C de lectura
	masterXfer.slaveAddress   = I2C_MASTER_SLAVE_ADDR_7BIT;
	masterXfer.direction      = kI2C_Read;
	masterXfer.subaddress     = 0x00;
	masterXfer.subaddressSize = 1;
	masterXfer.data           = rxBuff;
	masterXfer.dataSize       = 1;
	masterXfer.flags          = kI2C_TransferDefaultFlag;

	//Conservamos el resto de bits, y seteamos en 0 el bit 7 (CH) para activar el RTC en caso de que estuviera apagado.
	rxBuff[0] &= 0B01111111;

	//-----Transmision i2c de escritura (Enviamos rxBuff para aplicar los cambios del paso anterior)
	masterXfer.slaveAddress   = I2C_MASTER_SLAVE_ADDR_7BIT;
	masterXfer.direction      = kI2C_Write;
	masterXfer.subaddress     = 0x00;
	masterXfer.subaddressSize = 1;
	masterXfer.data           = rxBuff;
	masterXfer.dataSize       = 1;
	masterXfer.flags          = kI2C_TransferDefaultFlag;

} //Fin de void init_RTC



void set_hour_date_ds1307_i2c(uint8_t hr_year_fn, uint8_t min_month_fn, uint8_t sec_day_fn, uint8_t i2C_subaddress){
    i2c_master_transfer_t masterXfer;
    uint8_t g_master_txBuff[I2C_DATA_LENGTH] = {sec_day_fn, min_month_fn , hr_year_fn};

	masterXfer.slaveAddress   = I2C_MASTER_SLAVE_ADDR_7BIT;
    masterXfer.direction      = kI2C_Write;
    masterXfer.subaddress     = i2C_subaddress;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = g_master_txBuff;
    masterXfer.dataSize       = 3;
    masterXfer.flags          = kI2C_TransferDefaultFlag;

    bool status_i2c = I2C_MasterTransferBlocking(I2C0, &masterXfer);

}



bool read_hour_ds1307_i2c(uint8_t * full_hour){
    i2c_master_transfer_t masterXfer;
    uint8_t rxBuff[I2C_DATA_LENGTH];
    bool transmission_result = true;

    //-----Transmision i2C
    masterXfer.slaveAddress   = I2C_MASTER_SLAVE_ADDR_7BIT;
    masterXfer.direction      = kI2C_Read;
    masterXfer.subaddress     = 0x00;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = rxBuff;
    masterXfer.dataSize       = 3;
    masterXfer.flags          = kI2C_TransferDefaultFlag;
    transmission_result = I2C_MasterTransferBlocking(I2C0, &masterXfer);

    //-----Armado de cadena de caracteres.
    full_hour[0] = ((rxBuff[2] >> 4) & 0B00000011) + 0x30;
    full_hour[1] = (rxBuff[2] & 0B00001111) + 0x30;
    full_hour[2] = ':';
    full_hour[3] = ((rxBuff[1] >> 4) & 0B00000111) + 0x30;
    full_hour[4] = (rxBuff[1] & 0B00001111) + 0x30;
    full_hour[5] = ':';
    full_hour[6] = ((rxBuff[0] >> 4) & 0B00000111) + 0x30;
    full_hour[7] = (rxBuff[0] & 0B00001111) + 0x30;

    return !transmission_result;
}



bool read_date_ds1307_i2c(uint8_t * full_date){
    i2c_master_transfer_t masterXfer;
    uint8_t rxBuff[I2C_DATA_LENGTH];
    bool transmission_result = true;

    //-----Transmision i2C
    masterXfer.slaveAddress   = I2C_MASTER_SLAVE_ADDR_7BIT;
    masterXfer.direction      = kI2C_Read;
    masterXfer.subaddress     = 0x04;										//Esta linea cambiaria para la fecha
    masterXfer.subaddressSize = 1;
    masterXfer.data           = rxBuff;
    masterXfer.dataSize       = 3;
    masterXfer.flags          = kI2C_TransferDefaultFlag;
    transmission_result = I2C_MasterTransferBlocking(I2C0, &masterXfer);

    //-----Armado de cadena de caracteres.
    full_date[0] = ((rxBuff[0] >> 4) & 0B00000011) + 0x30;
    full_date[1] = (rxBuff[0] & 0B00001111) + 0x30;
    full_date[2] = '/';
    full_date[3] = ((rxBuff[1] >> 4) & 0B00000001) + 0x30;
    full_date[4] = (rxBuff[1] & 0B00001111) + 0x30;
    full_date[5] = '/';
    full_date[6] = ((rxBuff[2] >> 4) & 0B00001111) + 0x30;
    full_date[7] = (rxBuff[2] & 0B00001111) + 0x30;

    return !transmission_result;
}
