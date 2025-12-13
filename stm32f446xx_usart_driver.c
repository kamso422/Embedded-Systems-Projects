/*
 * stm32f446xx_usart_driver.c
 *
 *  Created on: Nov 18, 2025
 *      Author: chikamso
 */
#include "STM32F44xx__driver.h"

void USART_PeriClockControl(USART_RegDef_t *pUSARTx, uint8_t EnorDi){
	if(EnorDi == ENABLE){
		if(pUSARTx == USART1){
			USART1_PCLK_EN();
		}else if(pUSARTx == USART2){
			USART2_PCLK_EN();
		}else if(pUSARTx == USART3){
			USART3_PCLK_EN();
		}else if(pUSARTx == USART6){
			USART6_PCLK_EN();
		}
	}else if(EnorDi == DISABLE){
		if(pUSARTx == USART1){
			USART1_PCLK_DIS();
		}else if(pUSARTx == USART2){
			USART2_PCLK_DIS();
		}else if(pUSARTx == USART3){
			USART3_PCLK_DIS();
		}else if(pUSARTx == USART6){
			USART6_PCLK_DIS();
		}
	}
}

void USART_Enable(USART_RegDef_t* pUSARTx, uint8_t EnDi){
	if(EnDi == ENABLE){
		pUSARTx->CR1 |= (0x1 << USART_CR1_UE);
	}else{
		pUSARTx->CR1 &= ~(0x1 << USART_CR1_UE);
	}
}

void USART_Init(USART_Handle_t *pUSARTHandle)
{
	//Temporary variable
	uint32_t tempreg = 0;
/******************************** Configuration of CR1******************************************/
	//Implement the code to enable the Clock for given USART peripheral
	USART_PeriClockControl(pUSARTHandle->pUSARTx, ENABLE);
	//Enable USART Tx and Rx engines according to the USART_Mode configuration item
	if ( pUSARTHandle->USART_Config.USART_Mode == USART_MODE_ONLY_RX)
	{
		//Implement the code to enable the Receiver bit field
		tempreg|= (1 << USART_CR1_RE);
	}else if (pUSARTHandle->USART_Config.USART_Mode == USART_MODE_ONLY_TX)
	{
		//Implement the code to enable the Transmitter bit field
		tempreg |= ( 1 << USART_CR1_TE);

	}else if (pUSARTHandle->USART_Config.USART_Mode == USART_MODE_TXRX)
	{
		//Implement the code to enable the both Transmitter and Receiver bit fields
		tempreg |= ((1 << USART_CR1_RE) | (1 << USART_CR1_TE));
	}

    //Implement the code to configure the Word length configuration item
	tempreg |= pUSARTHandle->USART_Config.USART_WordLength << USART_CR1_M;

    //Configuration of parity control bit fields
	if ( pUSARTHandle->USART_Config.USART_ParityControl == PARITY_SELECTION_EVEN)
	{
		//Implement the code to enable the parity control
		tempreg |= ( 1 << USART_CR1_PCE);
		//Implement the code to enable EVEN parity
		//Not required because by default EVEN parity will be selected once you enable the parity control

	}else if (pUSARTHandle->USART_Config.USART_ParityControl == PARITY_SELECTION_ODD)
	{
		//Implement the code to enable the parity control
	    tempreg |= ( 1 << USART_CR1_PCE);
	    //Implement the code to enable ODD parity
	    tempreg |= ( 1 << USART_CR1_PS);
	}
   //Program the CR1 register
	pUSARTHandle->pUSARTx->CR1 |= tempreg;
/******************************** Configuration of CR2******************************************/
	tempreg = 0;
	//Implement the code to configure the number of stop bits inserted during USART frame transmission
	tempreg |= pUSARTHandle->USART_Config.USART_NoOfStopBits << USART_CR2_STOP;
	//Program the CR2 register
	pUSARTHandle->pUSARTx->CR2 |= tempreg;

/******************************** Configuration of CR3******************************************/
	tempreg = 0;
	//Configuration of USART hardware flow control
	if ( pUSARTHandle->USART_Config.USART_HWFlowControl == USART_HW_FLOW_CTRL_CTS)
	{
		//Implement the code to enable CTS flow control
		tempreg |= ( 1 << USART_CR3_CTSE);
	}else if (pUSARTHandle->USART_Config.USART_HWFlowControl == USART_HW_FLOW_CTRL_RTS)
	{
		//Implement the code to enable RTS flow control
		tempreg |= (1 << USART_CR3_RTSE);
	}else if (pUSARTHandle->USART_Config.USART_HWFlowControl == USART_HW_FLOW_CTRL_CTS_RTS)
	{
		//Implement the code to enable both CTS and RTS Flow control
		tempreg |= ((1 << USART_CR3_CTSE) | (1 << USART_CR3_RTSE));
	}
	pUSARTHandle->pUSARTx->CR3 |= tempreg;

/******************************** Configuration of BRR(Baudrate register)******************************************/

	//Implement the code to configure the baud rate
	USART_SetBaudRate(pUSARTHandle->pUSARTx, pUSARTHandle->USART_Config.USART_Baud);

}

