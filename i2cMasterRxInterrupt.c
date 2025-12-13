/*
 * i2cMasterRxInterrupt.c
 *
 *  Created on: Nov 8, 2025
 *      Author: chikamso
 */

#include "STM32F44xx__driver.h"
#include "STM32F44xxgpio__driver.h"
#include "stm32f446xx_i2c_driver.h"
#include <string.h>

#define MYADDR	0x61
#define SLAVEADDR 0x68
#define delay for(uint32_t i = 0; i < 250000; i++)
#define MAX_LEN		127

//COMMANDS
#define GET_LEN 0x51
#define GET_MESSAGE 0x52
//Machine states
#define STATE_READY 		0
#define STATE_LENGTH_REQ	1
#define STATE_LENGTH_SENT	2
#define STATE_MESSAGE_REQ	3
#define STATE_MESSAGE_SENT	4

uint8_t i2c_state = STATE_READY;

I2C_handle_t I2CBus;

uint8_t command = 0;
uint8_t length = 0;
char message[MAX_LEN];
int main(void){
	memset(message, 0, sizeof(char));//Initialize to zero
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
	I2CBus.DevAddr = MYADDR;
	I2CBus.TxRxState = I2C_READY;
	I2C_Init(&I2CBus);
	//Enable interrupt on processor side
	I2C_IRQConfig(I2C1_EV_IRQ_NUM, ENABLE);
	I2C_IRQConfig(I2C1_ER_IRQ_NUM, ENABLE);

	while(1){
		while(GPIO_ReadFromInputPin(GPIOB, GPIO_PIN_NO_6));
		delay;
		while(i2c_state != STATE_READY);
		command = GET_LEN;
		I2C_MasterSendDataIT(&I2CBus, &command, 1, SLAVEADDR, I2C_SR_ENABLE);
		i2c_state = STATE_LENGTH_REQ;
		//The rest of the transaction is handled by callbacks
	}
}

void I2C1_EV_IRQHandler(void){
	I2C_EV_IRQHandling(&I2CBus);
}

void I2C1_ER_IRQHandler(void){
	I2C_ER_IRQHandling(&I2CBus);
}


void I2C_ApplicationEventCallback(I2C_handle_t* pI2CHandle, uint8_t AppEv){
	if(AppEv == I2C_EV_TX_CMPLT){
		if(i2c_state == STATE_LENGTH_REQ){
			//Now will receive the length
			I2C_MasterReceiveDataIT(&I2CBus, &length, 1, SLAVEADDR, I2C_SR_ENABLE);
			i2c_state = STATE_LENGTH_SENT;
		}else if(i2c_state == STATE_MESSAGE_REQ){
			//Now will receive the message
			I2C_MasterReceiveDataIT(&I2CBus, (uint8_t*)message, length, SLAVEADDR, I2C_SR_DISABLE);
			i2c_state = STATE_READY;
		}
	}else if(AppEv == I2C_EV_RX_CMPLT){
		if(i2c_state == STATE_LENGTH_SENT){
			command = GET_MESSAGE;
			I2C_MasterSendDataIT(&I2CBus, &command, 1, SLAVEADDR, I2C_SR_ENABLE);
			i2c_state = STATE_MESSAGE_REQ;
		}else if(i2c_state == STATE_READY){
			I2CEnable(I2CBus.pI2Cx, DISABLE);
		}
	}
}


