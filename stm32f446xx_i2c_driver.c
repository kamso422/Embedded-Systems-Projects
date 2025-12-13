/*
 * stm32f446xx_i2c_driver.c
 *
 *  Created on: Oct 14, 2025
 *      Author: chikamso
 */
#include "STM32F44xx__driver.h"

static void I2C_GenerateStartCondition(I2C_RegDef_t* pI2Cx);
static void I2C_GenerateStopCondition(I2C_RegDef_t* pI2Cx);
static uint8_t I2C_checkStatusFlag(I2C_RegDef_t* pI2Cx, uint32_t flagName);
static void I2C_ExecuteAddressPhase(I2C_RegDef_t* pI2Cx, uint32_t slaveAddr);
static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t* pI2Cx, uint32_t slaveAddr);
static void I2C_ClearADDRFlag(I2C_handle_t *pI2CHandle);
static void I2C_ManageAcking(I2C_RegDef_t* pI2Cx, uint8_t state);

void I2C_EnableInterrupts(I2C_handle_t *pI2CHandle, uint8_t EnDi){
	if(EnDi == ENABLE){
		//Implement the code to enable ITBUFEN Control Bit
		pI2CHandle->pI2Cx->CR2 |= (0x1 << I2C_CR2_ITBUFEN);
		//Implement the code to enable ITEVFEN Control Bit
		pI2CHandle->pI2Cx->CR2 |= (0x1 << I2C_CR2_ITEVTEN);
		//Implement the code to enable ITERREN Control Bit
		pI2CHandle->pI2Cx->CR2 |= (0x1 << I2C_CR2_ITERREN);
	}else{
		//implement the code to disable ITBUFEN Control Bit
		pI2CHandle->pI2Cx->CR2 &= ~(0x1 << I2C_CR2_ITBUFEN);
		//implement the code to disable ITEVFEN control bit
		pI2CHandle->pI2Cx->CR2 &= ~(0x1 << I2C_CR2_ITEVTEN);
	}
}
void I2C_CloseReceiveData(I2C_handle_t* pI2CHandle){
	I2C_EnableInterrupts(pI2CHandle, DISABLE);
	pI2CHandle->TxRxState = I2C_READY;
	pI2CHandle->pRxBuffer = NULL;
	pI2CHandle->RxLen = 0;
	pI2CHandle->RxSize = 0;
	if(pI2CHandle->i2cConfigure.ackControl == I2C_ACK_EN){
		I2C_ManageAcking(pI2CHandle->pI2Cx, ENABLE);
	}
}
void I2C_CloseSendData(I2C_handle_t* pI2CHandle){
	I2C_EnableInterrupts(pI2CHandle, DISABLE);
	pI2CHandle->pI2Cx->CR1 &= ~(0x1 << I2C_CR1_PE); // Disable peripheral
	pI2CHandle->TxRxState = I2C_READY;
	pI2CHandle->pTxBuffer = NULL;
	pI2CHandle->TxLen = 0;
	if(pI2CHandle->i2cConfigure.ackControl == I2C_ACK_EN){
		I2C_ManageAcking(pI2CHandle->pI2Cx, ENABLE);
	}
}

static uint8_t I2C_checkStatusFlag(I2C_RegDef_t* pI2Cx, uint32_t flagName){
	if(pI2Cx->SR1 & flagName){
		return FLAG_SET;
	}
	return FLAG_RESET;
}

static void I2C_GenerateStartCondition(I2C_RegDef_t* pI2Cx){
	pI2Cx->CR1 |= (0x1 << I2C_CR1_START);
}

static void I2C_ExecuteAddressPhase(I2C_RegDef_t* pI2Cx, uint32_t slaveAddr){
	slaveAddr = slaveAddr << 1;
	slaveAddr &= ~(1); //SlaveAddr is Slave Address + r/nw bit=0
	pI2Cx->DR = slaveAddr;
}

static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t* pI2Cx, uint32_t slaveAddr){
	slaveAddr = slaveAddr << 1;
	slaveAddr |= 1; //SlaveAddr is Slave Address + r/nw bit=1
	pI2Cx->DR = slaveAddr;
}