void USART_SendData(USART_Handle_t *pUSARTHandle,uint8_t *pTxBuffer, uint32_t Len){
	uint16_t *pdata;
	//Loop over until "Len" number of bytes are transferred
	for(uint32_t i = 0; i < Len; i++){
		//Implement the code to wait until TXE flag is set in the SR
		while(!USART_GetFlagStatus(pUSARTHandle->pUSARTx, USART_SR_TXE));
		//Check the USART_WordLength item for 9BIT or 8BIT in a frame
		if(pUSARTHandle->USART_Config.USART_WordLength == WORD_LENGTH_9BIT){
			//if 9bit, load the DR with 2 bytes masking the bits other than the first 9 bit
			pdata = (uint16_t*)pTxBuffer;
			pUSARTHandle->pUSARTx->DR = (*pdata & (uint16_t)0x01FF);
			//check for USART_ParityControl
			if(pUSARTHandle->USART_Config.USART_ParityControl == PARITY_SELECTION_NONE){
				//No parity is used in this transfer. So 9 bits of user data will be sent
				//Implement the code to increment pTxBuffer twice
				pTxBuffer++;
				pTxBuffer++;
			}else{
				//Parity bit is used in this transfer. So 8 bits of user data will be sent
				//the 9th bit will be replaced by parity bit by the hardware
				pTxBuffer++;
			}
		}else{
			//This is 8 bit data transfer
			pUSARTHandle->pUSARTx->DR = (*pTxBuffer & 0xFF);
			pTxBuffer++;
		}
	}
	//Implement the code to wait till TC flag is set in the SR
	while(!USART_GetFlagStatus(pUSARTHandle->pUSARTx, USART_SR_TC));
}

uint8_t USART_SendDataIT(USART_Handle_t *pUSARTHandle,uint8_t *pTxBuffer, uint32_t Len){
	uint8_t txstate = pUSARTHandle->TxBusyState;
	if(txstate != USART_BUSY_IN_TX){
		pUSARTHandle->TxLen = Len;
		pUSARTHandle->pTxBuffer = pTxBuffer;
		pUSARTHandle->TxBusyState = USART_BUSY_IN_TX;

		//Implement the code to enable the interrupt for TXE
		pUSARTHandle->pUSARTx->CR1 |= (0x1 << USART_CR1_TXEIE);
		//Implement the code to enable the interrupt for TC
		pUSARTHandle->pUSARTx->CR1 |= (0x1 << USART_CR1_TCIE);
		//Enable the peripheral
		USART_Enable(pUSARTHandle->pUSARTx, ENABLE);
	}
	return txstate;
}
uint8_t USART_ReceiveDataIT(USART_Handle_t *pUSARTHandle, uint8_t *pRxBuffer, uint32_t Len){
	uint8_t rxstate = pUSARTHandle->RxBusyState;
	if(rxstate != USART_BUSY_IN_RX){
		pUSARTHandle->RxLen = Len;
		pUSARTHandle->pRxBuffer = pRxBuffer;
		pUSARTHandle->RxBusyState = USART_BUSY_IN_RX;

		//Implement the code to enable interrupt for RXNE
		pUSARTHandle->pUSARTx->CR1 |= (0x1 << USART_CR1_RXNEIE);
	}
	return rxstate;
}

