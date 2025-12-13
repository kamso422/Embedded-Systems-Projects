/*
 * USART_RX.c
 *
 *  Created on: Dec 2, 2025
 *      Author: chikamso
 */


#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include "STM32F44xx__driver.h"

#define STATE_READY			0
#define TRANSMISSION_CMPLT	1

uint8_t g_state = STATE_READY;

USART_Handle_t Usart;
uint32_t length = 0;
char* message[3] = {"this is just to see if it works\n", "another message to be sent\n","the end\n"};
char charc;
int main(void){


	//Set up the UART peripheral
	memset(&Usart, 0, sizeof(USART_Handle_t));
	Usart.pUSARTx = USART1;
	Usart.USART_Config.USART_Baud = USART_STD_BAUD_115200;
	Usart.USART_Config.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE;
	Usart.USART_Config.USART_Mode = USART_MODE_TXRX;
	Usart.USART_Config.USART_NoOfStopBits = ONE_STOP_BIT;
	Usart.USART_Config.USART_ParityControl = PARITY_SELECTION_NONE;
	Usart.USART_Config.USART_WordLength = WORD_LENGTH_8BIT;
	USART_Init(&Usart);
	USART_IRQInterruptConfig(USART1_IRQ_NUM, ENABLE);

	//Set up the pins for the UART peripheral
	gpio_handle_t UARTpin;
	memset(&UARTpin, 0, sizeof(gpio_handle_t));
	UARTpin.gpioReg = GPIOA;
	GPIO_PeriClockControl(UARTpin.gpioReg, ENABLE);

	//Initialize the UART TX pin
	UARTpin.gpioConfigure.ALTF = AF7;
	UARTpin.gpioConfigure.MODE = GPIO_MODE_ALFN;
	UARTpin.gpioConfigure.OSPEED = GPIO_LOW_SPEED;
	UARTpin.gpioConfigure.OTYPE = GPIO_PP;
	UARTpin.gpioConfigure.PUPD = GPIO_NO_PUPD;
	UARTpin.gpioConfigure.pinNumber = GPIO_PIN_NO_9;
	gpioInit(&UARTpin);

	//Initialize the UART RX pin
	UARTpin.gpioConfigure.pinNumber = GPIO_PIN_NO_10;
	gpioInit(&UARTpin);


	//Set up the button(PC13,BLUE BUTTON(USER))
	gpio_handle_t pin;
	memset(&pin, 0, sizeof(gpio_handle_t)); //Setting all internal structure values to zero
	pin.gpioReg = GPIOB;
	GPIO_PeriClockControl(pin.gpioReg, ENABLE);//Enable the peripheral clock for GPIOB

	//Initialize the button(PB5:D10)
	pin.gpioConfigure.MODE = GPIO_IT_MODE_FT;
	pin.gpioConfigure.OSPEED = GPIO_LOW_SPEED;
	pin.gpioConfigure.OTYPE = GPIO_PP;
	pin.gpioConfigure.PUPD = GPIO_PU;
	pin.gpioConfigure.pinNumber = GPIO_PIN_NO_5;
	gpioInit(&pin);
	GPIO_IRQConfig(EXTI9_5_IRQ_NUM, ENABLE);

	USART_Enable(Usart.pUSARTx, ENABLE);

	while(1){
		while(USART_ReceiveDataIT(&Usart, (uint8_t*)&charc, 1) != USART_READY);
	}
}

void USART1_IRQHandler(void){
	USART_IRQHandling(&Usart);
	//USART_SendData(&Usart, (uint8_t*)&charc, 1);
}

void EXTI9_5_IRQHandler(void){
	GPIO_IRQHandling(GPIO_PIN_NO_5);
	for(uint8_t i = 0; i < 3; i++){
		USART_SendData(&Usart, (uint8_t*)message[i],strlen(message[i]));
	}
}

void USART_ApplicationEventCallback(USART_Handle_t *pUSARTHandle,uint8_t AppEv){
	if(AppEv == USART_EVENT_RX_CMPLT){
		if(g_state == TRANSMISSION_CMPLT){
			g_state = STATE_READY;
			USART_Enable(pUSARTHandle->pUSARTx, DISABLE);
		}
	}
}



