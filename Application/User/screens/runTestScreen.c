/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "canData.h"

#define MAX_LINE 100

#ifdef _USE_BITMAP_PICTURES

extern GUI_CONST_STORAGE GUI_BITMAP bmbackground;
extern GUI_CONST_STORAGE GUI_BITMAP bmleftRelease;
extern GUI_CONST_STORAGE GUI_BITMAP bmleftPush;
extern GUI_CONST_STORAGE GUI_BITMAP bmrightPush;
extern GUI_CONST_STORAGE GUI_BITMAP bmrightRelease;
extern GUI_CONST_STORAGE GUI_BITMAP bmbackRelease;
extern GUI_CONST_STORAGE GUI_BITMAP bmbackPush;

#endif

#define ID_RUNLOG_WINDOW  		(GUI_ID_USER + 0x00)
#define ID_RUNLOG_BG_IMAGE 		(GUI_ID_USER + 0x01)
#define ID_PLAYLOG_BUTTON  		(GUI_ID_USER + 0x02)
#define ID_RESETLOG_BUTTON  	(GUI_ID_USER + 0x03)
#define ID_BACKLOG_BUTTON  		(GUI_ID_USER + 0x04)
#define ID_TEXT_1  				(GUI_ID_USER + 0x05)
#define ID_LOGPROGBAR			(GUI_ID_USER + 0x06)

static const GUI_WIDGET_CREATE_INFO _runTestWindow[] = {
	{ WINDOW_CreateIndirect, "Window", ID_RUNLOG_WINDOW, 0, 0, 800, 480, 0, 0x0, 0 },
	{ BUTTON_CreateIndirect, "Play", ID_PLAYLOG_BUTTON, 0, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 4, SCREEN_HEIGHT / 5, 0, 0, 0},
	{ BUTTON_CreateIndirect, "Stop", ID_RESETLOG_BUTTON, SCREEN_WIDTH * 3 / 4, SCREEN_HEIGHT / 2, SCREEN_WIDTH / 4, SCREEN_HEIGHT / 5, 0, 0, 0},
	{ TEXT_CreateIndirect, "Text", ID_TEXT_1, 239, 320, 329, 84, 0, 0x0, 0 },
	{ IMAGE_CreateIndirect, "Image", ID_RUNLOG_BG_IMAGE, 0, 0, 800, 480, WM_CF_BGND , IMAGE_CF_AUTOSIZE, 0 }
};

static FileInfo fileInfoInt;
static WM_HWIN window;
extern CAN_HandleTypeDef hcan1;
const char closeCmd = 1;

static QueueHandle_t queue;

static void sendCanLogThread(){
	int cnt = 1;

	CanTxMsgTypeDef CAN_message;
	hcan1.pTxMsg = &CAN_message;

	WM_HWIN hProgBar = WM_GetDialogItem(window, ID_LOGPROGBAR);

	FIL logFile;
	char fileName[PATH_MAX_SIZE];

	memset(fileName, 0, PATH_MAX_SIZE);
	sprintf(fileName, "%s%s.%s", fileInfoInt.root, fileInfoInt.name, fileInfoInt.ext);

	char buffer[MAX_LINE];

	if (f_open(&logFile, (TCHAR const*)fileName, FA_READ) == FR_OK){
		int fileSize = logFile.fsize;
		char quit = 0;
		xQueueReceive(queue, &quit, 0);
		while(!f_eof(&logFile) && !quit){

			PROGBAR_SetValue(hProgBar, cnt * 100 / fileSize);
			cnt += 50;	//average line length in the file

			memset(buffer,0,MAX_LINE);
			f_gets(buffer, MAX_LINE, &logFile);
			if(buffer[0] < '0' || buffer[0] > '9'){
				continue;
			}

			CAN_message.IDE = CAN_ID_STD;

			char bufferData[24];
			memset(bufferData, 0, 24);

			int res = sscanf(buffer, "%*d.%*d %*d %x %*c%*c %*c %d %d %d %d %d %d %d %d %d\n",
					(int*)&CAN_message.StdId,
					(int*)&CAN_message.DLC,
					(int*)&CAN_message.Data[0],
					(int*)&CAN_message.Data[1],
					(int*)&CAN_message.Data[2],
					(int*)&CAN_message.Data[3],
					(int*)&CAN_message.Data[4],
					(int*)&CAN_message.Data[5],
					(int*)&CAN_message.Data[6],
					(int*)&CAN_message.Data[7]
			);

			if(res < 2){
				continue;
			}

			HAL_CAN_Transmit(&hcan1, 1);
			xQueueReceive(queue, &quit, 0);
		}
	}
	vQueueDelete(queue);
	queue = NULL;
	f_close(&logFile);
	vTaskDelete(NULL);
}