static void I2C_GenerateStopCondition(I2C_RegDef_t* pI2Cx){
	pI2Cx->CR1 |= (0x1 << I2C_CR1_STOP);
}

static void I2C_ClearADDRFlag(I2C_handle_t *pI2CHandle){
	uint8_t dummy_read;
	if(pI2CHandle->pI2Cx->SR2 & I2C_SR2_MSL){
		if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX){
			if(pI2CHandle->RxSize == 1){
				//first disable the ACK
				I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);
				//Clear the ADDR flag(read SR1, read SR2)
				dummy_read = pI2CHandle->pI2Cx->SR1;
				dummy_read = pI2CHandle->pI2Cx->SR2;
				(void)dummy_read;
			}
		}else{
			//Clear the ADDR flag(read SR1, read SR2)
			dummy_read = pI2CHandle->pI2Cx->SR1;
			dummy_read = pI2CHandle->pI2Cx->SR2;
			(void)dummy_read;
		}
	}else{
		//Clear the ADDR flag(read SR1, read SR2)
		dummy_read = pI2CHandle->pI2Cx->SR1;
		dummy_read = pI2CHandle->pI2Cx->SR2;
		(void)dummy_read;
	}
}

static void I2C_ManageAcking(I2C_RegDef_t* pI2Cx, uint8_t state){
	if(state == ENABLE){
		pI2Cx->CR1 |= (0x1 << I2C_CR1_ACK);
	}else{
		pI2Cx->CR1 &= ~(0x1 << I2C_CR1_ACK);
	}
}

void I2C_PeriClockControl(I2C_RegDef_t* pI2Cx,uint8_t EnorDi){
	if(EnorDi == ENABLE){
		if(pI2Cx == I2C1){
			I2C1_PCLK_EN();
		}else if(pI2Cx == I2C2){
			I2C2_PCLK_EN();
		}else if(pI2Cx == I2C3){
			I2C3_PCLK_EN();
		}
	}else{
		if(pI2Cx == I2C1){
			I2C1_PCLK_DIS();
		}else if(pI2Cx == I2C2){
			I2C2_PCLK_DIS();
		}else if(pI2Cx == I2C3){
			I2C3_PCLK_DIS();
		}
	}
}

void I2C_Init(I2C_handle_t* handle){
	//Configure the FREQ field of CR2
	uint32_t tempreg = 0;
	tempreg = RCC_GetPCLK1Value() / 1000000;
	handle->pI2Cx->CR2 |= (tempreg & 0x3F);

	//program the device own address
	tempreg = 0;
	tempreg = handle->i2cConfigure.deviceAddress << 1;
	tempreg |= (1 << 14);
	handle->pI2Cx->OAR1 |= tempreg;

	//CCR Calculation
	uint16_t ccr_value = 0;
	tempreg = 0;
	if(handle->i2cConfigure.sclSpeed <= I2C_SCL_SPEED_SM){
		//mode is in standard mode
		ccr_value = (RCC_GetPCLK1Value() / (2 * handle->i2cConfigure.sclSpeed));
		tempreg |= (ccr_value & 0xFFF);
	}else{
		//mode is in fast mode
		tempreg |= (1 << 15);
		tempreg |= (handle->i2cConfigure.fmDutyCycle << 14);
		if(handle->i2cConfigure.fmDutyCycle == Fm_DC1){
			ccr_value = (RCC_GetPCLK1Value() / (3 * handle->i2cConfigure.sclSpeed));
		}else{
			ccr_value = (RCC_GetPCLK1Value() / (25 * handle->i2cConfigure.sclSpeed));
		}
		tempreg |= (ccr_value & 0xFFF);
	}
	handle->pI2Cx->CCR |= tempreg;

	//TRISE configuration
	tempreg = 0;
	if(handle->i2cConfigure.sclSpeed <= I2C_SCL_SPEED_SM){
		//mode is standard
		tempreg |= (RCC_GetPCLK1Value() / 1000000U) + 1;
	}else{
		//mode is fast mode
		tempreg |= ((RCC_GetPCLK1Value() * 300) / 1000000000) +1;
	}
	handle->pI2Cx->TRISE = (tempreg & 0x3f);

	//Enable the peripheral
	handle->pI2Cx->CR1 |= (0x1 << I2C_CR1_PE);

	//ack control bit
	tempreg = 0;
	tempreg |= handle->i2cConfigure.ackControl << I2C_CR1_ACK;
	handle->pI2Cx->CR1 |= (0x1 << I2C_CR1_ACK);
}

