#include "DIALOG.h"

#define ID_FRAMEWIN_MB  (GUI_ID_USER + 0x01)
#define ID_BUTTON_OK  (GUI_ID_USER + 0x02)
#define ID_TEXT_MESSAGE  (GUI_ID_USER + 0x03)

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
	{ FRAMEWIN_CreateIndirect, "Framewin", ID_FRAMEWIN_MB, 234, 160, 328, 159, 0, 0x64, 0 },
	{ BUTTON_CreateIndirect, "OK", ID_BUTTON_OK, 92, 60, 125, 45, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "Text", ID_TEXT_MESSAGE, 23, 25, 268, 20, 0, 0x0, 0 },
};

static const char* messageInt;

static void _cbDialog(WM_MESSAGE * pMsg)
{
	WM_HWIN hItem;
	int     NCode;
	int     Id;

	switch (pMsg->MsgId) {
	case WM_INIT_DIALOG:
		hItem = pMsg->hWin;
		FRAMEWIN_SetText(hItem, "WARNING");
		FRAMEWIN_SetFont(hItem, GUI_FONT_24_ASCII);
		FRAMEWIN_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);

		hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_MESSAGE);
		TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
		TEXT_SetFont(hItem, GUI_FONT_16B_1);
		TEXT_SetText(hItem, messageInt);
		break;
	case WM_NOTIFY_PARENT:
		Id    = WM_GetId(pMsg->hWinSrc);
		NCode = pMsg->Data.v;
		switch (Id) {
		case ID_BUTTON_OK: // Notifications sent by 'OK'
			switch(NCode) {
			case WM_NOTIFICATION_CLICKED:
				break;
			case WM_NOTIFICATION_RELEASED:
				messageInt = NULL;
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

WM_HWIN CreateMessageBox(const char* message)
{
	messageInt = message;
	return GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
}