static void windowCallback(WM_MESSAGE * pMsg) {

	WM_HWIN hItem;

	switch (pMsg->MsgId){
	case WM_INIT_DIALOG:

		WM_EnableWindow(pMsg->hWin);

#ifdef _USE_BITMAP_PICTURES
		hItem = WM_GetDialogItem(pMsg->hWin, ID_PLAYLOG_BUTTON);
		BUTTON_SetFont(hItem, GUI_FONT_20_1);
		WM_SetWindowPos(hItem, 0, 284, 214, 99);
		BUTTON_SetBitmap(hItem, BUTTON_BI_UNPRESSED, &bmleftRelease);
		BUTTON_SetBitmap(hItem, BUTTON_BI_PRESSED, &bmleftPush);
#endif

#ifdef _USE_BITMAP_PICTURES
		hItem = WM_GetDialogItem(pMsg->hWin, ID_RESETLOG_BUTTON);
		BUTTON_SetFont(hItem, GUI_FONT_20_1);
		WM_SetWindowPos(hItem, 586, 284, 214, 99);
		BUTTON_SetBitmap(hItem, BUTTON_BI_UNPRESSED, &bmrightRelease);
		BUTTON_SetBitmap(hItem, BUTTON_BI_PRESSED, &bmrightPush);
#endif

		hItem = BUTTON_CreateAsChild(0, 0, 91, 101, pMsg->hWin, ID_BACKLOG_BUTTON, WM_CF_SHOW);
		BUTTON_SetTextColor(hItem, 0, GUI_WHITE);
		BUTTON_SetFont(hItem, GUI_FONT_20_1);
#ifdef _USE_BITMAP_PICTURES
		BUTTON_SetBitmap(hItem, BUTTON_BI_UNPRESSED, &bmbackRelease);
		BUTTON_SetBitmap(hItem, BUTTON_BI_PRESSED, &bmbackPush);
#else
		BUTTON_SetText(hItem, "Back");
#endif

		hItem = PROGBAR_CreateAsChild(200, 400, 400, 40, pMsg->hWin, ID_LOGPROGBAR, WM_CF_SHOW);

	    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_1);
	    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
	    TEXT_SetFont(hItem, GUI_FONT_20_1);
	    TEXT_SetTextColor(hItem, GUI_WHITE);
	    TEXT_SetText(hItem, fileInfoInt.name);

#ifdef _USE_BITMAP_PICTURES
	    hItem = WM_GetDialogItem(pMsg->hWin, ID_RUNLOG_BG_IMAGE);
	    IMAGE_SetBitmap(hItem, &bmbackground);
#endif
	    break;

	case WM_PAINT:

		GUI_SetBkColor(GUI_WHITE);
		GUI_Clear();
		break;

	case WM_NOTIFY_PARENT:
		switch(WM_GetId(pMsg->hWinSrc)){
		case ID_PLAYLOG_BUTTON:
			switch(pMsg->Data.v){
			case WM_NOTIFICATION_RELEASED:
				queue = xQueueCreate(1,1);
				xTaskGenericCreate(sendCanLogThread, "sendCanLogThread", 4 * 1024, 0, osPriorityNormal - osPriorityIdle, NULL, NULL, NULL);
			}
			break;

		case ID_RESETLOG_BUTTON:
			switch(pMsg->Data.v){
			case WM_NOTIFICATION_RELEASED:
				//closeThread
				hItem = WM_GetDialogItem(pMsg->hWin, ID_LOGPROGBAR);
				PROGBAR_SetValue(hItem, 0);
				if(queue){
					xQueueSend(queue, &closeCmd, 0);
				}
				break;
			}
			break;

		case ID_BACKLOG_BUTTON:
			switch(pMsg->Data.v){
			case WM_NOTIFICATION_RELEASED:
				GUI_EndDialog(pMsg->hWin, 0);
				break;
			}
			break;
		}
		break;
	default:
		WM_DefaultProc(pMsg);
	}
}

void initRunTestWindow(FileInfo fileInfo){
	fileInfoInt = fileInfo;

	window = GUI_CreateDialogBox(_runTestWindow, GUI_COUNTOF(_runTestWindow), windowCallback, WM_HBKWIN, 0, 0);
	CleanDisplay(LCD_FB_START_ADDRESS);
}