void I2C_MasterSendData(I2C_handle_t* pI2CHandle, uint8_t *pTxbuffer, uint8_t Len, uint8_t SlaveAddr, uint8_t SR){
	//Generate a start condition
	I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

	//2. confirm that start generation is completed by checking the SB flag in the SR1
	// Note until the SB is cleared SCL will be stretched(pulled to LOW)
	while(!I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_SB));
	//3. Execute the address phase
	I2C_ExecuteAddressPhase(pI2CHandle->pI2Cx, SlaveAddr);
	//4. Confirm that address phase is completed by checking the ADDR flag in the SR1 register
	while(!I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_ADDR)){
		// Check if a NACK (AF flag) occurred
		    if(I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_AF))
		    {
		        // NACK occurred!
		        // 1. Clear the AF flag by writing 0 to it
		        pI2CHandle->pI2Cx->SR1 &= ~I2C_SR1_AF;
		        // 2. Generate a STOP condition
		        I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
		        // 3. Exit the function
		        return;
		    }
	}
	//5. Clear the ADDR flag according to its software sequence
	// Note: Until ADDR is cleared SCL will be stretched (pulled to low)
	I2C_ClearADDRFlag(pI2CHandle);
	//re-enable the ACK bit
	I2C_ManageAcking(pI2CHandle->pI2Cx, ENABLE);
	//6. send the data until len becomes zero
	while(Len > 0){
		while(!I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_TXE));//wait till TXE is set
		pI2CHandle->pI2Cx->DR = *pTxbuffer;
		pTxbuffer++;
		Len--;
	}
	//7. When Len becomes zero wait for TXE=1 and BTF=1 before generating the STOP condition
	//Note: TXE=1 and BTF=1, means that both SR and DR are empty and next transmission should begin
	//when BTF=1 SCL will be stretched (pulled to LOW)
	while(!I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_TXE));
	while(!I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_BTF));

	//8. Generate STOP condition and master need not to wait for the completion of stop condition.
	//Note: generate STOP, automatically clears the BTF
	if(SR == I2C_SR_DISABLE)
		I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
}

