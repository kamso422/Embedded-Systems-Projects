/*
 * UART_TX.c
 *
 *  Created on: Nov 24, 2025
 *      Author: chikamso
 */
#include <string.h>
#include<stdio.h>
#include "STM32F44xx__driver.h"

#define STATE_READY			0
#define LENGTH_SENT			1
#define MESSAGE_SENT		2

uint8_t g_state = STATE_READY;

USART_Handle_t Usart;
uint32_t length = 0;
char message[1024] = "This just a test message setup";
uint8_t len_tx_buffer[4];

int main(void){
	length = strlen(message);
	//Assembling the 4-byte length into the global buffer
	for(uint8_t i = 0; i < 4; i++){
		len_tx_buffer[i] = (length >> (i * 8)) & 0xFF;
	}
	//Set up the button
	gpio_handle_t pin;
	memset(&pin, 0, sizeof(gpio_handle_t)); //Setting all internal structure values to zero
	pin.gpioReg = GPIOB;
	GPIO_PeriClockControl(pin.gpioReg, ENABLE);//Enable the peripheral clock for GPIOB

	//Initialize the button(PB6:D10)
	pin.gpioConfigure.MODE = GPIO_IT_MODE_FT;
	pin.gpioConfigure.OSPEED = GPIO_LOW_SPEED;
	pin.gpioConfigure.OTYPE = GPIO_PP;
	pin.gpioConfigure.PUPD = GPIO_PU;
	pin.gpioConfigure.pinNumber = GPIO_PIN_NO_5;
	gpioInit(&pin);
	GPIO_IRQConfig(EXTI9_5_IRQ_NUM, ENABLE);

	//Set up the pins for the UART peripheral
	gpio_handle_t UARTpin;
	memset(&UARTpin, 0, sizeof(gpio_handle_t));
	UARTpin.gpioReg = GPIOB;
	//GPIO_PeriClockControl(UARTpin.gpioReg, ENABLE);

	//Initialize the UART TX pin
	UARTpin.gpioConfigure.ALTF = AF7;
	UARTpin.gpioConfigure.MODE = GPIO_MODE_ALFN;
	UARTpin.gpioConfigure.OSPEED = GPIO_LOW_SPEED;
	UARTpin.gpioConfigure.OTYPE = GPIO_PP;
	UARTpin.gpioConfigure.PUPD = GPIO_NO_PUPD;
	UARTpin.gpioConfigure.pinNumber = GPIO_PIN_NO_10;
	gpioInit(&UARTpin);

	//Initialize the UART RX pin
	UARTpin.gpioConfigure.pinNumber = GPIO_PIN_NO_11;
	gpioInit(&UARTpin);

	//Set up the UART peripheral
	memset(&Usart, 0, sizeof(USART_Handle_t));
	Usart.pUSARTx = USART3;
	Usart.USART_Config.USART_Baud = USART_STD_BAUD_115200;
	Usart.USART_Config.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE;
	Usart.USART_Config.USART_Mode = USART_MODE_ONLY_TX;
	Usart.USART_Config.USART_NoOfStopBits = ONE_STOP_BIT;
	Usart.USART_Config.USART_ParityControl = PARITY_SELECTION_NONE;
	Usart.USART_Config.USART_WordLength = WORD_LENGTH_8BIT;
	USART_Init(&Usart);
	USART_IRQInterruptConfig(USART3_IRQ_NUM, ENABLE);

	while(1);
}

void USART3_IRQHandler(void){
	USART_IRQHandling(&Usart);
}
void EXTI9_5_IRQHandler(void){
	GPIO_IRQHandling(GPIO_PIN_NO_5);
	if(g_state == STATE_READY){
		USART_SendDataIT(&Usart, len_tx_buffer, 4);
		g_state = LENGTH_SENT;
	}
}
void USART_ApplicationEventCallback(USART_Handle_t *pUSARTHandle,uint8_t AppEv){
	if(AppEv == USART_EVENT_TX_CMPLT){
		if(g_state == LENGTH_SENT){
			g_state = MESSAGE_SENT;
			USART_SendDataIT(pUSARTHandle, (uint8_t*)message, length);
		}else if(g_state == MESSAGE_SENT){
			g_state = STATE_READY;
			USART_Enable(pUSARTHandle->pUSARTx, DISABLE);
		}
	}
}



