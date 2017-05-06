#include "DIALOG.h"
#include "LISTVIEW.h"
#include "storage.h"

#ifdef _USE_BITMAP_PICTURES

extern GUI_CONST_STORAGE GUI_BITMAP bmbackRelease;
extern GUI_CONST_STORAGE GUI_BITMAP bmbackPush;
extern GUI_CONST_STORAGE GUI_BITMAP bmselectRelease;
extern GUI_CONST_STORAGE GUI_BITMAP bmselectPush;
extern GUI_CONST_STORAGE GUI_BITMAP bmupRelease;
extern GUI_CONST_STORAGE GUI_BITMAP bmupPush;
extern GUI_CONST_STORAGE GUI_BITMAP bmdownRelease;
extern GUI_CONST_STORAGE GUI_BITMAP bmdownPush;
extern GUI_CONST_STORAGE GUI_BITMAP bmlogo;

#endif

#define ID_OPENFILE_WINDOW  	(GUI_ID_USER + 0x00)
#define ID_FILELIST_LISTVIEW  	(GUI_ID_USER + 0x01)
#define ID_DRIVES_DROPDOWN  	(GUI_ID_USER + 0x02)
#define ID_SELECTFILE_BUTTON  	(GUI_ID_USER + 0x03)
#define ID_DOWNDIRECTORY_BUTTON	(GUI_ID_USER + 0x04)
#define ID_UPDIRECTORY_BUTTON  	(GUI_ID_USER + 0x05)
#define ID_BACK_BUTTON  		(GUI_ID_USER + 0x06)
#define ID_TEXT_CMD 	 		(GUI_ID_USER + 0x07)
#define ID_TEXT_PATH 	 		(GUI_ID_USER + 0x08)
#define ID_LOGO_KAYAK 	 		(GUI_ID_USER + 0x09)

static FileInfo* fileInfoInt;
static int currentFileSelection;
static int currentDriveSelection;

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
	{ WINDOW_CreateIndirect, "Explorer", ID_OPENFILE_WINDOW, 0, 0, 800, 480, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "Select the Log file to run", ID_TEXT_CMD, 250, 0, 500, 40, 0, 0x0, 0 },
	{ LISTVIEW_CreateIndirect, "Listview", ID_FILELIST_LISTVIEW, 0, 100, 800, 300, 0, 0x0, 0 },
	{ DROPDOWN_CreateIndirect, "Dropdown", ID_DRIVES_DROPDOWN, 250, 50, 300, 180, 0, 0x0, 0 },
	{ TEXT_CreateIndirect, "Path", ID_TEXT_PATH, 120, 420, 500, 40, 0, 0x0, 0 },
};

static void loadFileList(WM_HWIN *hItem, const char* path)
{
	int index = 0;
	FileInfo fileInfo;

	LISTVIEW_DeleteAllRows(*hItem);
	memset(&fileInfo, 0, sizeof(FileInfo));
	strcpy(fileInfo.root, path);
	fileInfo.Cmd = CHOOSEFILE_FINDFIRST;
	while (StorageGetFileInfo(&fileInfo) != FR_NO_FILE) {
		const GUI_ConstString fields[4] = {fileInfo.name, fileInfo.ext, fileInfo.size, fileInfo.attrib};
		LISTVIEW_InsertRow(*hItem, index++, fields);
		memset(&fileInfo, 0, sizeof(FileInfo));
		strcpy(fileInfo.root, path);
		fileInfo.Cmd = CHOOSEFILE_FINDNEXT;
	}
}