uint8_t I2C_MasterSendDataIT(I2C_handle_t* pI2CHandle, uint8_t *pTxbuffer, uint8_t Len, uint8_t SlaveAddr, uint8_t SR){
	uint8_t busystate = pI2CHandle->TxRxState;

		if( (busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
		{
			pI2CHandle->pTxBuffer = pTxbuffer;
			pI2CHandle->TxLen = Len;
			pI2CHandle->TxRxState = I2C_BUSY_IN_TX;
			pI2CHandle->DevAddr = SlaveAddr;
			pI2CHandle->Sr = SR;
			I2C_EnableInterrupts(pI2CHandle, ENABLE);
			if(pI2CHandle->pI2Cx->CR1 & (0x1 << I2C_CR1_PE)){
				// do nothing
			}else{
				pI2CHandle->pI2Cx->CR1 |= (0x1 << I2C_CR1_PE);
			}
			//Implement code to Generate START Condition
			I2C_GenerateStartCondition(pI2CHandle->pI2Cx);
		}

		return busystate;

}

uint8_t I2C_MasterReceiveDataIT(I2C_handle_t* pI2CHandle, uint8_t *pRxbuffer, uint8_t Len, uint8_t SlaveAddr, uint8_t SR){
	uint8_t busystate = pI2CHandle->TxRxState;

		if( (busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
		{
			pI2CHandle->pRxBuffer = pRxbuffer;
			pI2CHandle->RxLen = Len;
			pI2CHandle->TxRxState = I2C_BUSY_IN_RX;
			pI2CHandle->RxSize = Len; //Rxsize is used in the ISR code to manage the data reception
			pI2CHandle->DevAddr = SlaveAddr;
			pI2CHandle->Sr = SR;
			I2C_EnableInterrupts(pI2CHandle, ENABLE);
			if(pI2CHandle->pI2Cx->CR1 & (0x1 << I2C_CR1_PE)){
				// do nothing
			}else{
				pI2CHandle->pI2Cx->CR1 |= (0x1 << I2C_CR1_PE);
			}
			//Implement code to Generate START Condition
			I2C_GenerateStartCondition(pI2CHandle->pI2Cx);
		}

		return busystate;

}

void I2C_MasterReceiveData(I2C_handle_t* pI2CHandle, uint8_t *pRxbuffer, uint8_t Len, uint8_t SlaveAddr, uint8_t SR){
	//1. Generate the start condition
	I2C_GenerateStartCondition(pI2CHandle->pI2Cx);
	//2. Confirm that start generation is completed by checking the SB flag in the SR1
	//Note: Until SB is cleared SCL will be stretched (pulled to low)
	while(!I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_SB));
	//3. Send the address of the slave with r/nw bit set to R(1) (total 8 bit)
	I2C_ExecuteAddressPhaseRead(pI2CHandle->pI2Cx, SlaveAddr);
	//4. Wait until address phase is completed by checking the ADDR flag in the SR1
	while(!I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_ADDR)){
			// Check if a NACK (AF flag) occurred
			    if(I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_AF))
			    {
			        // NACK occurred!
			        // 1. Clear the AF flag by writing 0 to it
			        pI2CHandle->pI2Cx->SR1 &= ~I2C_SR1_AF;
			        // 2. Generate a STOP condition
			        I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
			        // 3. Exit the function
			        return;
			    }
		}
	//procedure to read only 1 byte from slave
	if(Len == 1){
		//Disable Acking
		I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);
		//Clear the ADDR flag
		I2C_ClearADDRFlag(pI2CHandle);
		//wait until RXNE becomes 1
		while(!I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_RXNE));
		//generate STOP condition
		if(SR == I2C_SR_DISABLE)
			I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
		//read data into buffer
		*pRxbuffer = pI2CHandle->pI2Cx->DR;
		return;
	}

	//procedure to read data from slave when Len > 1
	if(Len > 1){
		//Clear the ADDR flag
		I2C_ClearADDRFlag(pI2CHandle);
		//re-enabling ACKing
		I2C_ManageAcking(pI2CHandle->pI2Cx, ENABLE);
		//Read the data until Len becomes zero
		for(uint32_t i = Len; i > 0; i--){
			//wait until RXNE becomes zero
			while(!I2C_checkStatusFlag(pI2CHandle->pI2Cx, I2C_SR1_RXNE));
			// If this is the second-to-last byte...
			if(i == 2)
			{
				// Disable ACKing. This will send a NACK after this byte is read.
				I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);
			}
			// If this is the very last byte...
			if(i == 1)
			{
				// Generate the STOP condition.
				if(SR == I2C_SR_DISABLE)
					I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
			}
			//read the data from data register in to buffer
			*pRxbuffer = pI2CHandle->pI2Cx->DR;
			//Increment the buffer address
			++pRxbuffer;
		}
	}
	//re-enabling ACKing
	I2C_ManageAcking(pI2CHandle->pI2Cx, ENABLE);
}


void I2CEnable(I2C_RegDef_t* pI2Cx, uint8_t en_di){
	if(en_di == ENABLE){
		pI2Cx->CR1 |= (0x1 << I2C_CR1_PE);
	}else{
		pI2Cx->CR1 &= ~(0x1 << I2C_CR1_PE);
	}
}


