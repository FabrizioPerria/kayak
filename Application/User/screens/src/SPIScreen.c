#include "SPIScreen.h"
#include "ExplorerDLG.h"
#include "storage.h"
#include "bspManager.h"
#include "main.h"
#include "cmsis_os.h"
#include "panelsScreen.h"

//#ifdef _USE_BITMAP_PICTURES
//
//extern GUI_CONST_STORAGE GUI_BITMAP bmbackRelease;
//extern GUI_CONST_STORAGE GUI_BITMAP bmbackPush;
//extern GUI_CONST_STORAGE GUI_BITMAP bmlogo;
//
//#endif

#define MAX_NUM_MESSAGES (100)
#define NUM_BYTES (9)

volatile uint32_t wTransferState;
static WM_HWIN hWin;

const char cmdContinueSPI = CONTINUE;
const char cmdEndPeriodic = QUIT_40S_SLEEP;


uint8_t rxBuffer[NUM_BYTES];

WM_MESSAGE msg;

void receiveSPIMessage(void)
{
	SPI_Resume();

	char quit = 0;
	memset(rxBuffer, 0, NUM_BYTES);

	while(!SPIx_NSS_PIN);

	SPI_Receive(rxBuffer, NUM_BYTES);
	wTransferState = TRANSFER_WAIT;
	xQueueReceive(queueSPI, &quit, 0);
	while(!quit){
		while(wTransferState == TRANSFER_WAIT);

		SPI_Receive(rxBuffer, NUM_BYTES);
		wTransferState = TRANSFER_WAIT;
		//Notify GUI thread
		msg.MsgId = WM_USER;
		WM_SendMessage(hWin, &msg);
		xQueueReceive(queueSPI, &quit, 0);
	}
	SPI_Pause();
	vQueueDelete(queueSPI);
	queueSPI = NULL;
	vTaskDelete(NULL);
}
uint8_t CAN_Proxy_buffer[8];

void _cbSPIDialog(WM_MESSAGE * pMsg) {
	WM_HWIN hItem;
	int     NCode;
	int     Id;
	unsigned char mesg[NUM_BYTES];
	static int num_messages = 0;

	switch (pMsg->MsgId) {
	case WM_USER:
		memcpy(mesg, rxBuffer, NUM_BYTES);

		xQueueSend(queueSPI, &cmdContinueSPI, 0);
		hItem = WM_GetDialogItem(pMsg->hWin, ID_MULTIEDIT_SPI);
		if (num_messages++ > MAX_NUM_MESSAGES) {
			num_messages = 0;
			MULTIEDIT_SetText(hItem, "");
		}
		char str[4 * NUM_BYTES];

		char* idx = str;
		int j = 0;
		for (int i = 0; i < NUM_BYTES; ++i) {
			if (i != 4)
				CAN_Proxy_buffer[j++] = mesg[i];
			sprintf(idx, "%.2x ", mesg[i]);
			idx += 3;
		}
//		CAN_Send(0x582, 8, CAN_Proxy_buffer);
		sprintf(idx++,"\n");
		if (mesg[5] == 0xB1 && mesg[8] != 0x15) {
			MULTIEDIT_SetBkColor(hItem, 1, GUI_RED);
//			xQueueSend(queueSPI, &cmdClose, 0);
			xQueueSend(queueCAN, &cmdEndPeriodic, 0);
		}

		MULTIEDIT_AddText(hItem, str);
		break;
	case WM_INIT_DIALOG:
		hWin = pMsg->hWin;
		queueSPI = xQueueCreate(1,1);
		osThreadDef(receiveSPIMessage, receiveSPIMessage, osPriorityNormal, 0, 2 * 1024);
		osThreadCreate (osThread(receiveSPIMessage), NULL);

//#ifdef _USE_BITMAP_PICTURES
//		hItem = IMAGE_CreateEx(679, 0, 121, 91, pMsg->hWin, WM_CF_SHOW, 0, ID_LOGO_KAYAK);
//		IMAGE_SetBitmap(hItem, &bmlogo);
//#endif
		hItem = WM_GetDialogItem(pMsg->hWin, ID_MULTIEDIT_SPI);
		MULTIEDIT_SetText(hItem, "");
		MULTIEDIT_SetAutoScrollV(hItem, 1);
		MULTIEDIT_SetFocussable(hItem, 0);
		MULTIEDIT_SetReadOnly(hItem, 1);
		MULTIEDIT_SetInsertMode(hItem, 0);
		MULTIEDIT_SetBkColor(hItem, 1, GUI_WHITE);
		MULTIEDIT_SetFont(hItem, GUI_FONT_20_ASCII);
	break;
	default:
		WM_DefaultProc(pMsg);
	break;
	}
}
