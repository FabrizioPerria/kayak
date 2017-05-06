#include "panelsScreen.h"

#define ID_WINDOW_0  (GUI_ID_USER + 0x00)
#define ID_MULTIPAGE_0  (GUI_ID_USER + 0x01)
#define ID_WINDOW_MAIN  (GUI_ID_USER + 0x0C)
#define ID_SLIDER_0  (GUI_ID_USER + 0x02)
#define ID_SLIDER_1  (GUI_ID_USER + 0x03)
#define ID_TEXT_0  (GUI_ID_USER + 0x04)
#define ID_TEXT_1  (GUI_ID_USER + 0x05)
#define ID_RADIO_0  (GUI_ID_USER + 0x06)
#define ID_BUTTON_0  (GUI_ID_USER + 0x07)
#define ID_BUTTON_1  (GUI_ID_USER + 0x08)
#define ID_BUTTON_2  (GUI_ID_USER + 0x09)
#define ID_BUTTON_3  (GUI_ID_USER + 0x0A)
#define ID_BUTTON_4  (GUI_ID_USER + 0x0B)
#define ID_EXIT_BUTTON (GUI_ID_USER + 0x0C)

// USER START (Optionally insert additional defines)
// USER END

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static WM_HWIN hWin;
// USER START (Optionally insert additional static data)

//extern CAN_HandleTypeDef hcan1;
// USER END

