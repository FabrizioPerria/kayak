#ifndef __K_RTC_H
#define __K_RTC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 
void     rtcInit(void);
void     rtcSaveParameter(uint32_t address, uint32_t data);
uint32_t rtcRestoreParameter(uint32_t address);

void rtcSetTime  (RTC_TimeTypeDef *Time);
void rtcGetTime  (RTC_TimeTypeDef *Time);
void rtcSetDate  (RTC_DateTypeDef *Date);
void rtcGetDate  (RTC_DateTypeDef *Date);

#ifdef __cplusplus
}
#endif

#endif /*__K_RTC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
