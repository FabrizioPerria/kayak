#include "DIALOG.h"
#include "ExplorerDLG.h"
#include "storage.h"
#include "bspManager.h"

#ifdef _USE_BITMAP_PICTURES

extern GUI_CONST_STORAGE GUI_BITMAP bmbackRelease;
extern GUI_CONST_STORAGE GUI_BITMAP bmbackPush;
extern GUI_CONST_STORAGE GUI_BITMAP bmlogo;

#endif

#define ID_WINDOW_0  (GUI_ID_USER + 0x01)
#define ID_MULTIEDIT_0  (GUI_ID_USER + 0x03)
#define ID_STORE_BUTTON  (GUI_ID_USER + 0x04)
#define ID_BACK_BUTTON  (GUI_ID_USER + 0x07)
#define ID_TEXT_0  (GUI_ID_USER + 0x08)
#define ID_LOGO_KAYAK (GUI_ID_USER + 0x09)
#define USER_DATA 15

volatile uint32_t wTransferState;
extern SPI_HandleTypeDef SpiHandle;
static WM_HWIN hWin;
const char closeCmdd = 1;

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
	{ WINDOW_CreateIndirect, "Window", ID_WINDOW_0, 0, 0, 800, 480, 0, 0x0, 0 },
	{ MULTIEDIT_CreateIndirect, "Multiedit", ID_MULTIEDIT_0, 0, 95, 800, 320, 0, 0x0, 0 },
	{ BUTTON_CreateIndirect, "Button", ID_STORE_BUTTON, 1, 419, 798, 60, 0, 0x0, 0 },
	{ BUTTON_CreateIndirect, "Button", ID_BACK_BUTTON, 0, 0, 80, 80, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "SPI data", ID_TEXT_0, 356, 37, 80, 20, 0, 0x0, 0 },
};


uint8_t rxBuffer[120];
static QueueHandle_t queue;
WM_MESSAGE msg;

static void receiveSPIMessage(void)
{
	msg.MsgId = WM_USER;
	char quit = 0;
	xQueueReceive(queue, &quit, 0);
	while(!quit){
		wTransferState = TRANSFER_WAIT;
		if(HAL_SPI_Receive_DMA(&SpiHandle, rxBuffer, 111) != HAL_OK)
			ErrorHandler();

		while(wTransferState == TRANSFER_WAIT);

		//Notify GUI thread
		msg.Data.p = &rxBuffer;
		WM_SendMessage(hWin, &msg);
		xQueueReceive(queue, &quit, 0);
	}
	vQueueDelete(queue);
	queue = NULL;
	vTaskDelete(NULL);
}

static void _cbDialog(WM_MESSAGE * pMsg) {
	WM_HWIN hItem;
	int     NCode;
	int     Id;

	switch (pMsg->MsgId) {
	case WM_USER:
		hItem = WM_GetDialogItem(pMsg->hWin, ID_MULTIEDIT_0);
		MULTIEDIT_AddText(hItem, (const char*)pMsg->Data.p);
		break;
	case WM_INIT_DIALOG:
#ifdef _USE_BITMAP_PICTURES
		hItem = IMAGE_CreateEx(679, 0, 121, 91, pMsg->hWin, WM_CF_SHOW, 0, ID_LOGO_KAYAK);
		IMAGE_SetBitmap(hItem, &bmlogo);
#endif
		hItem = WM_GetDialogItem(pMsg->hWin, ID_MULTIEDIT_0);
		MULTIEDIT_SetText(hItem, "");
		MULTIEDIT_SetFont(hItem, GUI_FONT_16_ASCII);

		hItem = WM_GetDialogItem(pMsg->hWin, ID_STORE_BUTTON);
		BUTTON_SetText(hItem, "Save...");

		hItem = BUTTON_CreateAsChild(0, 0, 91, 100, pMsg->hWin, ID_BACK_BUTTON, WM_CF_SHOW);
		BUTTON_SetTextColor(hItem, 0, GUI_WHITE);
		BUTTON_SetFont(hItem, GUI_FONT_20_1);
#ifdef _USE_BITMAP_PICTURES
		BUTTON_SetBitmap(hItem, BUTTON_BI_UNPRESSED, &bmbackRelease);
		BUTTON_SetBitmap(hItem, BUTTON_BI_PRESSED, &bmbackPush);
#else
		BUTTON_SetText(hItem, "Back");
#endif
		hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0);
		TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x000000FF));
		TEXT_SetFont(hItem, GUI_FONT_20_ASCII);
	break;
	case WM_NOTIFY_PARENT:
		Id    = WM_GetId(pMsg->hWinSrc);
		NCode = pMsg->Data.v;
		switch(Id) {
		case ID_STORE_BUTTON: // Notifications sent by 'Button'
			switch(NCode) {
			case WM_NOTIFICATION_CLICKED:
			break;
			case WM_NOTIFICATION_RELEASED:
			break;
		}
		break;
		case ID_BACK_BUTTON: // Notifications sent by 'Button'
			switch(NCode) {
			case WM_NOTIFICATION_CLICKED:
			break;
			case WM_NOTIFICATION_RELEASED:
				if(queue){
					xQueueSend(queue, &closeCmdd, 0);
				}
				GUI_EndDialog(pMsg->hWin, 0);
			break;
			}
		break;
		}
	break;
	default:
		WM_DefaultProc(pMsg);
	break;
	}
}

WM_HWIN storeSPIWindow(void)
{
	int res = RET_IS_FILE;
	while (res == RET_IS_FILE) {
		FileInfo fileInfoInt;
		res = WaitForDialog(CreateExplorer(&fileInfoInt));
		if (res == RET_IS_DIR) {
			hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
			queue = xQueueCreate(1,1);
			xTaskGenericCreate(receiveSPIMessage, "receiveSPIMessage", 4 * 1024, 0, osPriorityNormal - osPriorityIdle, NULL, NULL, NULL);
		} else if (res == RET_IS_FILE) {
			WaitForDialog(CreateMessageBox("Please select a folder"));
		}
	}

	return hWin;
}
