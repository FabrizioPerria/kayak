#include "DIALOG.h"
#include "ExplorerDLG.h"
#include "storage.h"
#include "bspManager.h"
#include "main.h"
#include "cmsis_os.h"

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
#define NUM_BYTES 10

volatile uint32_t wTransferState;
static WM_HWIN hWin;
const char closeCmdd = 1;
const char cmdContinue = 0;

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
	{ WINDOW_CreateIndirect, "Window", ID_WINDOW_0, 0, 0, 800, 480, 0, 0x0, 0 },
	{ MULTIEDIT_CreateIndirect, "Multiedit", ID_MULTIEDIT_0, 0, 105, 800, 320, 0, 0x0, 0 },
	{ BUTTON_CreateIndirect, "Button", ID_STORE_BUTTON, 1, 419, 798, 60, 0, 0x0, 0 },
	{ BUTTON_CreateIndirect, "Button", ID_BACK_BUTTON, 0, 0, 80, 80, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "SPI data", ID_TEXT_0, 356, 37, 80, 20, 0, 0x0, 0 },
};


uint8_t rxBuffer1[NUM_BYTES];
uint8_t rxBuffer2[NUM_BYTES];
uint8_t rxBuffer3[NUM_BYTES];
static int idxBuffer;

uint8_t *tripleBuffer[] = {rxBuffer1, rxBuffer2, rxBuffer3};
static QueueHandle_t queue;
WM_MESSAGE msg;

static void receiveSPIMessage(void)
{
	SPI_Resume();
	msg.MsgId = WM_USER;
	char quit = 0;

	SPI_Receive(tripleBuffer[idxBuffer], NUM_BYTES);
	wTransferState = TRANSFER_WAIT;
	xQueueReceive(queue, &quit, 0);
	while(!quit){
		while(wTransferState == TRANSFER_WAIT);
		SPI_Receive(tripleBuffer[(idxBuffer + 1 ) % 3], NUM_BYTES);
		wTransferState = TRANSFER_WAIT;
		//Notify GUI thread
		WM_SendMessage(hWin, &msg);
		xQueueReceive(queue, &quit, 0);
	}
	SPI_Pause();
	vQueueDelete(queue);
	queue = NULL;
	vTaskDelete(NULL);
}

static void _cbDialog(WM_MESSAGE * pMsg) {
	WM_HWIN hItem;
	int     NCode;
	int     Id;
	unsigned char* mesg;
	switch (pMsg->MsgId) {
	case WM_USER:
		mesg = tripleBuffer[idxBuffer++];
		idxBuffer %= 3;

		xQueueSend(queue, &cmdContinue, 0);
		hItem = WM_GetDialogItem(pMsg->hWin, ID_MULTIEDIT_0);
		char str[4 * NUM_BYTES];

		char* idx = str;
		for (int i = 0; i < NUM_BYTES; ++i) {
			sprintf(idx, "%.2x ", *(mesg++));
			idx += 3;
		}
		sprintf(idx, "\n");
		MULTIEDIT_AddText(hItem, str);
		break;
	case WM_INIT_DIALOG:
#ifdef _USE_BITMAP_PICTURES
		hItem = IMAGE_CreateEx(679, 0, 121, 91, pMsg->hWin, WM_CF_SHOW, 0, ID_LOGO_KAYAK);
		IMAGE_SetBitmap(hItem, &bmlogo);
#endif
		hItem = WM_GetDialogItem(pMsg->hWin, ID_MULTIEDIT_0);
		MULTIEDIT_SetText(hItem, "");
		MULTIEDIT_SetAutoScrollV(hItem, 1);
//		MULTIEDIT_SetFocussable(hItem, 0);
		MULTIEDIT_SetReadOnly(hItem, 1);
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
					HAL_Delay(200);
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

	hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
	queue = xQueueCreate(1,1);
	osThreadDef(receiveSPIMessage, receiveSPIMessage, osPriorityNormal, 0, 1024);
	osThreadCreate (osThread(receiveSPIMessage), NULL);
	return hWin;
}
