/* Includes ------------------------------------------------------------------*/
#include "bspManager.h"

static DMA_HandleTypeDef hdma_tx;
static DMA_HandleTypeDef hdma_rx;

TS_StateTypeDef  TS_State = {0};

//HANDLERS
CAN_HandleTypeDef hcan1;
SPI_HandleTypeDef SpiHandle;

/* DMA transfer state */
uint32_t wTransferState = TRANSFER_WAIT;

static void CAN_Init();
static void SPI_Init();

void bspManagerInit(void){
	/* Initialize the NOR */
	BSP_QSPI_Init();
	BSP_QSPI_EnableMemoryMappedMode();

	/* Debug Led */
	BSP_LED_Init(LED1);
	BSP_LED_Init(LED2);

	/* Hard button */
	BSP_PB_Init(BUTTON_USER, BUTTON_MODE_GPIO);

	/* Initialize the SDRAM */
	BSP_SDRAM_Init();

	/* Initialize the Touch screen */
	BSP_TS_Init(800, 480);

	/* Enable CRC to Unlock GUI */
	__HAL_RCC_CRC_CLK_ENABLE();

	/* Enable Back up SRAM */
	__HAL_RCC_BKPSRAM_CLK_ENABLE();

	CAN_Init();

	SPI_Init();
}

static void CAN_Init(){
	hcan1.Instance = CAN1;
	hcan1.Init.Prescaler = 5;
	hcan1.Init.Mode = CAN_MODE_NORMAL;
	hcan1.Init.SJW = CAN_SJW_1TQ;
	hcan1.Init.BS1 = CAN_BS1_16TQ;
	hcan1.Init.BS2 = CAN_BS2_3TQ;
	hcan1.Init.TTCM = DISABLE;
	hcan1.Init.ABOM = DISABLE;
	hcan1.Init.AWUM = DISABLE;
	hcan1.Init.NART = DISABLE;
	hcan1.Init.RFLM = DISABLE;
	hcan1.Init.TXFP = DISABLE;
	if(HAL_CAN_Init(&hcan1) != HAL_OK)
		ErrorHandler();
}

static void SPI_Init(){
	SpiHandle.Instance               = SPI2;
//	SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
	SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
	SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
	SpiHandle.Init.CLKPolarity       = SPI_POLARITY_HIGH;
	SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
	SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
	SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
	SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
	SpiHandle.Init.CRCPolynomial     = 7;
	SpiHandle.Init.NSS               = SPI_NSS_SOFT;
	SpiHandle.Init.Mode = SPI_MODE_SLAVE;
	HAL_Delay(5);
	if(HAL_SPI_Init(&SpiHandle) != HAL_OK)
		ErrorHandler();
}