void I2C_IRQConfig(uint8_t IRQNumber,uint8_t ENorDi){
	if(ENorDi == ENABLE){
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
void I2C_IRQPriorityConfig(uint8_t IRQNUmber, uint8_t IRQpriority){
	uint8_t iprx = IRQNUmber / 4;
	uint8_t iprSection = IRQNUmber % 4;
	uint8_t shiftNo = iprSection*8 + 4;
	*(NVIC_IPR0 + iprx) |= IRQpriority << shiftNo;
}

void I2C_SlaveSendData(I2C_RegDef_t* pI2C, uint8_t data){
	pI2C->DR = data;
}
uint8_t I2C_SlaveReceiveData(I2C_RegDef_t* pI2C){
	return pI2C->DR;
}

void I2C_EV_IRQHandling(I2C_handle_t* pI2CHandle){
	//Interrupt handling for both master and slave mode device
	uint32_t temp1,temp3, temp_bufen;
	temp1 = pI2CHandle->pI2Cx->CR2 & (0x1 << I2C_CR2_ITEVTEN);
	temp_bufen = pI2CHandle->pI2Cx->CR2 & (0x1 << I2C_CR2_ITBUFEN);

	temp3 = pI2CHandle->pI2Cx->SR1  & I2C_SR1_SB;
	//1. Handle for interrupt generated by SB event
	// Note: SB flag is only applicable in Master mode
	if(temp1 && temp3){
		//SB flag is set
		if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX){
			I2C_ExecuteAddressPhaseRead(pI2CHandle->pI2Cx, pI2CHandle->DevAddr);
		}else if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX){
			I2C_ExecuteAddressPhase(pI2CHandle->pI2Cx, pI2CHandle->DevAddr);
		}
	}

	//2. Handle for interrupt generated by ADDR event
	//Note: When master mode: Address is sent
	// When slave mode : Address matched with own address
	temp3 = pI2CHandle->pI2Cx->SR1  & I2C_SR1_ADDR;
	if(temp1 && temp3){
		//ADDR flag is set
		I2C_ClearADDRFlag(pI2CHandle);
	    // For a multi-byte read (RxSize > 1), we must
	    // re-enable ACKing after the ADDR flag is cleared.
	    //
	    if( (pI2CHandle->TxRxState == I2C_BUSY_IN_RX) && (pI2CHandle->RxSize > 1) )
	    {
	        I2C_ManageAcking(pI2CHandle->pI2Cx, ENABLE);
	    }
	}

	//3. Handle for interrupt generated by BTF event
	temp3 = pI2CHandle->pI2Cx->SR1  & I2C_SR1_BTF;
	if(temp1 && temp3){
		//BTF flag is set
		if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX){
			//make sure TXE is also set(Transfer buffer is also empty)
			if(pI2CHandle->pI2Cx->SR1 & I2C_SR1_TXE){
				//BTF, TXE = 1
				if(pI2CHandle->TxLen == 0){
					//1. generate the STOP condition
					if(pI2CHandle->Sr == I2C_SR_DISABLE){
						I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
					}
					//2. reset all the member element of the handle structure
					I2C_CloseSendData(pI2CHandle);
					//3. notify the application about transmission completion
					I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_TX_CMPLT);
				}
			}
		}else if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX){

		}
	}
	//4. Handle for interrupt generated by STOPF event
	// NOte: Stop detection flag is applicable only slave mode.
	temp3 = pI2CHandle->pI2Cx->SR1  & I2C_SR1_STOPF;
	if(temp1 && temp3){
		//STOPF flag is set
		//clear the STOPF 1.) read SR1 2) write to CR1
		pI2CHandle->pI2Cx->CR1 |= 0x0000;
		//Notify the application that STOP is detected
		I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_STOPF);
	}
	//5. Handle for interrupt generated by TXE event
	temp3 = pI2CHandle->pI2Cx->SR1  & I2C_SR1_TXE;
	if(temp1 && temp_bufen && temp3){
		//TXE flag is set
		//we have to do the data transmission
		if(pI2CHandle->pI2Cx->SR2 & I2C_SR2_MSL){
			if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX){
				if(pI2CHandle->TxLen > 0){
					//1. Load the data in to DR
					pI2CHandle->pI2Cx->DR = *(pI2CHandle->pTxBuffer);
					//2.Decrement the TxLen
					pI2CHandle->TxLen--;
					//3.Increment the buffer address
					pI2CHandle->pTxBuffer++;
				}
			}
		}else{
			//In slave mode
			//The TRA bit is set during the addressing phase
			if(pI2CHandle->pI2Cx->SR2 & I2C_SR2_TRA){
				I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_REQ);
			}
		}
	}
	//6. Handle for interrupt generated by RXNE event
	temp3 = pI2CHandle->pI2Cx->SR1  & I2C_SR1_RXNE;
	if(temp1 && temp_bufen && temp3){
		//RXNE flag is set
		//Check for device mode
		if(pI2CHandle->pI2Cx->SR2 & I2C_SR2_MSL){
			if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX){
				//we have to do the data reception
				if(pI2CHandle->RxSize == 1){
					*pI2CHandle->pRxBuffer = pI2CHandle->pI2Cx->DR;
					pI2CHandle->RxLen--;
				}
				if(pI2CHandle->RxSize > 1){
					if(pI2CHandle->RxLen == 2){
						//clear the ack bit
						I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);
					}
					//read DR
					*pI2CHandle->pRxBuffer = pI2CHandle->pI2Cx->DR;
					pI2CHandle->pRxBuffer++;
					pI2CHandle->RxLen--;
					pI2CHandle->RxSize--;
				}
				if(pI2CHandle->RxLen == 0){
					//Close the I2C data reception and notify the application
					//1. Generate the stop condition
					if(pI2CHandle->Sr == I2C_SR_DISABLE){
						I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
					}
					//2. Close the I2C rx
					I2C_CloseReceiveData(pI2CHandle);
					//3. Notify the application
					I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_RX_CMPLT);
				}
			}
		}else{
			//In slave mode
			if(!(pI2CHandle->pI2Cx->SR2 & I2C_SR2_TRA)){
				I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_RCVD);
			}
		}
	}
}
void I2C_ER_IRQHandling(I2C_handle_t* pI2CHandle){
	uint32_t temp1,temp2;

    //Know the status of  ITERREN control bit in the CR2
	temp2 = (pI2CHandle->pI2Cx->CR2) & I2C_CR2_ITERREN;


/***********************Check for Bus error************************************/
	temp1 = (pI2CHandle->pI2Cx->SR1) & I2C_SR1_BERR;
	if(temp1  && temp2 )
	{
		//This is Bus error
		//Implement the code to clear the buss error flag
		pI2CHandle->pI2Cx->SR1 &= ~I2C_SR1_BERR;
		//Implement the code to notify the application about the error
	   I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_BERR);
	}