void USART_ReceiveData(USART_Handle_t *pUSARTHandle, uint8_t *pRxBuffer, uint32_t Len){
	//Loop over until "Len" number of bytes have been received
	for(uint32_t i = 0; i < Len; i++){
		//Implement the code to wait until RXNE flag is set in the SR
		while(!USART_GetFlagStatus(pUSARTHandle->pUSARTx, USART_SR_RXNE));
		//Check the USART_WordLength to decide whether we are going to receive 9bit of data in a frame or 8 bit
		if(pUSARTHandle->USART_Config.USART_WordLength == WORD_LENGTH_9BIT){
			//We are going to receive 9bit in a frame
			//Check are we using USART_ParityControl control or not
			if(pUSARTHandle->USART_Config.USART_ParityControl == PARITY_SELECTION_NONE){
				//No parity is used. so, all 9bit will be of user data
				//read only 9 bits. So mask the DR with 0x01ff
				*((uint16_t*)pRxBuffer) = (pUSARTHandle->pUSARTx->DR & (uint8_t)0xFF);
				//Increment the pRxBuffer
				pRxBuffer++;
				pRxBuffer++;
			}else{
				*pRxBuffer = (pUSARTHandle->pUSARTx->DR & (uint16_t)0xFF);
				//increment the buffer
				pRxBuffer++;
			}
		}else{
			//We are going to receive 8 bit in a frame
			//check are we using USART_ParityControl or not
			if(pUSARTHandle->USART_Config.USART_ParityControl == PARITY_SELECTION_NONE){
				*pRxBuffer = pUSARTHandle->pUSARTx->DR;
			}else{
				*pRxBuffer = (uint8_t)(pUSARTHandle->pUSARTx->DR & (uint16_t)0x7F);
			}
			pRxBuffer++;
		}
	}
}

void USART_DeInit(USART_RegDef_t *pUSARTx){
	if(pUSARTx == USART1){
		RESET_USART1_REG();
	}else if(pUSARTx == USART2){
		RESET_USART2_REG();
	}else if(pUSARTx == USART3){
		RESET_USART3_REG();
	}else if(pUSARTx == USART6){
		RESET_USART6_REG();
	}
}

uint8_t USART_GetFlagStatus(USART_RegDef_t *pUSARTx , uint32_t FlagName){
	if(pUSARTx->SR & FlagName){
		return TRUE;
	}
	return FALSE;
}

void USART_ClearFlag(USART_RegDef_t *pUSARTx, uint16_t StatusFlagName){
	pUSARTx->SR &= ~StatusFlagName;
}

void USART_SetBaudRate(USART_RegDef_t *pUSARTx, uint32_t BaudRate){

	//Variable to hold the APB clock
	uint32_t PCLKx;
	uint32_t usartdiv;
	//variables to hold Mantissa and Fraction values
	uint32_t M_part,F_part;

	uint32_t tempreg=0;
	//Get the value of APB bus clock in to the variable PCLKx
	if(pUSARTx == USART1 || pUSARTx == USART6)
	{
	   //USART1 and USART6 are hanging on APB2 bus
	   PCLKx = RCC_GetPCLK2Value();
	}else
	{
	   PCLKx = RCC_GetPCLK1Value();
	}
	//Check for OVER8 configuration bit
	if(pUSARTx->CR1 & (1 << USART_CR1_OVER8))
	{
	   //OVER8 = 1 , over sampling by 8
	   usartdiv = ((25 * PCLKx) / (2 *BaudRate));
	}else
	{
	   //over sampling by 16
		usartdiv = ((25 * PCLKx) / (4 *BaudRate));
	}
	//Calculate the Mantissa part
	M_part = usartdiv/100;
	//Place the Mantissa part in appropriate bit position . refer USART_BRR
	tempreg |= M_part << 4;
	//Extract the fraction part
	F_part = (usartdiv - (M_part * 100));
	//Calculate the final fractional
	if(pUSARTx->CR1 & ( 1 << USART_CR1_OVER8))
	{
	  //OVER8 = 1 , over sampling by 8
	  F_part = ((( F_part * 8)+ 50) / 100)& ((uint8_t)0x07);
	}else
	{
	   //over sampling by 16
	   F_part = ((( F_part * 16)+ 50) / 100) & ((uint8_t)0x0F);
	}
	//Place the fractional part in appropriate bit position . refer USART_BRR
	tempreg |= F_part;

	//copy the value of tempreg in to BRR register
	pUSARTx->BRR = tempreg;
}

