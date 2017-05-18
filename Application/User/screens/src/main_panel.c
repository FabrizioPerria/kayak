#include "main_panel.h"
#include "panelsScreen.h"
#include "bspManager.h"

const char cmdContinue = CONTINUE;
const char cmdStopPeriodic = QUIT_40S_SLEEP;
const char cmdStopCANSend = QUIT_PERIODIC_CAN;

void mainCallback(WM_MESSAGE *pMsg)
{
	WM_HWIN hItem;
	int     NCode;
	int     Id;

	switch (pMsg->MsgId) {
	case WM_INIT_DIALOG:
		hItem = WM_GetDialogItem(pMsg->hWin, ID_RADIO_STATUS);
		RADIO_SetText(hItem, "Lock", 0);
		RADIO_SetText(hItem, "Off/ACC", 1);
		RADIO_SetText(hItem, "Run", 2);
		RADIO_SetText(hItem, "Start", 3);
		RADIO_SetValue(hItem, 2);
		hItem = WM_GetDialogItem(pMsg->hWin, ID_RADIO_PERIODIC);
		RADIO_SetText(hItem, "Periodic Enabled", 0);
		RADIO_SetText(hItem, "Periodic Disabled", 1);
		hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_SPEED);
		SLIDER_SetRange(hItem, 0, 512);
		hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_RPM);
		SLIDER_SetRange(hItem, 0, 65534/4);
		break;

	case WM_NOTIFY_PARENT:
		Id    = WM_GetId(pMsg->hWinSrc);
		NCode = pMsg->Data.v;
		switch (Id) {
		case ID_SLIDER_SPEED: // Notifications sent by 'Slider'
			switch(NCode) {
			case WM_NOTIFICATION_VALUE_CHANGED:
				hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_SPEED);
				int speed = SLIDER_GetValue(hItem) * 128;
				periodicData[CAN_ESP_A8][4] = (uint8_t) ((speed >> 8) & 0xFF);
				periodicData[CAN_ESP_A8][5] = (uint8_t) (speed & 0xFF);
				break;
			}
			break;
		case ID_SLIDER_RPM: // Notifications sent by 'Slider'
			switch(NCode) {
			case WM_NOTIFICATION_VALUE_CHANGED:
				hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_RPM);
				int rpm = SLIDER_GetValue(hItem);
				periodicData[CAN_ECM_A1][0] = (uint8_t) ((rpm >> 8) & 0xFF);
				periodicData[CAN_ECM_A1][1] = (uint8_t) (rpm & 0xFF);
				break;
			}
			break;
		case ID_RADIO_STATUS: // Notifications sent by 'Radio'
			switch (NCode) {
			case WM_NOTIFICATION_VALUE_CHANGED:
				hItem = WM_GetDialogItem(pMsg->hWin, ID_RADIO_STATUS);
				int val = RADIO_GetValue(hItem);
				switch (val) {
				case 1:
					periodicData[CAN_CBC_PT2][0] = 3;
					break;
				case 2:
					periodicData[CAN_CBC_PT2][0] = 4;
					break;
				case 3:
					periodicData[CAN_CBC_PT2][0] = 5;
					break;
				default:
					periodicData[CAN_CBC_PT2][0] = 0;
					break;
				}
				break;
			}
			break;
			case ID_RADIO_PERIODIC:
				switch (NCode) {
				case WM_NOTIFICATION_VALUE_CHANGED:
					hItem = WM_GetDialogItem(pMsg->hWin, ID_RADIO_PERIODIC);
					int val = RADIO_GetValue(hItem);
					switch (val) {
					case 0:
						xQueueSend(queueCAN, &cmdStopPeriodic, 0);
						break;
					case 1:
						xQueueSend(queueCAN, &cmdStopCANSend, 0);
						break;
					}
					break;
				}
				break;
		case ID_BUTTON_UP:
			switch (NCode) {
			case WM_NOTIFICATION_CLICKED:
				periodicData[CAN_SWS_8][5] = 0x04;
				break;
			case WM_NOTIFICATION_RELEASED:
				periodicData[CAN_SWS_8][5] = 0x00;
				break;
			}
			CAN_Send(periodicID[CAN_SWS_8], periodicLength[CAN_SWS_8], periodicData[CAN_SWS_8]);
			break;
		case ID_BUTTON_RIGHT:
			switch (NCode) {
			case WM_NOTIFICATION_CLICKED:
				periodicData[CAN_SWS_8][5] = 0x01;
				break;
			case WM_NOTIFICATION_RELEASED:
				periodicData[CAN_SWS_8][5] = 0x00;
				break;
			}
			CAN_Send(periodicID[CAN_SWS_8], periodicLength[CAN_SWS_8], periodicData[CAN_SWS_8]);
			break;
		case ID_BUTTON_LEFT:
			switch (NCode) {
			case WM_NOTIFICATION_CLICKED:
				periodicData[CAN_SWS_8][4] = 0x10;
				break;
			case WM_NOTIFICATION_RELEASED:
				periodicData[CAN_SWS_8][4] = 0x00;
				break;
			}
			CAN_Send(periodicID[CAN_SWS_8], periodicLength[CAN_SWS_8], periodicData[CAN_SWS_8]);
			break;
		case ID_BUTTON_DOWN:
			switch (NCode) {
			case WM_NOTIFICATION_CLICKED:
				periodicData[CAN_SWS_8][4] = 0x40;
				break;
			case WM_NOTIFICATION_RELEASED:
				periodicData[CAN_SWS_8][4] = 0x00;
				break;
			}
			CAN_Send(periodicID[CAN_SWS_8], periodicLength[CAN_SWS_8], periodicData[CAN_SWS_8]);
			break;
		case ID_BUTTON_OK:
			switch (NCode) {
			case WM_NOTIFICATION_CLICKED:
				periodicData[CAN_SWS_8][5] = 0x10;
				break;
			case WM_NOTIFICATION_RELEASED:
				periodicData[CAN_SWS_8][5] = 0x00;
				break;
			}
			CAN_Send(periodicID[CAN_SWS_8], periodicLength[CAN_SWS_8], periodicData[CAN_SWS_8]);
			break;
		break;
	default:
		WM_DefaultProc(pMsg);
		break;
	}
}
