/*
 * Created by Chikamso Okeru on Dec 10, 2025
 * Objective: Use the SPI communication protocol to
 * interface with an arduino and display message on an LCD.
 */

#include <stdio.h>
#include <string.h>
#include "STM32F44xx__driver.h"

//Initialize pins for SPI pins
gpio_handle_t pin;
SPI_Handle_t spi;
uint32_t length = 0;

char* message = "chikamso okeru\0";

int main(void){
	memset(&pin, 0, sizeof(pin));

	//setup the MOSI pin
	pin.gpioReg = GPIOA;
	pin.gpioConfigure.ALTF = AF5;
	pin.gpioConfigure.MODE = GPIO_MODE_ALFN;
	pin.gpioConfigure.OSPEED = GPIO_LOW_SPEED;
	pin.gpioConfigure.OTYPE = GPIO_PP;
	pin.gpioConfigure.PUPD = GPIO_NO_PUPD;
	pin.gpioConfigure.pinNumber = GPIO_PIN_NO_7;
	gpioInit(&pin);
	//setup the MISO pin
	pin.gpioConfigure.pinNumber = GPIO_PIN_NO_6;
	gpioInit(&pin);
	//setup the SCK pin
	pin.gpioConfigure.pinNumber = GPIO_PIN_NO_5;
	gpioInit(&pin);
	//Setup the NSS pin
	pin.gpioConfigure.pinNumber = GPIO_PIN_NO_4;
	gpioInit(&pin);

	//Setup the button
	pin.gpioConfigure.MODE = GPIO_IT_MODE_FT;
	pin.gpioConfigure.PUPD = GPIO_PU;
	pin.gpioConfigure.pinNumber = GPIO_PIN_NO_1;
	gpioInit(&pin);
	GPIO_IRQConfig(EXTI1_IRQ_NUM, ENABLE);

	//Setup the spi bus
	memset(&spi, 0, sizeof(spi));
	spi.pSPIx = SPI1;
	spi.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
	spi.SPIConfig.SPI_CPHA  = SPI_CPHA1;
	spi.SPIConfig.SPI_CPOL = SPI_CPOL0;
	spi.SPIConfig.SPI_DFF = DFF8;
	spi.SPIConfig.SPI_DeviceMode = MASTER;
	spi.SPIConfig.SPI_SSM = SSM_HW;
	spi.SPIConfig.SPI_SclkSpeed = SPI_CLK_SPEED_DIV256;
	spiInit(&spi);

	length = strlen(message);

	//Set SSOE bit to use hardware flow control
	setSSOE(spi.pSPIx, ENABLE);

}

void EXTI1_IRQHandler(void){
	GPIO_IRQHandling(GPIO_PIN_NO_1);
	spiEnable(spi.pSPIx, ENABLE);
	SPI_SendData(spi.pSPIx, (uint8_t*)&length, 1);
	SPI_SendData(spi.pSPIx,(uint8_t*)message, length);
	spiEnable(spi.pSPIx, DISABLE);
}