/***********************Check for arbitration lost error************************************/
	temp1 = (pI2CHandle->pI2Cx->SR1) & I2C_SR1_ARLO;
	if(temp1  && temp2)
	{
		//This is arbitration lost error
		//Implement the code to clear the arbitration lost error flag
		pI2CHandle->pI2Cx->SR1 &= ~I2C_SR1_ARLO;
		//Implement the code to notify the application about the error
		I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_ARLO);
	}

/***********************Check for ACK failure  error************************************/

	temp1 = (pI2CHandle->pI2Cx->SR1) & I2C_SR1_AF;
	if(temp1  && temp2)
	{
		//This is ACK failure error
	    //Implement the code to clear the ACK failure error flag
		pI2CHandle->pI2Cx->SR1 &= ~I2C_SR1_AF;
		//Generate a stop condition
		I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
		//Reset the driver's state
		I2C_CloseSendData(pI2CHandle);
		//Implement the code to notify the application about the error
		I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_AF);
	}

/***********************Check for Overrun/underrun error************************************/
	temp1 = (pI2CHandle->pI2Cx->SR1) & I2C_SR1_OVR;
	if(temp1  && temp2)
	{
		//This is Overrun/underrun
	    //Implement the code to clear the Overrun/underrun error flag
		pI2CHandle->pI2Cx->SR1 &= ~I2C_SR1_OVR;
		//Implement the code to notify the application about the error
		I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_OVR);
	}

/***********************Check for Time out error************************************/
	temp1 = (pI2CHandle->pI2Cx->SR1) & I2C_SR1_TIMEOUT;
	if(temp1  && temp2)
	{
		//This is Time out error
	    //Implement the code to clear the Time out error flag
		pI2CHandle->pI2Cx->SR1 &= ~I2C_SR1_TIMEOUT;
		//Implement the code to notify the application about the error
		I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_TIMEOUT);
	}

}

__attribute__((weak)) void I2C_ApplicationEventCallback(I2C_handle_t*,uint8_t){

}
