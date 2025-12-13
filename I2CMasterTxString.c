/*
 * I2CMasterTxString.c
 *
 *  Created on: Oct 22, 2025
 *      Author: chikamsoh
 */
#include "STM32F44xx__driver.h"
#include "STM32F44xxgpio__driver.h"
#include "stm32f446xx_i2c_driver.h"
#include <string.h>

#define MYADDR	0x61
#define SLAVEADDR 0x42
#define delay for(uint32_t i = 0; i < 250000; i++)


int main(void){

	I2C_handle_t I2CBus;

	char message[] = "Hello world\0";
	uint8_t length = 0;

	//Set up the button
	gpio_handle_t pin;


	memset(&pin, 0, sizeof(gpio_handle_t)); //Setting all internal structure values to zero

	pin.gpioReg = GPIOB;
	GPIO_PeriClockControl(pin.gpioReg, ENABLE);//Enable the peripheral clock for GPIOB


	//Initialize the button(PB6:D10)
	pin.gpioConfigure.MODE = GPIO_MODE_INPUT;
	pin.gpioConfigure.OSPEED = GPIO_LOW_SPEED;
	pin.gpioConfigure.OTYPE = GPIO_PP;
	pin.gpioConfigure.PUPD = GPIO_PU;
	pin.gpioConfigure.pinNumber = GPIO_PIN_NO_6;
	gpioInit(&pin);

	//Initialize the SDA line(PB9:D14)
	pin.gpioConfigure.MODE = GPIO_MODE_ALFN;
	pin.gpioConfigure.OTYPE = GPIO_OD;
	pin.gpioConfigure.PUPD = GPIO_NO_PUPD;
	pin.gpioConfigure.pinNumber = GPIO_PIN_NO_9;
	pin.gpioConfigure.ALTF = AF4;
	gpioInit(&pin);

	//Initialize the SCL line(PB8:D15)
	pin.gpioConfigure.pinNumber = GPIO_PIN_NO_8;
	gpioInit(&pin);

	//Set up the I2C peripheral
	memset(&I2CBus, 0, sizeof(I2C_handle_t)); //Setting all internal structure values to zero

	I2CBus.pI2Cx = I2C1;
	I2C_PeriClockControl(I2CBus.pI2Cx, ENABLE); //Enable the peripheral clock for I2C1

	//Initialize the I2C bus
	I2CBus.i2cConfigure.ackControl = I2C_ACK_EN;
	I2CBus.i2cConfigure.deviceAddress = MYADDR;
	I2CBus.i2cConfigure.fmDutyCycle = Fm_DC1;
	I2CBus.i2cConfigure.sclSpeed = I2C_SCL_SPEED_SM;
	I2C_Init(&I2CBus);

	length = (uint8_t)strlen(message);
	I2CEnable(I2CBus.pI2Cx, ENABLE);

	while(1){
		while((GPIO_ReadFromInputPin(GPIOB, GPIO_PIN_NO_6)));
		delay;
		I2C_MasterSendData(&I2CBus, (uint8_t*)message, length, SLAVEADDR, I2C_SR_DISABLE);
	}

}