static void _cbDialog(WM_MESSAGE * pMsg)
{
	WM_HWIN hItem;
	WM_HWIN hListView;

	switch (pMsg->MsgId) {
	case WM_INIT_DIALOG:
#ifdef _USE_BITMAP_PICTURES
		hItem = IMAGE_CreateEx(679, 0, 121, 91, pMsg->hWin, WM_CF_SHOW, 0, ID_LOGO_KAYAK);
		IMAGE_SetBitmap(hItem, &bmlogo);
#endif
		hItem = BUTTON_CreateAsChild(720, 400, 80, 80, pMsg->hWin, ID_SELECTFILE_BUTTON, WM_CF_SHOW);
		BUTTON_SetTextColor(hItem, 0, GUI_WHITE);
		BUTTON_SetFont(hItem, GUI_FONT_20_1);
#ifdef _USE_BITMAP_PICTURES
		BUTTON_SetBitmap(hItem, BUTTON_BI_UNPRESSED, &bmselectRelease);
		BUTTON_SetBitmap(hItem, BUTTON_BI_PRESSED, &bmselectPush);
#else
		BUTTON_SetText(hItem, "Select");
#endif
		hItem = BUTTON_CreateAsChild(620, 400, 80, 80, pMsg->hWin, ID_DOWNDIRECTORY_BUTTON, WM_CF_SHOW);
		BUTTON_SetTextColor(hItem, 0, GUI_WHITE);
		BUTTON_SetFont(hItem, GUI_FONT_20_1);
#ifdef _USE_BITMAP_PICTURES
		BUTTON_SetBitmap(hItem, BUTTON_BI_UNPRESSED, &bmdownRelease);
		BUTTON_SetBitmap(hItem, BUTTON_BI_PRESSED, &bmdownPush);
#else
		BUTTON_SetText(hItem, "Go");
#endif
		WM_HideWindow(hItem);
		hItem = BUTTON_CreateAsChild(0, 400, 80, 80, pMsg->hWin, ID_UPDIRECTORY_BUTTON, WM_CF_SHOW);
		BUTTON_SetTextColor(hItem, 0, GUI_WHITE);
		BUTTON_SetFont(hItem, GUI_FONT_20_1);
#ifdef _USE_BITMAP_PICTURES
		BUTTON_SetBitmap(hItem, BUTTON_BI_UNPRESSED, &bmupRelease);
		BUTTON_SetBitmap(hItem, BUTTON_BI_PRESSED, &bmupPush);
#else
		BUTTON_SetText(hItem, "Up");
#endif
		hItem = BUTTON_CreateAsChild(0, 0, 91, 100, pMsg->hWin, ID_BACK_BUTTON, WM_CF_SHOW);
		BUTTON_SetTextColor(hItem, 0, GUI_WHITE);
		BUTTON_SetFont(hItem, GUI_FONT_20_1);
#ifdef _USE_BITMAP_PICTURES
		BUTTON_SetBitmap(hItem, BUTTON_BI_UNPRESSED, &bmbackRelease);
		BUTTON_SetBitmap(hItem, BUTTON_BI_PRESSED, &bmbackPush);
#else
		BUTTON_SetText(hItem, "Back");
#endif
		hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_CMD);
		TEXT_SetFont(hItem, GUI_FONT_32_ASCII);

		hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_PATH);
		TEXT_SetFont(hItem, GUI_FONT_13_ASCII);

		hItem = WM_GetDialogItem(pMsg->hWin, ID_DRIVES_DROPDOWN);
		DROPDOWN_SetAutoScroll(hItem, 1);
		DROPDOWN_SetItemSpacing(hItem, 25);
		DROPDOWN_AddString(hItem, "USB Drive");
		DROPDOWN_AddString(hItem, "SD card");

		hListView = WM_GetDialogItem(pMsg->hWin, ID_FILELIST_LISTVIEW);
		LISTVIEW_AddColumn(hListView, 450, "File Name", GUI_TA_LEFT | GUI_TA_VCENTER);
		LISTVIEW_AddColumn(hListView, 100, "Type", GUI_TA_HCENTER | GUI_TA_VCENTER);
		LISTVIEW_AddColumn(hListView, 100, "Size", GUI_TA_HCENTER | GUI_TA_VCENTER);
		LISTVIEW_AddColumn(hListView, 110, "Attributes", GUI_TA_HCENTER | GUI_TA_VCENTER);
		LISTVIEW_SetGridVis(hListView, 1);
		LISTVIEW_SetAutoScrollV(hListView, 1);
		LISTVIEW_SetRowHeight(hListView, 50);
		LISTVIEW_SetHeaderHeight(hListView, 28);

		if (StorageGetStatus(USB_DISK_UNIT)) {
			loadFileList(&hListView, StorageGetPath(USB_DISK_UNIT));
			DROPDOWN_SetSel(hItem, USB_DISK_UNIT);
			hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_PATH);
			TEXT_SetText(hItem, StorageGetPath(USB_DISK_UNIT));
			currentDriveSelection = 0;
		} else if (StorageGetStatus(SDCARD_DISK_UNIT)) {
			loadFileList(&hListView, StorageGetPath(SDCARD_DISK_UNIT));
			DROPDOWN_SetSel(hItem, SDCARD_DISK_UNIT);
			hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_PATH);
			TEXT_SetText(hItem, StorageGetPath(SDCARD_DISK_UNIT));
			currentDriveSelection = 1;
		} else {
			hItem = CreateMessageBox("No Drive Found!");
			WaitForDialog(hItem);
			GUI_EndDialog(pMsg->hWin,0);
		}

		break;
	case WM_NOTIFY_PARENT:
		switch (WM_GetId(pMsg->hWinSrc)) {
		case ID_FILELIST_LISTVIEW:
			switch (pMsg->Data.v) {
			case WM_NOTIFICATION_SEL_CHANGED:
				currentFileSelection = LISTVIEW_GetSel(pMsg->hWinSrc);
				char isFile[FILE_EXT_SIZE];
				LISTVIEW_GetItemText(pMsg->hWinSrc, 1, currentFileSelection, isFile, FILE_EXT_SIZE);
				hItem = WM_GetDialogItem(pMsg->hWin, ID_DOWNDIRECTORY_BUTTON);
				if (!isFile[0]) {
					WM_ShowWindow(hItem);
				} else {
					WM_HideWindow(hItem);
				}
				break;
			}
			break;
		case ID_DRIVES_DROPDOWN:
			switch (pMsg->Data.v) {
			case WM_NOTIFICATION_CLICKED:
			case WM_NOTIFICATION_SEL_CHANGED:
				hListView = WM_GetDialogItem(pMsg->hWin, ID_FILELIST_LISTVIEW);
				hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_PATH);
				int sel = DROPDOWN_GetSel(pMsg->hWinSrc);
				if (StorageGetStatus(sel)) {
					char path[PATH_MAX_SIZE];
					memset(path, 0, PATH_MAX_SIZE);
					if (currentDriveSelection == sel) {
						TEXT_GetText(hItem, path, PATH_MAX_SIZE);
					} else {
						memcpy(path, StorageGetPath(sel), PATH_MAX_SIZE);
						currentDriveSelection = sel;
					}
					loadFileList(&hListView, path);
					TEXT_SetText(hItem, path);
				} else {
					currentDriveSelection = sel;
					LISTVIEW_DeleteAllRows(hListView);
					TEXT_SetText(hItem, "Drive Not Detected");
				}

				hItem = WM_GetDialogItem(pMsg->hWin, ID_DOWNDIRECTORY_BUTTON);
				WM_HideWindow(hItem);
				break;
			}
			break;
		case ID_SELECTFILE_BUTTON:
			switch (pMsg->Data.v) {
			case WM_NOTIFICATION_CLICKED:
//#ifdef _USE_BITMAP_PICTURES
//				hListView = WM_GetDialogItem(pMsg->hWin, ID_FILELIST_LISTVIEW);
//				char isFile[FILE_EXT_SIZE];
//				LISTVIEW_GetItemText(hListView, 1, currentFileSelection, isFile, FILE_EXT_SIZE);
//				if (!isFile[0]) {
//					BUTTON_SetBitmap(pMsg->hWinSrc, BUTTON_BI_UNPRESSED, &bmdownRelease);
//					BUTTON_SetBitmap(pMsg->hWinSrc, BUTTON_BI_PRESSED, &bmdownPush);
//				} else {
//					BUTTON_SetBitmap(pMsg->hWinSrc, BUTTON_BI_UNPRESSED, &bmselectRelease);
//					BUTTON_SetBitmap(pMsg->hWinSrc, BUTTON_BI_PRESSED, &bmselectPush);
//				}
//#endif
				break;
			case WM_NOTIFICATION_RELEASED:
				if (currentFileSelection >= 0) {
					hListView = WM_GetDialogItem(pMsg->hWin, ID_FILELIST_LISTVIEW);
					LISTVIEW_GetItemText(hListView, 0, currentFileSelection, fileInfoInt->name, FILE_NAME_SIZE);
					LISTVIEW_GetItemText(hListView, 1, currentFileSelection, fileInfoInt->ext, FILE_EXT_SIZE);
					char currentPath[PATH_MAX_SIZE];
					memset(currentPath, 0, PATH_MAX_SIZE);
					hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_PATH);
					TEXT_GetText(hItem, currentPath, PATH_MAX_SIZE);
					if (fileInfoInt->ext[0] == '\0') {
						LISTVIEW_DeleteAllRows(hListView);
						sprintf(currentPath, "%s%s/", currentPath, fileInfoInt->name);
						loadFileList(&hListView, currentPath);
						TEXT_SetText(hItem, currentPath);
						currentFileSelection = -1;
					} else {
						LISTVIEW_GetItemText(hListView, 2, currentFileSelection, fileInfoInt->size, FILE_NAME_SIZE);
						LISTVIEW_GetItemText(hListView, 3, currentFileSelection, fileInfoInt->attrib, FILE_EXT_SIZE);
						strcpy(fileInfoInt->root, currentPath);
						initRunTestWindow(*fileInfoInt);
					}
				} else {
					GUI_EndDialog(pMsg->hWin, 0);
				}
				break;
			}
			break;
		case ID_UPDIRECTORY_BUTTON:
			switch (pMsg->Data.v) {
			case WM_NOTIFICATION_RELEASED:
				hListView = WM_GetDialogItem(pMsg->hWin, ID_FILELIST_LISTVIEW);
				char currentPath[PATH_MAX_SIZE];
				memset(currentPath, 0, PATH_MAX_SIZE);
				hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_PATH);
				TEXT_GetText(hItem, currentPath, PATH_MAX_SIZE);;
				LISTVIEW_DeleteAllRows(hListView);
				for (int i = strlen(currentPath) - 2; i >= 0; --i) {
					if (currentPath[i] == '/')
						break;
					currentPath[i] = 0;
				}

				loadFileList(&hListView, currentPath);
				TEXT_SetText(hItem, currentPath);
				break;
			}
			break;
		case ID_BACK_BUTTON:
			switch (pMsg->Data.v) {
			case WM_NOTIFICATION_RELEASED:
				currentDriveSelection = -1;
				GUI_EndDialog(pMsg->hWin, 0);
				break;
			}
		}
		break;
	default:
		WM_DefaultProc(pMsg);
		break;
	}
}

void CreateExplorer(void)
{
	currentFileSelection = -1;
	GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
}