void USART_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi){
	if(EnorDi == ENABLE){
		if(IRQNumber < 32){
			*NVIC_ISER0 |= (0x1 << IRQNumber);
		}else if(IRQNumber > 31 && IRQNumber < 64){
			*NVIC_ISER1 |= (0x1 << (IRQNumber % 32));
		}else if(IRQNumber > 63 && IRQNumber < 96){
			*NVIC_ISER2 |= (0x1 << (IRQNumber % 64));
		}else if(IRQNumber > 95 && IRQNumber < 128){
			*NVIC_ISER3 |= (0x1 << (IRQNumber % 96));
		}
	}else{
		if(IRQNumber < 31){
			*NVIC_ICER0 |= (0x1 << IRQNumber);
		}else if(IRQNumber > 31 && IRQNumber < 64){
			*NVIC_ICER1 |= (0x1 << (IRQNumber % 32));
		}else if(IRQNumber > 63 && IRQNumber < 96){
			*NVIC_ICER2 |= (0x1 << (IRQNumber % 64));
		}else if(IRQNumber > 95 && IRQNumber < 128){
			*NVIC_ICER3 |= (0x1 << (IRQNumber % 96));
		}
	}
}

void USART_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority){
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprSection = IRQNumber % 4;
	uint8_t shiftNo = iprSection*8 + 4;
	*(NVIC_IPR0 + iprx) |= IRQPriority << shiftNo;
}


