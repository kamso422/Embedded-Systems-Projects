/*
 * stm32f446xx_rcc_driver.c
 *
 *  Created on: Nov 24, 2025
 *      Author: chikamso
 */
#include "STM32F44xx__driver.h"

uint16_t AHB_PreScaler[8] = {2,4,8,16,64,128,256,512};
uint8_t APB1_PreScaler[4] = {2,4,8,16};
uint8_t APB2_PreScaler[4] = {2,4,8,16};

uint32_t RCC_GetPLLOutputClock(){
	return 0;
}

uint32_t RCC_GetPCLK1Value(void){
	uint32_t pclk1, SystemClk;
	uint8_t clksrc, ahbp, apb1;
	uint8_t temp = 0;

	clksrc = ((RCC->CFGR >> 2) & 0x3);
	if(clksrc == 0){
		SystemClk = 16000000;
	}else if(clksrc == 1){
		SystemClk = 8000000;
	}else if(clksrc == 2){
		SystemClk = RCC_GetPLLOutputClock();
	}

	//for AHB
	temp = ((RCC->CFGR >> 4) & 0xF);
	if(temp < 8){
		ahbp = 1;
	}else{
		ahbp = AHB_PreScaler[temp -8];
	}

	//APB1
	temp = 0;
	temp |= ((RCC->CFGR >> 10) & 0x7);
	if(temp < 4){
		apb1 = 1;
	}else{
		apb1 = APB1_PreScaler[temp -4];
	}
	pclk1 = (SystemClk / ahbp) /apb1;

	return pclk1;
}

uint32_t RCC_GetPCLK2Value(void){
	uint32_t pclk1, SystemClk;
	uint8_t clksrc, ahbp, apb2;
	uint8_t temp = 0;

	clksrc = ((RCC->CFGR >> 2) & 0x3);
	if(clksrc == 0){
		SystemClk = 16000000;
	}else if(clksrc == 1){
		SystemClk = 8000000;
	}else if(clksrc == 2){
		SystemClk = RCC_GetPLLOutputClock();
	}

	//for AHB
	temp = ((RCC->CFGR >> 4) & 0xF);
	if(temp < 8){
		ahbp = 1;//AHB clock not divided
	}else{
		ahbp = AHB_PreScaler[temp -8];
	}

	//APB2
	temp = 0;
	temp |= ((RCC->CFGR >> 13) & 0x7);
	if(temp < 4){
		apb2 = 1;
	}else{
		apb2 = APB2_PreScaler[temp -4];
	}
	pclk1 = (SystemClk / ahbp) /apb2;

	return pclk1;
}
