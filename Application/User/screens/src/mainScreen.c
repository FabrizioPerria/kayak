/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "storage.h"

#ifdef _USE_BITMAP_PICTURES

extern GUI_CONST_STORAGE GUI_BITMAP bmbackground;
extern GUI_CONST_STORAGE GUI_BITMAP bmleftRelease;
extern GUI_CONST_STORAGE GUI_BITMAP bmleftPush;
extern GUI_CONST_STORAGE GUI_BITMAP bmrightPush;
extern GUI_CONST_STORAGE GUI_BITMAP bmrightRelease;
extern GUI_CONST_STORAGE GUI_BITMAP bmcenter_press;
extern GUI_CONST_STORAGE GUI_BITMAP bmcenter_release;

#endif

#define ID_MAIN_WINDOW  		(GUI_ID_USER + 0x00)
#define ID_BACKGROUND_IMAGE		(GUI_ID_USER + 0x01)
#define ID_RUNLOG_BUTTON  		(GUI_ID_USER + 0x02)
#define ID_RUNPANELS_BUTTON  	(GUI_ID_USER + 0x03)
#define ID_STORELOG_BUTTON		(GUI_ID_USER + 0x05)

static const GUI_WIDGET_CREATE_INFO _mainWindow[] = {
	{ WINDOW_CreateIndirect, "Window", ID_MAIN_WINDOW, 0, 0, 800, 480, 0, 0x0, 0 },
	{ BUTTON_CreateIndirect, "Run Log", ID_RUNLOG_BUTTON, 0, 240, 200, 100, 0, 0, 0},
	{ BUTTON_CreateIndirect, "Store Log", ID_STORELOG_BUTTON, 600, 240, 200, 100, 0, 0, 0},
	{ BUTTON_CreateIndirect, "Open Panels", ID_RUNPANELS_BUTTON, 400, 320, 200, 100, 0, 0, 0},
	{ IMAGE_CreateIndirect, "Image", ID_BACKGROUND_IMAGE, 0, 0, 800, 480, WM_CF_BGND , IMAGE_CF_AUTOSIZE, 0 }
};

static void windowCallback(WM_MESSAGE * pMsg)
{
	WM_HWIN hItem;

	switch (pMsg->MsgId) {
	case WM_INIT_DIALOG:
		WM_EnableWindow(pMsg->hWin);

#ifdef _USE_BITMAP_PICTURES
		hItem = WM_GetDialogItem(pMsg->hWin, ID_RUNLOG_BUTTON);
		BUTTON_SetTextColor(hItem, BUTTON_BI_UNPRESSED, GUI_WHITE);
		BUTTON_SetFont(hItem, GUI_FONT_20_1);
		WM_SetWindowPos(hItem, 0, 284, 214, 99);
		BUTTON_SetBitmap(hItem, BUTTON_BI_UNPRESSED, &bmleftRelease);
		BUTTON_SetBitmap(hItem, BUTTON_BI_PRESSED, &bmleftPush);
#endif

#ifdef _USE_BITMAP_PICTURES
		hItem = WM_GetDialogItem(pMsg->hWin, ID_STORELOG_BUTTON);
		BUTTON_SetTextColor(hItem, BUTTON_BI_UNPRESSED, GUI_WHITE);
		BUTTON_SetFont(hItem, GUI_FONT_20_1);
		WM_SetWindowPos(hItem, 586, 284, 214, 99);
		BUTTON_SetBitmap(hItem, BUTTON_BI_UNPRESSED, &bmrightRelease);
		BUTTON_SetBitmap(hItem, BUTTON_BI_PRESSED, &bmrightPush);
#endif

#ifdef _USE_BITMAP_PICTURES
		hItem = WM_GetDialogItem(pMsg->hWin, ID_RUNPANELS_BUTTON);
		BUTTON_SetTextColor(hItem, BUTTON_BI_UNPRESSED, GUI_WHITE);
		BUTTON_SetFont(hItem, GUI_FONT_20_1);
		WM_SetWindowPos(hItem, 289, 380, 223, 86);
		BUTTON_SetBitmap(hItem, BUTTON_BI_UNPRESSED, &bmcenter_release);
		BUTTON_SetBitmap(hItem, BUTTON_BI_PRESSED, &bmcenter_press);
#endif

#ifdef _USE_BITMAP_PICTURES
	    hItem = WM_GetDialogItem(pMsg->hWin, ID_BACKGROUND_IMAGE);
	    IMAGE_SetBitmap(hItem, &bmbackground);
#endif
	    break;

	case WM_PAINT:
		GUI_SetBkColor(GUI_WHITE);
		GUI_Clear();
		break;

	case WM_NOTIFY_PARENT:
		switch (WM_GetId(pMsg->hWinSrc)) {
		case ID_RUNLOG_BUTTON:
			switch (pMsg->Data.v) {
			case WM_NOTIFICATION_RELEASED:
				runTestWindow();
				break;
			}
			break;
		case ID_STORELOG_BUTTON:
			switch(pMsg->Data.v) {
			case WM_NOTIFICATION_RELEASED:
				storeSPIWindow();
				break;
			}
			break;
		case ID_RUNPANELS_BUTTON:
			switch (pMsg->Data.v) {
			case WM_NOTIFICATION_RELEASED:
				openPanels();
				break;
			}
			break;
		}
		break;
	default:
		WM_DefaultProc(pMsg);
	}
}

void initMainWindow(void)
{
	GUI_CreateDialogBox(_mainWindow, GUI_COUNTOF(_mainWindow), windowCallback, WM_HBKWIN, 0, 0);
//	CleanDisplay(LCD_FB_START_ADDRESS);
}