void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan){
	GPIO_InitTypeDef GPIO_InitStruct;
	if(hcan->Instance == CAN1){
		__HAL_RCC_CAN1_CLK_ENABLE();
		GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* hcan){
	if(hcan->Instance == CAN1){
		__HAL_RCC_CAN1_CLK_DISABLE();
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8 | GPIO_PIN_9);
	}
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
GPIO_InitTypeDef  GPIO_InitStruct;

  if (hspi->Instance == SPIx)
  {
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    SPIx_SCK_GPIO_CLK_ENABLE();
    SPIx_MISO_GPIO_CLK_ENABLE();
    SPIx_MOSI_GPIO_CLK_ENABLE();
    /* Enable SPI2 clock */
    SPIx_CLK_ENABLE();
    /* Enable DMA clock */
    DMAx_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* SPI SCK GPIO pin configuration  */
    GPIO_InitStruct.Pin       = SPIx_SCK_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = SPIx_SCK_AF;
    HAL_GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStruct);

    /* SPI MISO GPIO pin configuration  */
    GPIO_InitStruct.Pin = SPIx_MISO_PIN;
    GPIO_InitStruct.Alternate = SPIx_MISO_AF;
    HAL_GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStruct);

    /* SPI MOSI GPIO pin configuration  */
    GPIO_InitStruct.Pin = SPIx_MOSI_PIN;
    GPIO_InitStruct.Alternate = SPIx_MOSI_AF;
    HAL_GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStruct);

    /*##-3- Configure the DMA ##################################################*/
    /* Configure the DMA handler for Transmission process */
    hdma_tx.Instance                 = SPIx_TX_DMA_STREAM;
    hdma_tx.Init.Channel             = SPIx_TX_DMA_CHANNEL;
    hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_tx.Init.MemBurst            = DMA_MBURST_INC4;
    hdma_tx.Init.PeriphBurst         = DMA_PBURST_INC4;
    hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_tx.Init.Mode                = DMA_NORMAL;
    hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;

    HAL_DMA_Init(&hdma_tx);

    /* Associate the initialized DMA handle to the the SPI handle */
    __HAL_LINKDMA(hspi, hdmatx, hdma_tx);

    /* Configure the DMA handler for Transmission process */
    hdma_rx.Instance                 = SPIx_RX_DMA_STREAM;

    hdma_rx.Init.Channel             = SPIx_RX_DMA_CHANNEL;
    hdma_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_rx.Init.MemBurst            = DMA_MBURST_INC4;
    hdma_rx.Init.PeriphBurst         = DMA_PBURST_INC4;
    hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_rx.Init.Mode                = DMA_NORMAL;
    hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;

    HAL_DMA_Init(&hdma_rx);

    /* Associate the initialized DMA handle to the the SPI handle */
    __HAL_LINKDMA(hspi, hdmarx, hdma_rx);

    /*##-4- Configure the NVIC for DMA #########################################*/
    /* NVIC configuration for DMA transfer complete interrupt (SPI2_TX) */
    HAL_NVIC_SetPriority(SPIx_DMA_TX_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(SPIx_DMA_TX_IRQn);

    /* NVIC configuration for DMA transfer complete interrupt (SPI2_RX) */
    HAL_NVIC_SetPriority(SPIx_DMA_RX_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(SPIx_DMA_RX_IRQn);
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
  if(hspi->Instance == SPIx)
  {
    /*##-1- Reset peripherals ##################################################*/
    SPIx_FORCE_RESET();
    SPIx_RELEASE_RESET();

    /*##-2- Disable peripherals and GPIO Clocks ################################*/
    /* Deconfigure SPI SCK */
    HAL_GPIO_DeInit(SPIx_SCK_GPIO_PORT, SPIx_SCK_PIN);
    /* Deconfigure SPI MISO */
    HAL_GPIO_DeInit(SPIx_MISO_GPIO_PORT, SPIx_MISO_PIN);
    /* Deconfigure SPI MOSI */
    HAL_GPIO_DeInit(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_PIN);

    /*##-3- Disable the DMA ####################################################*/
    /* De-Initialize the DMA associated to transmission process */
    HAL_DMA_DeInit(&hdma_tx);
    /* De-Initialize the DMA associated to reception process */
    HAL_DMA_DeInit(&hdma_rx);

    /*##-4- Disable the NVIC for DMA ###########################################*/
    HAL_NVIC_DisableIRQ(SPIx_DMA_TX_IRQn);
    HAL_NVIC_DisableIRQ(SPIx_DMA_RX_IRQn);

    /*##-5- Configure the NVIC for SPI #########################################*/
    /* NVIC for SPI */
    HAL_NVIC_SetPriority(SPIx_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(SPIx_IRQn);
  }
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi){
	wTransferState = TRANSFER_COMPLETE;
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi){
	wTransferState = TRANSFER_ERROR;
}

void touchUpdate(void){
	static GUI_PID_STATE TS_State = {0, 0, 0, 0};
	__IO TS_StateTypeDef  ts;
	uint16_t xDiff, yDiff;

	BSP_TS_GetState((TS_StateTypeDef *)&ts);

	if(ts.touchX[0] >= LCD_GetXSize()){
		ts.touchX[0] = 0;
	}

	if(ts.touchY[0] >= LCD_GetYSize()){
		ts.touchY[0] = 0;
	}

	xDiff = (TS_State.x > ts.touchX[0]) ? (TS_State.x - ts.touchX[0]) : (ts.touchX[0] - TS_State.x);
	yDiff = (TS_State.y > ts.touchY[0]) ? (TS_State.y - ts.touchY[0]) : (ts.touchY[0] - TS_State.y);

	if((TS_State.Pressed != ts.touchDetected ) || (xDiff > 20 )|| (yDiff > 20)){
		TS_State.Pressed = ts.touchDetected;
		if(ts.touchDetected){
			TS_State.x = ts.touchX[0];
			if(ts.touchY[0] < 240){
				TS_State.y = ts.touchY[0] ;
			} else {
				TS_State.y = (ts.touchY[0] * 480) / 445;
			}
			GUI_TOUCH_StoreStateEx(&TS_State);
		} else {
			GUI_TOUCH_StoreStateEx(&TS_State);
			TS_State.x = 0;
			TS_State.y = 0;
		}
	}
}
