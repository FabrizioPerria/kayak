/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
 extern "C" {
#endif
   
/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "stm32f7xx_it.h"

/* EVAL includes component */
#include "stm32f769i_discovery.h"     
#include "stm32f769i_discovery_sdram.h"
#include "stm32f769i_discovery_ts.h"
#include "stm32f769i_discovery_audio.h"	 
#include "stm32f769i_discovery_qspi.h"
   
/* FatFs includes components */
#include "ff_gen_drv.h"
#include "usbh_diskio.h"
#include "sd_diskio.h"
   
/* GUI includes components */
#include "GUI.h"
#include "DIALOG.h"

 /* Kernel includes components */
#include "storage.h"
#include "rtc.h"
#include "bspManager.h"
#include "mpu.h"
   
/* utilities and standard includes components */   
#include "cpu_utils.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
   
/* Exported types ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 
void CleanDisplay (uint32_t address);
int WaitForDialog(WM_HWIN hDialog);
void ErrorHandler(void);
void initMainWindow(void);

WM_HWIN CreateMessageBox(const char* message);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