/*********************************************************************
*
*       _aDialogCreate
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
	{ WINDOW_CreateIndirect, "Window", ID_WINDOW_0, 0, 0, 800, 480, 0, 0x0, 0 },
	{ MULTIPAGE_CreateIndirect, "Multipage", ID_MULTIPAGE_0, 0, 0, 799, 479, 0, 0x0, 0 },
};

static const GUI_WIDGET_CREATE_INFO _aDialogMainCreate[] = {
	{ WINDOW_CreateIndirect, "Window", ID_WINDOW_MAIN, 0, 30, 800, 450, 0, 0x0, 0 },
	{ SLIDER_CreateIndirect, "Slider", ID_SLIDER_0, 64, 70, 353, 50, 0, 0x0, 0 },
	{ SLIDER_CreateIndirect, "Slider", ID_SLIDER_1, 64, 180, 353, 50, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "Speed", ID_TEXT_0, 12, 77, 80, 20, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "RPM", ID_TEXT_1, 18, 186, 80, 20, 0, 0x0, 0 },
	{ RADIO_CreateIndirect, "Radio", ID_RADIO_0, 34, 270, 80, 153, 0, 0x2804, 0 },
	{ BUTTON_CreateIndirect, "Up", ID_BUTTON_0, 550, 62, 80, 80, 0, 0x0, 0 },
	{ BUTTON_CreateIndirect, "Right", ID_BUTTON_1, 652, 160, 80, 80, 0, 0x0, 0 },
	{ BUTTON_CreateIndirect, "Left", ID_BUTTON_2, 449, 160, 80, 80, 0, 0x0, 0 },
	{ BUTTON_CreateIndirect, "Down", ID_BUTTON_3, 550, 262, 80, 80, 0, 0x0, 0 },
	{ BUTTON_CreateIndirect, "OK", ID_BUTTON_4, 549, 163, 80, 80, 0, 0x0, 0 },
	{ BUTTON_CreateIndirect, "Quit", ID_EXIT_BUTTON, 710, 350, 80, 80, 0, 0x0, 0 },

};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

// USER START (Optionally insert additional static code)
//static void sendCANMessage(void)
//{
//	CanTxMsgTypeDef msg;
//	msg.Data[0] = 0;
//	msg.Data[1] = 0x11;
//	msg.Data[2] = 0x22;
//	msg.Data[3] = 0x33;
//	msg.Data[4] = 0x44;
//	msg.Data[5] = 0x55;
//	msg.Data[6] = 0x66;
//	msg.Data[7] = 0x77;
//	msg.DLC  =8;
//	msg.StdId = 11;
//	msg.IDE = CAN_ID_STD;
//
//	hcan1.pTxMsg = &msg;
//	HAL_CAN_Transmit(&hcan1, 10000);
//}

static void mainCallback(WM_MESSAGE *pMsg)
{
	WM_HWIN hItem;
	int     NCode;
	int     Id;

	switch (pMsg->MsgId) {
	case WM_INIT_DIALOG:
		hItem = WM_GetDialogItem(pMsg->hWin, ID_RADIO_0);
		RADIO_SetText(hItem, "Lock", 0);
		RADIO_SetText(hItem, "Off/ACC", 1);
		RADIO_SetText(hItem, "Run", 2);
		RADIO_SetText(hItem, "Start", 3);
		break;
	case WM_NOTIFY_PARENT:
		Id    = WM_GetId(pMsg->hWinSrc);
		NCode = pMsg->Data.v;
		switch (Id) {
		case ID_SLIDER_0: // Notifications sent by 'Slider'
			switch (NCode) {
			case WM_NOTIFICATION_CLICKED:
				break;
			case WM_NOTIFICATION_RELEASED:
				break;
			case WM_NOTIFICATION_VALUE_CHANGED:
				break;
			}
			break;
		case ID_SLIDER_1: // Notifications sent by 'Slider'
			switch (NCode) {
			case WM_NOTIFICATION_CLICKED:
				break;
			case WM_NOTIFICATION_RELEASED:
				break;
			case WM_NOTIFICATION_VALUE_CHANGED:
				break;
			}
			break;
		case ID_RADIO_0: // Notifications sent by 'Radio'
			switch (NCode) {
			case WM_NOTIFICATION_CLICKED:
				break;
			case WM_NOTIFICATION_RELEASED:
				break;
			case WM_NOTIFICATION_VALUE_CHANGED:
				break;
			}
			break;
		case ID_BUTTON_0: // Notifications sent by 'Up'
			switch (NCode) {
			case WM_NOTIFICATION_CLICKED:
				break;
			case WM_NOTIFICATION_RELEASED:
				break;
			}
			break;
		case ID_BUTTON_1: // Notifications sent by 'Right'
			switch (NCode) {
			case WM_NOTIFICATION_CLICKED:
				break;
			case WM_NOTIFICATION_RELEASED:
				break;
			}
			break;
		case ID_BUTTON_2: // Notifications sent by 'Left'
			switch (NCode) {
			case WM_NOTIFICATION_CLICKED:
				break;
			case WM_NOTIFICATION_RELEASED:
				break;
			}
			break;
		case ID_BUTTON_3: // Notifications sent by 'Down'
			switch (NCode) {
			case WM_NOTIFICATION_CLICKED:
				break;
			case WM_NOTIFICATION_RELEASED:
				break;
			}
			break;
		case ID_BUTTON_4: // Notifications sent by 'OK'
			switch (NCode) {
			case WM_NOTIFICATION_CLICKED:
				break;
			case WM_NOTIFICATION_RELEASED:
				break;
			}
			break;
		case ID_EXIT_BUTTON: // Notifications sent by 'OK'
			switch (NCode) {
			case WM_NOTIFICATION_CLICKED:
				break;
			case WM_NOTIFICATION_RELEASED:
				GUI_EndDialog(hWin,0);
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

// USER END

/*********************************************************************
*
*       _cbDialog
*/
static void _cbDialog(WM_MESSAGE * pMsg) {
	WM_HWIN hItem;
	int     NCode;
	int     Id;

	switch (pMsg->MsgId) {
	case WM_INIT_DIALOG:
		hItem = WM_GetDialogItem(pMsg->hWin, ID_MULTIPAGE_0);
		// Add pages HERE
		WM_HWIN hDialog = GUI_CreateDialogBox(_aDialogMainCreate, GUI_COUNTOF(_aDialogMainCreate), mainCallback, WM_UNATTACHED, 0, 0);
		MULTIPAGE_AddPage(hItem, hDialog, "Main");

		MULTIPAGE_AddEmptyPage(hItem, 0, "Second Panel");
		//

		MULTIPAGE_SelectPage(hItem, 0);
		break;
	case WM_NOTIFY_PARENT:
		Id    = WM_GetId(pMsg->hWinSrc);
		NCode = pMsg->Data.v;
		switch (Id) {
		case ID_MULTIPAGE_0: // Notifications sent by 'Multipage'
			switch (NCode) {
			case WM_NOTIFICATION_CLICKED:
				break;
			case WM_NOTIFICATION_RELEASED:
				break;
			case WM_NOTIFICATION_MOVED_OUT:
				break;
			case WM_NOTIFICATION_VALUE_CHANGED:
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

WM_HWIN openPanels(void)
{
	hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
	return hWin;
}
