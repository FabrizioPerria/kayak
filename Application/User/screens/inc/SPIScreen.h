/*
 * SPIScreen.h
 *
 *  Created on: May 16, 2017
 *      Author: jack
 */

#ifndef APPLICATION_USER_CORE_INC_SPISCREEN_H_
#define APPLICATION_USER_CORE_INC_SPISCREEN_H_

#include "DIALOG.h"

#define ID_WINDOW_0  (GUI_ID_USER + 0x01)
#define ID_MULTIEDIT_0  (GUI_ID_USER + 0x03)
#define ID_BACK_BUTTON  (GUI_ID_USER + 0x07)
#define ID_TEXT_0  (GUI_ID_USER + 0x08)
#define ID_LOGO_KAYAK (GUI_ID_USER + 0x09)

//const GUI_WIDGET_CREATE_INFO _aDialogSPICreate[] = {
//	{ WINDOW_CreateIndirect, "Window", ID_WINDOW_0, 0, 0, 800, 480, 0, 0x0, 0 },
//	{ MULTIEDIT_CreateIndirect, "Multiedit", ID_MULTIEDIT_0, 0, 105, 800, 375, 0, 0x0, 0 },
//	{ BUTTON_CreateIndirect, "Button", ID_BACK_BUTTON, 0, 0, 80, 80, 0, 0x0, 0 },
//	{ TEXT_CreateIndirect, "SPI data", ID_TEXT_0, 356, 37, 80, 20, 0, 0x0, 0 },
//};

void _cbSPIDialog(WM_MESSAGE * pMsg);

#endif /* APPLICATION_USER_CORE_INC_SPISCREEN_H_ */
