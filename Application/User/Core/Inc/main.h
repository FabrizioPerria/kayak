/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
 extern "C" {
#endif
   
///* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "DIALOG.h"
   
/* utilities and standard includes components */   
   
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
