#include "main.h"

#include "storage.h"
#include "bspManager.h"
#include "mpu.h"
#include "rtc.h"

#define USE_BANDING_MEMDEV
static void SystemClock_Config(void);
static void MainThread(void);
static void CPU_CACHE_Enable(void);
static void setupTheme(void);

#if (configAPPLICATION_ALLOCATED_HEAP == 1)
	uint8_t  ucHeap[configTOTAL_HEAP_SIZE];
#endif

int main(void)
{
	/* Configure the MPU attributes as Write Through */
	MPU_Config();

	CPU_CACHE_Enable();

	/* STM32F7xx HAL library initialization:
	- Configure the Flash ART accelerator on ITCM interface
	- Configure the Systick to generate an interrupt each 1 msec
	- Set NVIC Group Priority to 4
	- Global MSP (MCU Support Package) initialization
	*/
	HAL_Init();

	/* Configure the system clock @ 200 Mhz */
	SystemClock_Config();

	/* Initialize RTC */
	rtcInit();

	/* Configure the board */
	bspManagerInit();

	/* Create GUI task */
	osThreadDef(MainThread, MainThread, osPriorityNormal, 0, 1024);
	osThreadCreate (osThread(MainThread), NULL);
	/* Initialize GUI */
	GUI_Init();

	WM_MULTIBUF_Enable(1);
	GUI_SetLayerVisEx (1, 0);
	GUI_SelectLayer(0);

	GUI_SetBkColor(GUI_WHITE);
	GUI_Clear();

	setupTheme();

	/* Start scheduler */
	osKernelStart ();

	ErrorHandler();
}

static void MainThread(void)
{
	/* Initialize Storage Units */
	StorageInit();

	/* Create Touch screen Timer */
	osTimerDef(touchUpdate, touchUpdate);
	osTimerId timer = osTimerCreate(osTimer(touchUpdate), osTimerPeriodic, NULL);
	/* Start the TS Timer */
	osTimerStart(timer, 20);

	/* Show the main menu */
	initMainWindow();

	/* Gui background Task */
	while (1) {
		GUI_Exec(); /* Do the background work ... Update windows etc.) */
		osDelay(10);
	}
}

static void setupTheme(void)
{
	GUI_SetDefaultFont(&GUI_Font24_ASCII);

	BUTTON_SetDefaultSkin(BUTTON_SKIN_FLEX);
	BUTTON_SetDefaultTextColor(GUI_BLACK, BUTTON_CI_UNPRESSED);
	BUTTON_SetDefaultTextColor(GUI_BLACK, BUTTON_CI_PRESSED);

	PROGBAR_SetDefaultSkin(PROGBAR_SKIN_FLEX);

	DROPDOWN_SetDefaultSkin(DROPDOWN_SKIN_FLEX);
	DROPDOWN_SetDefaultFont(&GUI_Font32_ASCII);

	FRAMEWIN_SetDefaultTextColor(FRAMEWIN_CI_INACTIVE, GUI_WHITE);
	FRAMEWIN_SetDefaultTextColor(FRAMEWIN_CI_ACTIVE, GUI_WHITE);
	FRAMEWIN_SetDefaultBarColor(FRAMEWIN_CI_INACTIVE, GUI_LIGHTBLUE);
	FRAMEWIN_SetDefaultBarColor(FRAMEWIN_CI_ACTIVE, GUI_LIGHTBLUE);
	FRAMEWIN_SetDefaultFont(&GUI_Font32_ASCII);
	FRAMEWIN_SetDefaultClientColor(GUI_WHITE);
	FRAMEWIN_SetDefaultTitleHeight(37);

	WINDOW_SetDefaultBkColor(GUI_WHITE);

	LISTVIEW_SetDefaultGridColor(GUI_WHITE);
	LISTVIEW_SetDefaultFont(&GUI_Font20_ASCII);

	SCROLLBAR_SetDefaultWidth(44);

	HEADER_SetDefaultBkColor(GUI_LIGHTBLUE);
	HEADER_SetDefaultTextColor(GUI_WHITE);
	HEADER_SetDefaultFont(&GUI_Font24_ASCII);

	TEXT_SetDefaultTextColor(GUI_BLACK);

	MULTIPAGE_SetDefaultFont(&GUI_Font32_ASCII);

	RADIO_SetDefaultFont(&GUI_Font24_ASCII);
}

/**
* @brief  System Clock Configuration
*         The system Clock is configured as follow :
*            System Clock source            = PLL (HSE)
*            SYSCLK(Hz)                     = 200000000
*            HCLK(Hz)                       = 200000000
*            AHB Prescaler                  = 1
*            APB1 Prescaler                 = 4
*            APB2 Prescaler                 = 2
*            HSE Frequency(Hz)              = 25000000
*            PLL_M                          = 25
*            PLL_N                          = 400
*            PLL_P                          = 2
*            PLLSAI_N                       = 384
*            PLLSAI_P                       = 8
*            VDD(V)                         = 3.3
*            Main regulator output voltage  = Scale1 mode
*            Flash Latency(WS)              = 6
* @param  None
* @retval None
*/
void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;
	HAL_StatusTypeDef ret = HAL_OK;

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 400;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 9;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	ret = HAL_PWREx_EnableOverDrive();

	if(ret != HAL_OK)
		ErrorHandler();

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
	 clocks dividers */
	RCC_ClkInitStruct.ClockType = (	RCC_CLOCKTYPE_SYSCLK |
									RCC_CLOCKTYPE_HCLK |
									RCC_CLOCKTYPE_PCLK1 |
									RCC_CLOCKTYPE_PCLK2);

	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
}


void HAL_Delay (__IO uint32_t Delay)
{
	while (Delay) {
		if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
			Delay--;
		}
	}
}

static void CPU_CACHE_Enable(void)
{
	/* Invalidate I-Cache : ICIALLU register*/
	SCB_InvalidateICache();

	/* Enable branch prediction */
	SCB->CCR |= (1 <<18);
	__DSB();

	/* Invalidate I-Cache : ICIALLU register*/
	SCB_InvalidateICache();

	/* Enable I-Cache */
	SCB_EnableICache();

	SCB_InvalidateDCache();
	SCB_EnableDCache();
}

void CleanDisplay (uint32_t address)
{
	/* Set up mode */
	DMA2D->CR		= 0x00030000UL | (1 << 9);
	DMA2D->OCOLR	= 0x00;

	/* Set up pointers */
	DMA2D->OMAR		= address;

	/* Set up offsets */
	DMA2D->OOR		 = 0;

	/* Set up pixel format */
	DMA2D->OPFCCR	= LTDC_PIXEL_FORMAT_RGB888;

	/*	Set up size */
	DMA2D->NLR		 = (U32)(800 << 16) | (U16)480;

	DMA2D->CR		 |= DMA2D_CR_START;

	/* Wait until transfer is done */
	while (DMA2D->CR & DMA2D_CR_START);
}

int WaitForDialog(WM_HWIN hDialog)
{
	WM_DIALOG_STATUS DialogStatus = {0};

	GUI_SetDialogStatusPtr(hDialog, &DialogStatus);
	while (!DialogStatus.Done) {
		if (!GUI_Exec()) {
			GUI_Delay(100);
		}
	}
	return DialogStatus.ReturnValue;
}

void ErrorHandler(void)
{
	BSP_LED_On(LED2);
	BSP_LED_Off(LED1);

	while (1) {
		BSP_LED_Toggle(LED1);
		BSP_LED_Toggle(LED2);
		HAL_Delay(50);
	}
}
