#include "SPIScreen.h"
#include "ExplorerDLG.h"
#include "storage.h"
#include "bspManager.h"
#include "main.h"
#include "cmsis_os.h"
//#include "panelsScreen.h"

#ifdef _USE_BITMAP_PICTURES

extern GUI_CONST_STORAGE GUI_BITMAP bmbackRelease;
extern GUI_CONST_STORAGE GUI_BITMAP bmbackPush;
extern GUI_CONST_STORAGE GUI_BITMAP bmlogo;

#endif

#define USER_DATA 15
#define NUM_BYTES (9)

volatile uint32_t wTransferState;
static WM_HWIN hWin;
const char closeCmdd = 1;
const char cmdContinue = 0;

uint8_t rxBuffer1[NUM_BYTES];
uint8_t rxBuffer2[NUM_BYTES];
uint8_t rxBuffer3[NUM_BYTES];
static int idxBuffer;

uint8_t *tripleBuffer[] = {rxBuffer1, rxBuffer2, rxBuffer3};
static QueueHandle_t queue;
WM_MESSAGE msg;

void receiveSPIMessage(void)
{
	SPI_Resume();

	char quit = 0;
	idxBuffer = 0;
	int cnt = 100;
	memset(rxBuffer1, 0, NUM_BYTES);
	memset(rxBuffer2, 0, NUM_BYTES);
	memset(rxBuffer3, 0, NUM_BYTES);

	while(!SPIx_NSS_PIN);

	SPI_Receive(tripleBuffer[idxBuffer], NUM_BYTES);
	wTransferState = TRANSFER_WAIT;
	xQueueReceive(queue, &quit, 0);
	while(!quit){
		while(wTransferState == TRANSFER_WAIT){
		}

		SPI_Receive(tripleBuffer[(idxBuffer + 1 ) % 3], NUM_BYTES);

//		SPI_Receive(tripleBuffer[0], NUM_BYTES);
		wTransferState = TRANSFER_WAIT;
		//Notify GUI thread
		msg.MsgId = WM_USER;
		WM_SendMessage(hWin, &msg);
		//xQueueReceive(queue, &quit, 0);
	}
	SPI_Pause();
	vQueueDelete(queue);
	queue = NULL;
	GUI_EndDialog(hWin, 0);
	vTaskDelete(NULL);
}

void _cbSPIDialog(WM_MESSAGE * pMsg) {
	WM_HWIN hItem;
	int     NCode;
	int     Id;
	unsigned char* mesg;
	int prevIdx;

	switch (pMsg->MsgId) {
	case WM_USER:
	case 35:
	case 992:
		prevIdx = idxBuffer;
		mesg = tripleBuffer[idxBuffer++];
		idxBuffer %= 3;

		xQueueSend(queue, &cmdContinue, 0);
		hItem = WM_GetDialogItem(pMsg->hWin, ID_MULTIEDIT_0);
		char str[4 * NUM_BYTES];

		char* idx = str;
//		sprintf(idx, "%d ", prevIdx);
//		idx += 2;
		for (int i = 0; i < NUM_BYTES; ++i) {
			sprintf(idx, "%.2x ", *(mesg++));
			idx += 3;
		}
//		memset(tripleBuffer[prevIdx], 0, NUM_BYTES);
		sprintf(idx++,"\n");

		MULTIEDIT_AddText(hItem, str);
		break;
	case WM_INIT_DIALOG:
		hWin = pMsg->hWin;
		queue = xQueueCreate(1,1);
		osThreadDef(receiveSPIMessage, receiveSPIMessage, osPriorityNormal, 0, 2 * 1024);
		osThreadCreate (osThread(receiveSPIMessage), NULL);

#ifdef _USE_BITMAP_PICTURES
		hItem = IMAGE_CreateEx(679, 0, 121, 91, pMsg->hWin, WM_CF_SHOW, 0, ID_LOGO_KAYAK);
		IMAGE_SetBitmap(hItem, &bmlogo);
#endif
		hItem = WM_GetDialogItem(pMsg->hWin, ID_MULTIEDIT_0);
		MULTIEDIT_SetText(hItem, "");
		MULTIEDIT_SetAutoScrollV(hItem, 1);
		MULTIEDIT_SetFocussable(hItem, 0);
		MULTIEDIT_SetReadOnly(hItem, 1);
		MULTIEDIT_SetFont(hItem, GUI_FONT_20_ASCII);

//		hItem = BUTTON_CreateAsChild(0, 0, 91, 100, pMsg->hWin, ID_BACK_BUTTON, WM_CF_SHOW);
//		BUTTON_SetTextColor(hItem, 0, GUI_WHITE);
//		BUTTON_SetFont(hItem, GUI_FONT_20_1);
//#ifdef _USE_BITMAP_PICTURES
//		BUTTON_SetBitmap(hItem, BUTTON_BI_UNPRESSED, &bmbackRelease);
//		BUTTON_SetBitmap(hItem, BUTTON_BI_PRESSED, &bmbackPush);
//#else
//		BUTTON_SetText(hItem, "Back");
//#endif
//		hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0);
//		TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x000000FF));
//		TEXT_SetFont(hItem, GUI_FONT_20_ASCII);
	break;
	case WM_NOTIFY_PARENT:
		Id    = WM_GetId(pMsg->hWinSrc);
		NCode = pMsg->Data.v;
		switch(Id) {
		case ID_BACK_BUTTON: // Notifications sent by 'Button'
			switch(NCode) {
			case WM_NOTIFICATION_CLICKED:
			break;
			case WM_NOTIFICATION_RELEASED:
				if (queue){
					xQueueSend(queue, &closeCmdd, 0);
				}
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
//	osMutexCreate(&mutex);
//	hWin = GUI_CreateDialogBox(_aDialogSPICreate, GUI_COUNTOF(_aDialogSPICreate), _cbSPIDialog, WM_HBKWIN, 0, 0);
//	queue = xQueueCreate(1,1);
//	osThreadDef(receiveSPIMessage, receiveSPIMessage, osPriorityNormal, 0, 2 * 1024);
//	osThreadCreate (osThread(receiveSPIMessage), NULL);
//	openPanels();
	return hWin;
}