void USART_IRQHandling(USART_Handle_t *pUSARTHandle)
{

	uint32_t temp1 , temp2, temp3;

/*************************Check for TC flag ********************************************/

    //Implement the code to check the state of TC bit in the SR
	temp1 = pUSARTHandle->pUSARTx->SR & USART_SR_TC;
	 //Implement the code to check the state of TCEIE bit
	temp2 = pUSARTHandle->pUSARTx->CR1 & ( 1 << USART_CR1_TCIE);
	if(temp1 && temp2 )
	{
		//this interrupt is because of TC
		//close transmission and call application callback if TxLen is zero
		if (pUSARTHandle->TxBusyState == USART_BUSY_IN_TX)
		{
			//Check the TxLen . If it is zero then close the data transmission
			if(pUSARTHandle->TxLen == 1)
			{
				//Clear the TCIE bit
				//Implement the code to clear the TCIE control bit
				pUSARTHandle->pUSARTx->CR1 &= ~(0x1 << USART_CR1_TCIE);
				//Implement the code to clear the TXEIE control bit
				pUSARTHandle->pUSARTx->CR1 &= ~(0x1 << USART_CR1_TXEIE);
				//Implement the code to clear the TC flag
				uint32_t dummyRead = pUSARTHandle->pUSARTx->SR;
				(void)dummyRead;
				pUSARTHandle->pUSARTx->DR = (*pUSARTHandle->pTxBuffer  & (uint8_t)0xFF);
				//wait for the last bit to be transferred
				while(!USART_GetFlagStatus(pUSARTHandle->pUSARTx, USART_SR_TC));

				//Reset the application state
				pUSARTHandle->TxBusyState = USART_READY;
				//Reset Buffer address to NULL
				pUSARTHandle->pTxBuffer = NULL;
				//Reset the length to zero
				pUSARTHandle->TxLen = 0;
				//Call the applicaton call back with event USART_EVENT_TX_CMPLT
				USART_ApplicationEventCallback(pUSARTHandle,USART_EVENT_TX_CMPLT);
			}
		}
	}

/*************************Check for TXE flag ********************************************/

	//Implement the code to check the state of TXE bit in the SR
	temp1 = pUSARTHandle->pUSARTx->SR & USART_SR_TXE;
	//Implement the code to check the state of TXEIE bit in CR1
	temp2 = pUSARTHandle->pUSARTx->CR1 & (0x1 << USART_CR1_TXEIE);

	if(temp1 && temp2 )
	{
		uint16_t pdata = 0;
		//this interrupt is because of TXE
		if(pUSARTHandle->TxBusyState == USART_BUSY_IN_TX)
		{
			//Keep sending data until Txlen reaches to zero
			if(pUSARTHandle->TxLen > 1)
			{
				//Check the USART_WordLength item for 9BIT or 8BIT in a frame
				if(pUSARTHandle->USART_Config.USART_WordLength == WORD_LENGTH_9BIT)
				{
					//if 9BIT , load the DR with 2bytes masking the bits other than first 9 bits
					pdata = *((uint16_t*)pUSARTHandle->pTxBuffer);
					//loading only first 9 bits , so we have to mask with the value 0x01FF
					pUSARTHandle->pUSARTx->DR = (pdata & (uint16_t)0x01FF);

					//check for USART_ParityControl
					if(pUSARTHandle->USART_Config.USART_ParityControl == PARITY_SELECTION_NONE)
					{
						//No parity is used in this transfer , so, 9bits of user data will be sent
						//Implement the code to increment pTxBuffer twice
						pUSARTHandle->pTxBuffer++;
						pUSARTHandle->pTxBuffer++;
						//Implement the code to decrement the length
						pUSARTHandle->TxLen--;
						pUSARTHandle->TxLen--;
					}
					else
					{
						//Parity bit is used in this transfer . so , 8bits of user data will be sent
						//The 9th bit will be replaced by parity bit by the hardware
						pUSARTHandle->pTxBuffer++;
						//Implement the code to decrement the length
						pUSARTHandle->TxLen--;
					}
				}
				else
				{
					//This is 8bit data transfer
					pUSARTHandle->pUSARTx->DR = (*pUSARTHandle->pTxBuffer  & (uint8_t)0xFF);
					//Implement the code to increment the buffer address
					pUSARTHandle->pTxBuffer++;
					//Implement the code to decrement the length
					pUSARTHandle->TxLen--;
				}
			}
		}
	}

/*************************Check for RXNE flag ********************************************/

	temp1 = pUSARTHandle->pUSARTx->SR & USART_SR_RXNE;
	temp2 = pUSARTHandle->pUSARTx->CR1 & ( 1 << USART_CR1_RXNEIE);

	if(temp1 && temp2 )
	{
		//this interrupt is because of rxne
		if(pUSARTHandle->RxBusyState == USART_BUSY_IN_RX)
		{
			//RXNE is set so receive data
			if(pUSARTHandle->RxLen > 0)
			{
				//Check the USART_WordLength to decide whether we are going to receive 9bit of data in a frame or 8 bit
				if(pUSARTHandle->USART_Config.USART_WordLength == WORD_LENGTH_9BIT)
				{
					//We are going to receive 9bit data in a frame
					//Now, check are we using USART_ParityControl control or not
					if(pUSARTHandle->USART_Config.USART_ParityControl == PARITY_SELECTION_NONE)
					{
						//No parity is used. so, all 9bits will be of user data

						//read only first 9 bits so mask the DR with 0x01FF
						*((uint16_t*)pUSARTHandle->pRxBuffer) = (pUSARTHandle->pUSARTx->DR  & (uint16_t)0x01FF);
						//Now increment the pRxBuffer two times
						pUSARTHandle->pRxBuffer++;
						pUSARTHandle->pRxBuffer++;
						//Implement the code to decrement the length
						pUSARTHandle->RxLen--;
						pUSARTHandle->RxLen--;
					}
					else
					{
						//Parity is used. so, 8bits will be of user data and 1 bit is parity
						 *(pUSARTHandle->pRxBuffer) = (pUSARTHandle->pUSARTx->DR  & (uint8_t)0xFF);
						 //Now increment the pRxBuffer
						 pUSARTHandle->pRxBuffer++;
						 //Implement the code to decrement the length
						 pUSARTHandle->RxLen--;
					}
				}
				else
				{
					//We are going to receive 8bit data in a frame
					//Now, check are we using USART_ParityControl control or not
					if(pUSARTHandle->USART_Config.USART_ParityControl == PARITY_SELECTION_NONE)
					{
						//No parity is used , so all 8bits will be of user data
						//read 8 bits from DR
						*(pUSARTHandle->pRxBuffer) = (uint8_t) (pUSARTHandle->pUSARTx->DR  & (uint8_t)0xFF);
					}
					else
					{
						//Parity is used, so , 7 bits will be of user data and 1 bit is parity
						//read only 7 bits , hence mask the DR with 0X7F
						*(pUSARTHandle->pRxBuffer) = (uint8_t) (pUSARTHandle->pUSARTx->DR  & (uint8_t)0x7F);
					}
					//Now , increment the pRxBuffer
					pUSARTHandle->pRxBuffer++;
					//Implement the code to decrement the length
					pUSARTHandle->RxLen--;
				}
			}//if RxLen = 0
			if(!pUSARTHandle->RxLen)
			{
				//disable the rxne
				pUSARTHandle->pUSARTx->CR1 &= ~( 1 << USART_CR1_RXNEIE );
				pUSARTHandle->RxBusyState = USART_READY;
				USART_ApplicationEventCallback(pUSARTHandle,USART_EVENT_RX_CMPLT);
			}
		}
	}


/*************************Check for CTS flag ********************************************/
//Note : CTS feature is not applicable for UART4 and UART5

	//Implement the code to check the status of CTS bit in the SR
	temp1 = pUSARTHandle->pUSARTx->SR & USART_SR_CTS;
	//Implement the code to check the state of CTSE bit in CR1
	temp2 = pUSARTHandle->pUSARTx->CR3 & ( 1 << USART_CR3_CTSE);
	//Implement the code to check the state of CTSIE bit in CR3 (This bit is not available for UART4 & UART5.)
	temp3 = pUSARTHandle->pUSARTx->CR3 & ( 1 << USART_CR3_CTSIE);

	if(temp1  && temp2 && temp3)
	{
		//Implement the code to clear the CTS flag in SR
		pUSARTHandle->pUSARTx->SR &= ~USART_SR_CTS;
		//this interrupt is because of CTS
		USART_ApplicationEventCallback(pUSARTHandle,USART_EVENT_CTS);
	}

/*************************Check for IDLE detection flag ********************************************/

	//Implement the code to check the status of IDLE flag bit in the SR
	temp1 = pUSARTHandle->pUSARTx->SR & USART_SR_IDLE;
	//Implement the code to check the state of IDLEIE bit in CR1
	temp2 = pUSARTHandle->pUSARTx->CR3 & ( 1 << USART_CR1_IDLEIE);

	if(temp1 && temp2)
	{
		//Implement the code to clear the IDLE flag. Refer to the RM to understand the clear sequence
		uint32_t dummyRead = pUSARTHandle->pUSARTx->SR;
		dummyRead = pUSARTHandle->pUSARTx->DR;
		(void)dummyRead;
		//this interrupt is because of idle
		USART_ApplicationEventCallback(pUSARTHandle,USART_EVENT_IDLE);
	}

/*************************Check for Overrun detection flag ********************************************/

	//Implement the code to check the status of ORE flag  in the SR
	temp1 = pUSARTHandle->pUSARTx->SR & USART_SR_ORE;
	//Implement the code to check the status of RXNEIE  bit in the CR1
	temp2 = pUSARTHandle->pUSARTx->CR1 & (0x1 << USART_CR1_RXNEIE);

	if(temp1  && temp2 )
	{
		//Need not to clear the ORE flag here, instead give an api for the application to clear the ORE flag .
		//this interrupt is because of Overrun error
		USART_ApplicationEventCallback(pUSARTHandle,USART_ERR_ORE);
	}



/*************************Check for Error Flag ********************************************/

//Noise Flag, Overrun error and Framing Error in multibuffer communication
//We dont discuss multibuffer communication in this course. please refer to the RM
//The blow code will get executed in only if multibuffer mode is used.

	temp2 =  pUSARTHandle->pUSARTx->CR3 & ( 1 << USART_CR3_EIE) ;
	if(temp2)
	{
		temp1 = pUSARTHandle->pUSARTx->SR;
		if(temp1 & USART_SR_FE)
		{
			/*
				This bit is set by hardware when a de-synchronization, excessive noise or a break character
				is detected. It is cleared by a software sequence (an read to the USART_SR register
				followed by a read to the USART_DR register).
			*/
			USART_ApplicationEventCallback(pUSARTHandle,USART_ERR_FE);
		}

		if(temp1 & USART_SR_NF)
		{
			/*
				This bit is set by hardware when noise is detected on a received frame. It is cleared by a
				software sequence (an read to the USART_SR register followed by a read to the
				USART_DR register).
			*/
			USART_ApplicationEventCallback(pUSARTHandle,USART_ERR_NE);
		}
		if(temp1 & USART_SR_ORE )
		{
			USART_ApplicationEventCallback(pUSARTHandle,USART_ERR_ORE);
		}
	}
}


__attribute__((weak)) void USART_ApplicationEventCallback(USART_Handle_t *pUSARTHandle,uint8_t AppEv){

}

