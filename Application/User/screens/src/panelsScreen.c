#include "panelsScreen.h"
#include "SPIScreen.h"
#include "messages.h"
#include "bspManager.h"
#include "cmsis_os.h"
#include <string.h>
#include "main.h"

#define ID_WINDOW_MAIN_0  (GUI_ID_USER + 0x00)
#define ID_MULTIPAGE_0  (GUI_ID_USER + 0x01)
#define ID_WINDOW_MAIN  (GUI_ID_USER + 0x0C)
#define ID_SLIDER_0  (GUI_ID_USER + 0x02)
#define ID_SLIDER_1  (GUI_ID_USER + 0x03)
#define ID_TEXT_MAIN_0  (GUI_ID_USER + 0x04)
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

//static int choiceCBCPT2;

static int periodicID[NUM_CAN_MESSAGES];
static uint8_t* periodicData[NUM_CAN_MESSAGES];
static int periodicLength[NUM_CAN_MESSAGES];
WM_HWIN SPI_Dialog;
WM_HWIN Main_Dialog;
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

static const GUI_WIDGET_CREATE_INFO _aDialogSPICreate[] = {
	{ WINDOW_CreateIndirect, "Window", ID_WINDOW_0, 0, 0, 800, 480, 0, 0x0, 0 },
	{ MULTIEDIT_CreateIndirect, "Multiedit", ID_MULTIEDIT_0, 0, 0, 800, 375, 0, 0x0, 0 },
	//{ BUTTON_CreateIndirect, "Button", ID_BACK_BUTTON, 0, 0, 80, 80, 0, 0x0, 0 },
//	{ TEXT_CreateIndirect, "SPI data", ID_TEXT_0, 356, 37, 80, 20, 0, 0x0, 0 },
};
/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

static void mainCallback(WM_MESSAGE *pMsg)
{
	WM_HWIN hItem;
	int     NCode;
	int     Id;
	uint8_t buffer[8];
	static int x = 0;

	switch (pMsg->MsgId) {
	case 948:
		hItem = WM_GetDialogItem(pMsg->hWin, ID_RADIO_0);
		RADIO_SetValue(hItem, x++);
		x %= 4;
		break;
	case WM_INIT_DIALOG:
		hItem = WM_GetDialogItem(pMsg->hWin, ID_RADIO_0);
		RADIO_SetText(hItem, "Lock", 0);
		RADIO_SetText(hItem, "Off/ACC", 1);
		RADIO_SetText(hItem, "Run", 2);
		RADIO_SetText(hItem, "Start", 3);
		hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_0);
		SLIDER_SetRange(hItem, 0, 512);
		hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_1);
		SLIDER_SetRange(hItem, 0, 65534/4);
		break;

	case WM_NOTIFY_PARENT:
		Id    = WM_GetId(pMsg->hWinSrc);
		NCode = pMsg->Data.v;
		memset(buffer, 0, 8);
		switch (Id) {
		case ID_SLIDER_0: // Notifications sent by 'Slider'
			switch(NCode) {
			case WM_NOTIFICATION_VALUE_CHANGED:
				hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_0);
				int speed = SLIDER_GetValue(hItem) * 128;
				periodicData[CAN_ESP_A8][4] = (uint8_t) ((speed >> 8) & 0xFF);
				periodicData[CAN_ESP_A8][5] = (uint8_t) (speed & 0xFF);
				break;
			}
			break;
		case ID_SLIDER_1: // Notifications sent by 'Slider'
			switch(NCode) {
			case WM_NOTIFICATION_VALUE_CHANGED:
				hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_1);
				int rpm = SLIDER_GetValue(hItem);
				periodicData[CAN_ECM_A1][0] = (uint8_t) ((rpm >> 8) & 0xFF);
				periodicData[CAN_ECM_A1][1] = (uint8_t) (rpm & 0xFF);
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
				hItem = WM_GetDialogItem(pMsg->hWin, ID_RADIO_0);
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
		case ID_BUTTON_0: // Notifications sent by 'Up'
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
		case ID_BUTTON_1: // Notifications sent by 'Right'
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
		case ID_BUTTON_2: // Notifications sent by 'Left'
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
		case ID_BUTTON_3: // Notifications sent by 'Down'
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
		case ID_BUTTON_4: // Notifications sent by 'OK'
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

static void _cbDialog(WM_MESSAGE * pMsg) {
	WM_HWIN hItem;
	int     NCode;
	int     Id;

	switch (pMsg->MsgId) {
	case WM_INIT_DIALOG:
		hItem = WM_GetDialogItem(pMsg->hWin, ID_MULTIPAGE_0);
		SPI_Dialog = GUI_CreateDialogBox(_aDialogSPICreate, GUI_COUNTOF(_aDialogSPICreate), _cbSPIDialog, WM_HBKWIN, 0, 0);
		MULTIPAGE_AddPage(hItem, SPI_Dialog, "SPI");
		Main_Dialog = GUI_CreateDialogBox(_aDialogMainCreate, GUI_COUNTOF(_aDialogMainCreate), mainCallback, WM_HBKWIN, 0, 0);
		MULTIPAGE_AddPage(hItem, Main_Dialog, "Main");
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


WM_MESSAGE msg;

void periodic_CAN_Timer(void)
{
	uint8_t buffer[8];
	static uint32_t ui32_Millisecond_Count = 0;
	static char exit = 0;
	//CBCPT2
//	memset(buffer, 0, 8);
//	buffer[0] = (uint8_t) (periodicData[CBC_PT2] & 0xFF);
//	sendCANMessage(periodicID[CBC_PT2], periodicLength[CBC_PT2], buffer);
//	//SPEED
//	memset(buffer, 0, 8);
//	buffer[4] = (uint8_t) ((periodicData[SPEED] >> 8) & 0xFF);
//	buffer[5] = (uint8_t) (periodicData[SPEED] & 0xFF);
//	sendCANMessage(periodicID[SPEED], periodicLength[SPEED], buffer);
//	//RPM
//	memset(buffer, 0, 8);
//	buffer[0] = (uint8_t) ((periodicData[RPM] >> 8) & 0xFF);
//	buffer[1] = (uint8_t) (periodicData[RPM] & 0xFF);
//	sendCANMessage(periodicID[RPM], periodicLength[RPM], buffer);

//	WM_SendMessage(SPI_Dialog, &msg);
	while (1) {
		osDelay(1);
		ui32_Millisecond_Count++;
		if (ui32_Millisecond_Count % 40000 == 0)
			exit ^= 1;
		if(!exit) {
			if ((ui32_Millisecond_Count % 10) == 0) {

				CAN_Send(periodicID[CAN_ECM_A1], periodicLength[CAN_ECM_A1], periodicData[CAN_ECM_A1]);
				CAN_Send(periodicID[CAN_ECM_A3], periodicLength[CAN_ECM_A3], periodicData[CAN_ECM_A3]);
				CAN_Send(periodicID[CAN_SCCM_STW_ANGL_STAT], periodicLength[CAN_SCCM_STW_ANGL_STAT], periodicData[CAN_SCCM_STW_ANGL_STAT]);
			}

			if ((ui32_Millisecond_Count % (20 - 2)) == 0) {
	//			WM_SendMessage(SPI_Dialog, &msg);
				CAN_Send(periodicID[CAN_CBC_PT2], periodicLength[CAN_CBC_PT2], periodicData[CAN_CBC_PT2]);
				CAN_Send(periodicID[CAN_DTCM_A1], periodicLength[CAN_DTCM_A1], periodicData[CAN_DTCM_A1]);
				CAN_Send(periodicID[CAN_ECM_CRUISE_MAP], periodicLength[CAN_ECM_CRUISE_MAP], periodicData[CAN_ECM_CRUISE_MAP]);
				CAN_Send(periodicID[CAN_ESP_A1], periodicLength[CAN_ESP_A1], periodicData[CAN_ESP_A1]);
				CAN_Send(periodicID[CAN_ESP_A8], periodicLength[CAN_ESP_A8], periodicData[CAN_ESP_A8]);
				CAN_Send(periodicID[CAN_SBW_ROT1], periodicLength[CAN_SBW_ROT1], periodicData[CAN_SBW_ROT1]);
				CAN_Send(periodicID[CAN_TCM_A7], periodicLength[CAN_TCM_A7], periodicData[CAN_TCM_A7]);
				CAN_Send(periodicID[CAN_TRNS_SPD], periodicLength[CAN_TRNS_SPD], periodicData[CAN_TRNS_SPD]);
				CAN_Send(periodicID[CAN_TRNS_STAT], periodicLength[CAN_TRNS_STAT], periodicData[CAN_TRNS_STAT]);
			}    // end if for 20 msec send periodic

			if ((ui32_Millisecond_Count % (50 - 6)) == 0) {
				CAN_Send(periodicID[CAN_ESP_B1], periodicLength[CAN_ESP_B1], periodicData[CAN_ESP_B1]);
				CAN_Send(periodicID[CAN_CFG_RQ], periodicLength[CAN_CFG_RQ], periodicData[CAN_CFG_RQ]);
			}    // end if for 50 msec send periodic

			if ((ui32_Millisecond_Count % (100 - 12)) == 0) {
				CAN_Send(periodicID[CAN_CBC_PT3], periodicLength[CAN_CBC_PT3], periodicData[CAN_CBC_PT3]);
				CAN_Send(periodicID[CAN_CBC_PT9], periodicLength[CAN_CBC_PT9], periodicData[CAN_CBC_PT9]);
				CAN_Send(periodicID[CAN_ECM_B11], periodicLength[CAN_ECM_B11], periodicData[CAN_ECM_B11]);
				CAN_Send(periodicID[CAN_ECM_B2], periodicLength[CAN_ECM_B2], periodicData[CAN_ECM_B2]);
				CAN_Send(periodicID[CAN_ECM_B3], periodicLength[CAN_ECM_B3], periodicData[CAN_ECM_B3]);
				CAN_Send(periodicID[CAN_ECM_B9], periodicLength[CAN_ECM_B9], periodicData[CAN_ECM_B9]);
				CAN_Send(periodicID[CAN_ECM_DIESEL], periodicLength[CAN_ECM_DIESEL], periodicData[CAN_ECM_DIESEL]);
				CAN_Send(periodicID[CAN_ECM_INDICATORS], periodicLength[CAN_ECM_INDICATORS], periodicData[CAN_ECM_INDICATORS]);
				CAN_Send(periodicID[CAN_ENG_CFG], periodicLength[CAN_ENG_CFG], periodicData[CAN_ENG_CFG]);
				CAN_Send(periodicID[CAN_EPS_A1], periodicLength[CAN_EPS_A1], periodicData[CAN_EPS_A1]);
				CAN_Send(periodicID[CAN_HCP_C1], periodicLength[CAN_HCP_C1], periodicData[CAN_HCP_C1]);
				CAN_Send(periodicID[CAN_TRNS_STAT2], periodicLength[CAN_TRNS_STAT2], periodicData[CAN_TRNS_STAT2]);
				CAN_Send(periodicID[CAN_VIN], periodicLength[CAN_VIN], periodicData[CAN_VIN]);
			}    // end if for 100 msec send periodic

			if ((ui32_Millisecond_Count % (200 - 26)) == 0) {
				CAN_Send(periodicID[CAN_RFHUB_A2], periodicLength[CAN_RFHUB_A2], periodicData[CAN_RFHUB_A2]);
			}    // end if for 200 msec send periodic

			if ((ui32_Millisecond_Count % (500 - 60)) == 0) {
				CAN_Send(periodicID[CAN_BSM_A1], periodicLength[CAN_BSM_A1], periodicData[CAN_BSM_A1]);
				CAN_Send(periodicID[CAN_HCP_DISP], periodicLength[CAN_HCP_DISP], periodicData[CAN_HCP_DISP]);
				CAN_Send(periodicID[CAN_PN14_STAT], periodicLength[CAN_PN14_STAT], periodicData[CAN_PN14_STAT]);
				CAN_Send(periodicID[CAN_PTS_1], periodicLength[CAN_PTS_1], periodicData[CAN_PTS_1]);
				CAN_Send(periodicID[CAN_PTS_2], periodicLength[CAN_PTS_2], periodicData[CAN_PTS_2]);
				CAN_Send(periodicID[CAN_STATUS_C_PTS], periodicLength[CAN_STATUS_C_PTS], periodicData[CAN_STATUS_C_PTS]);
				CAN_Send(periodicID[CAN_VehCfg1], periodicLength[CAN_VehCfg1], periodicData[CAN_VehCfg1]);
			}    // end if for 500 msec send periodic

			if ((ui32_Millisecond_Count % (1000 - 120)) == 0) {
				CAN_Send(periodicID[CAN_AMB_TEMP_DISP], periodicLength[CAN_AMB_TEMP_DISP], periodicData[CAN_AMB_TEMP_DISP]);
				CAN_Send(periodicID[CAN_ASBS_1], periodicLength[CAN_ASBS_1], periodicData[CAN_ASBS_1]);
				CAN_Send(periodicID[CAN_CBC_PT1], periodicLength[CAN_CBC_PT1], periodicData[CAN_CBC_PT1]);
				CAN_Send(periodicID[CAN_CBC_PT10], periodicLength[CAN_CBC_PT10], periodicData[CAN_CBC_PT10]);
				CAN_Send(periodicID[CAN_CBC_PT4], periodicLength[CAN_CBC_PT4], periodicData[CAN_CBC_PT4]);
				CAN_Send(periodicID[CAN_CBC_PT8], periodicLength[CAN_CBC_PT8], periodicData[CAN_CBC_PT8]);
				CAN_Send(periodicID[CAN_Clock_Date], periodicLength[CAN_Clock_Date], periodicData[CAN_Clock_Date]);
				CAN_Send(periodicID[CAN_COMPASS_A1], periodicLength[CAN_COMPASS_A1], periodicData[CAN_COMPASS_A1]);
				CAN_Send(periodicID[CAN_DIRECT_INFO], periodicLength[CAN_DIRECT_INFO], periodicData[CAN_DIRECT_INFO]);
				CAN_Send(periodicID[CAN_DTCM_B1], periodicLength[CAN_DTCM_B1], periodicData[CAN_DTCM_B1]);
				CAN_Send(periodicID[CAN_ECM_B5], periodicLength[CAN_ECM_B5], periodicData[CAN_ECM_B5]);
				CAN_Send(periodicID[CAN_GW_I_C1], periodicLength[CAN_GW_I_C1], periodicData[CAN_GW_I_C1]);
				CAN_Send(periodicID[CAN_GW_LIN_I_C2], periodicLength[CAN_GW_LIN_I_C2], periodicData[CAN_GW_LIN_I_C2]);
				CAN_Send(periodicID[CAN_GW_LIN_I_C4], periodicLength[CAN_GW_LIN_I_C4], periodicData[CAN_GW_LIN_I_C4]);
				CAN_Send(periodicID[CAN_ORC_A1], periodicLength[CAN_ORC_A1], periodicData[CAN_ORC_A1]);
				CAN_Send(periodicID[CAN_ORC_A3], periodicLength[CAN_ORC_A3], periodicData[CAN_ORC_A3]);
				CAN_Send(periodicID[CAN_RFHUB_A3], periodicLength[CAN_RFHUB_A3], periodicData[CAN_RFHUB_A3]);
		//		CAN_Send(periodicID[CAN_SWS_8], periodicLength[CAN_SWS_8], periodicData[CAN_SWS_8]);
				CAN_Send(periodicID[CAN_TGW_DATA_IC], periodicLength[CAN_TGW_DATA_IC], periodicData[CAN_TGW_DATA_IC]);
				CAN_Send(periodicID[CAN_TPM_A1], periodicLength[CAN_TPM_A1], periodicData[CAN_TPM_A1]);
			}    // end if for 1000 msec send periodic

			if ((ui32_Millisecond_Count % (2000 - 240)) == 0) {
				CAN_Send(periodicID[CAN_EcuCfg10], periodicLength[CAN_EcuCfg10], periodicData[CAN_EcuCfg10]);
				CAN_Send(periodicID[CAN_EcuCfg12], periodicLength[CAN_EcuCfg12], periodicData[CAN_EcuCfg12]);
				CAN_Send(periodicID[CAN_EcuCfg4], periodicLength[CAN_EcuCfg4], periodicData[CAN_EcuCfg4]);
				CAN_Send(periodicID[CAN_NET_CFG_INT], periodicLength[CAN_NET_CFG_INT], periodicData[CAN_NET_CFG_INT]);
				CAN_Send(periodicID[CAN_NET_CFG_PT], periodicLength[CAN_NET_CFG_PT], periodicData[CAN_NET_CFG_PT]);
				CAN_Send(periodicID[CAN_VehCfg2], periodicLength[CAN_VehCfg2], periodicData[CAN_VehCfg2]);
				CAN_Send(periodicID[CAN_VehCfg3], periodicLength[CAN_VehCfg3], periodicData[CAN_VehCfg3]);
				CAN_Send(periodicID[CAN_VehCfg5], periodicLength[CAN_VehCfg5], periodicData[CAN_VehCfg5]);
				CAN_Send(periodicID[CAN_VehCfg6], periodicLength[CAN_VehCfg6], periodicData[CAN_VehCfg6]);
				CAN_Send(periodicID[CAN_VehCfg7], periodicLength[CAN_VehCfg7], periodicData[CAN_VehCfg7]);
				CAN_Send(periodicID[CAN_VehCfg8], periodicLength[CAN_VehCfg8], periodicData[CAN_VehCfg8]);
				CAN_Send(periodicID[CAN_VehCfgCSM1], periodicLength[CAN_VehCfgCSM1], periodicData[CAN_VehCfgCSM1]);
				CAN_Send(periodicID[CAN_VehCfgCSM2], periodicLength[CAN_VehCfgCSM2], periodicData[CAN_VehCfgCSM2]);

			}    // end if for 2000 msec send periodic
		}
	}
}

void initArrays(void)
{
//speed is CAN_ESP_A8
//	periodicID[SPEED] = 0x79;
//	periodicLength[SPEED] = 8;

	//RPM is CAN_CBC_PT8
//	periodicID[RPM] = 0x25;
//	periodicLength[RPM] = 8;

	//BUTTONS ARE CAN_SWS_8

//	periodicID[CBC_PT2] = 0x77;
//	periodicLength[CBC_PT2] = 2;

	msg.MsgId = WM_USER;

	periodicLength[CAN_AMB_TEMP_DISP] = 4;
	periodicID[CAN_AMB_TEMP_DISP] = 0x3C9;
	periodicData[CAN_AMB_TEMP_DISP] = m_abyte_AMB_TEMP_DISP_Msg;

	periodicLength[CAN_APPL_ECU_IC] = 8;
	periodicID[CAN_APPL_ECU_IC] = 0x6C2;
	periodicData[CAN_APPL_ECU_IC] = m_abyte_APPL_ECU_IC_Msg;

	periodicLength[CAN_ASBS_1] = 8;
	periodicID[CAN_ASBS_1] = 0x371;
	periodicData[CAN_ASBS_1] = m_abyte_ASBS_1_Msg;

	periodicLength[CAN_BSM_A1] = 6;
	periodicID[CAN_BSM_A1] = 0x2D0;
	periodicData[CAN_BSM_A1] = m_abyte_BSM_A1_Msg;

	periodicLength[CAN_CBC_PT1] = 8;
	periodicID[CAN_CBC_PT1] = 0x291;
	periodicData[CAN_CBC_PT1] = m_abyte_CBC_PT1_Msg;

	periodicLength[CAN_CBC_PT10] = 8;
	periodicID[CAN_CBC_PT10] = 0x265;
	periodicData[CAN_CBC_PT10] = m_abyte_CBC_PT10_Msg;

	periodicLength[CAN_CBC_PT2] = 2;
	periodicID[CAN_CBC_PT2] = 0x77;
	periodicData[CAN_CBC_PT2] = m_abyte_CBC_PT2_Msg;

	periodicLength[CAN_CBC_PT3] = 8;
	periodicID[CAN_CBC_PT3] = 0x12B;
	periodicData[CAN_CBC_PT3] = m_abyte_CBC_PT3_Msg;

	periodicLength[CAN_CBC_PT4] = 8;
	periodicID[CAN_CBC_PT4] = 0x283;
	periodicData[CAN_CBC_PT4] = m_abyte_CBC_PT4_Msg;

	periodicLength[CAN_CBC_PT8] = 8;
	periodicID[CAN_CBC_PT8] = 0x25F;
	periodicData[CAN_CBC_PT8] = m_abyte_CBC_PT8_Msg;

	periodicLength[CAN_CBC_PT9] = 8;
	periodicID[CAN_CBC_PT9] = 0x13F;
	periodicData[CAN_CBC_PT9] = m_abyte_CBC_PT9_Msg;

	periodicLength[CAN_CFG_RQ] = 3;
	periodicID[CAN_CFG_RQ] = 0x314;
	periodicData[CAN_CFG_RQ] = m_abyte_CFG_RQ_Msg;

	periodicLength[CAN_Clock_Date] = 8;
	periodicID[CAN_Clock_Date] = 0x350;
	periodicData[CAN_Clock_Date] = m_abyte_Clock_Date_Msg;

	periodicLength[CAN_COMPASS_A1] = 4;
	periodicID[CAN_COMPASS_A1] = 0x358;
	periodicData[CAN_COMPASS_A1] = m_abyte_COMPASS_A1_Msg;

	periodicLength[CAN_D_RQ_IC] = 8;
	periodicID[CAN_D_RQ_IC] = 0x742;
	periodicData[CAN_D_RQ_IC] = m_abyte_D_RQ_IC_Msg;

	periodicLength[CAN_DG_RQ_GLOBAL_UDS] = 8;
	periodicID[CAN_DG_RQ_GLOBAL_UDS] = 0x441;
	periodicData[CAN_DG_RQ_GLOBAL_UDS] = m_abyte_DG_RQ_GLOBAL_UDS_Msg;

	periodicLength[CAN_DIRECT_INFO] = 6;
	periodicID[CAN_DIRECT_INFO] = 0x35C;
	periodicData[CAN_DIRECT_INFO] = m_abyte_DIRECT_INFO_Msg;

	periodicLength[CAN_DTCM_A1] = 8;
	periodicID[CAN_DTCM_A1] = 0x8D;
	periodicData[CAN_DTCM_A1] = m_abyte_DTCM_A1_Msg;

	periodicLength[CAN_DTCM_B1] = 8;
	periodicID[CAN_DTCM_B1] = 0x277;
	periodicData[CAN_DTCM_B1] = m_abyte_DTCM_B1_Msg;

	periodicLength[CAN_ECM_A1] = 8;
	periodicID[CAN_ECM_A1] = 0x25;
	periodicData[CAN_ECM_A1] = m_abyte_ECM_A1_Msg;

	periodicLength[CAN_ECM_A3] = 8;
	periodicID[CAN_ECM_A3] = 0x35;
	periodicData[CAN_ECM_A3] = m_abyte_ECM_A3_Msg;

	periodicLength[CAN_ECM_B11] = 8;
	periodicID[CAN_ECM_B11] = 0x2A4;
	periodicData[CAN_ECM_B11] = m_abyte_ECM_B11_Msg;

	periodicLength[CAN_ECM_B2] = 8;
	periodicID[CAN_ECM_B2] = 0x137;
	periodicData[CAN_ECM_B2] = m_abyte_ECM_B2_Msg;

	periodicLength[CAN_ECM_B3] = 8;
	periodicID[CAN_ECM_B3] = 0x127;
	periodicData[CAN_ECM_B3] = m_abyte_ECM_B3_Msg;

	periodicLength[CAN_ECM_B5] = 8;
	periodicID[CAN_ECM_B5] = 0x279;
	periodicData[CAN_ECM_B5] = m_abyte_ECM_B5_Msg;

	periodicLength[CAN_ECM_B9] = 8;
	periodicID[CAN_ECM_B9] = 0x157;
	periodicData[CAN_ECM_B9] = m_abyte_ECM_B9_Msg;

	periodicLength[CAN_ECM_CRUISE_MAP] = 8;
	periodicID[CAN_ECM_CRUISE_MAP] = 0x81;
	periodicData[CAN_ECM_CRUISE_MAP] = m_abyte_ECM_CRUISE_MAP_Msg;

	periodicLength[CAN_ECM_DIESEL] = 8;
	periodicID[CAN_ECM_DIESEL] = 0x12D;
	periodicData[CAN_ECM_DIESEL] = m_abyte_ECM_DIESEL_Msg;

	periodicLength[CAN_ECM_INDICATORS] = 8;
	periodicID[CAN_ECM_INDICATORS] = 0x13D;
	periodicData[CAN_ECM_INDICATORS] = m_abyte_ECM_INDICATORS_Msg;

	periodicLength[CAN_EcuCfg10] = 8;
	periodicID[CAN_EcuCfg10] = 0x3D7;
	periodicData[CAN_EcuCfg10] = m_abyte_EcuCfg10_Msg;

	periodicLength[CAN_EcuCfg12] = 8;
	periodicID[CAN_EcuCfg12] = 0x3D5;
	periodicData[CAN_EcuCfg12] = m_abyte_EcuCfg12_Msg;

	periodicLength[CAN_EcuCfg4] = 8;
	periodicID[CAN_EcuCfg4] = 0x3F3;
	periodicData[CAN_EcuCfg4] = m_abyte_EcuCfg4_Msg;

	periodicLength[CAN_ENG_CFG] = 7;
	periodicID[CAN_ENG_CFG] = 0x3E1;
	periodicData[CAN_ENG_CFG] = m_abyte_ENG_CFG_Msg;

	periodicLength[CAN_EPS_A1] = 8;
	periodicID[CAN_EPS_A1] = 0x128;
	periodicData[CAN_EPS_A1] = m_abyte_EPS_A1_Msg;

	periodicLength[CAN_ESP_A1] = 8;
	periodicID[CAN_ESP_A1] = 0x83;
	periodicData[CAN_ESP_A1] = m_abyte_ESP_A1_Msg;

	periodicLength[CAN_ESP_A8] = 8;
	periodicID[CAN_ESP_A8] = 0x79;
	periodicData[CAN_ESP_A8] = m_abyte_ESP_A8_Msg;

	periodicLength[CAN_ESP_B1] = 8;
	periodicID[CAN_ESP_B1] = 0xDD;
	periodicData[CAN_ESP_B1] = m_abyte_ESP_B1_Msg;

	periodicLength[CAN_GW_I_C1] = 8;
	periodicID[CAN_GW_I_C1] = 0x289;
	periodicData[CAN_GW_I_C1] = m_abyte_GW_I_C1_Msg;

	periodicLength[CAN_GW_LIN_I_C2] = 8;
	periodicID[CAN_GW_LIN_I_C2] = 0x281;
	periodicData[CAN_GW_LIN_I_C2] = m_abyte_GW_LIN_I_C2_Msg;

	periodicLength[CAN_GW_LIN_I_C4] = 8;
	periodicID[CAN_GW_LIN_I_C4] = 0x25D;
	periodicData[CAN_GW_LIN_I_C4] = m_abyte_GW_LIN_I_C4_Msg;

	periodicLength[CAN_HCP_C1] = 8;
	periodicID[CAN_HCP_C1] = 0x13B;
	periodicData[CAN_HCP_C1] = m_abyte_HCP_C1_Msg;

	periodicLength[CAN_HCP_DISP] = 8;
	periodicID[CAN_HCP_DISP] = 0x1F9;
	periodicData[CAN_HCP_C1] = m_abyte_HCP_C1_Msg;

	periodicLength[CAN_NAV_DATA] = 8;
	periodicID[CAN_NAV_DATA] = 0x3BC;
	periodicData[CAN_NAV_DATA] = m_abyte_NAV_DATA_Msg;

	periodicLength[CAN_NET_CFG_INT] = 8;
	periodicID[CAN_NET_CFG_INT] = 0x3E3;
	periodicData[CAN_NAV_DATA] = m_abyte_NAV_DATA_Msg;

	periodicLength[CAN_NET_CFG_PT] = 8;
	periodicID[CAN_NET_CFG_PT] = 0x3E4;
	periodicData[CAN_NET_CFG_PT] = m_abyte_NET_CFG_PT_Msg;

	periodicLength[CAN_NM_CBC] = 8;
	periodicID[CAN_NM_CBC] = 0x401;
	periodicData[CAN_NM_CBC] = m_abyte_NM_CBC_Msg;

	periodicLength[CAN_NM_EPS] = 8;
	periodicID[CAN_NM_EPS] = 0x41A;
	periodicData[CAN_NM_EPS] = m_abyte_NM_EPS_Msg;

	periodicLength[CAN_NM_ESC] = 8;
	periodicID[CAN_NM_ESC] = 0x407;
	periodicData[CAN_NM_ESC] = m_abyte_NM_ESC_Msg;

	periodicLength[CAN_NM_ESL] = 8;
	periodicID[CAN_NM_ESL] = 0x408;
	periodicData[CAN_NM_ESL] = m_abyte_NM_ESL_Msg;

	periodicLength[CAN_NM_HCP] = 8;
	periodicID[CAN_NM_HCP] = 0x43A;
	periodicData[CAN_NM_HCP] = m_abyte_NM_HCP_Msg;

	periodicLength[CAN_NM_IC] = 8;
	periodicID[CAN_NM_IC] = 0x402;
	periodicData[CAN_NM_IC] = m_abyte_NM_IC_Msg;

	periodicLength[CAN_NM_RF_HUB] = 8;
	periodicID[CAN_NM_RF_HUB] = 0x400;
	periodicData[CAN_NM_RF_HUB] = m_abyte_NM_RF_HUB_Msg;

	periodicLength[CAN_NM_SBWM] = 8;
	periodicID[CAN_NM_SBWM] = 0x409;
	periodicData[CAN_NM_SBWM] = m_abyte_NM_SBWM_Msg;

	periodicLength[CAN_NM_SCCM] = 8;
	periodicID[CAN_NM_SCCM] = 0x423;
	periodicData[CAN_NM_SCCM] = m_abyte_NM_SCCM_Msg;

	periodicLength[CAN_NM_TPM] = 8;
	periodicID[CAN_NM_TPM] = 0x403;
	periodicData[CAN_NM_TPM] = m_abyte_NM_TPM_Msg;

	periodicLength[CAN_ORC_A1] = 8;
	periodicID[CAN_ORC_A1] = 0x1D0;
	periodicData[CAN_ORC_A1] = m_abyte_ORC_A1_Msg;

	periodicLength[CAN_ORC_A3] = 8;
	periodicID[CAN_ORC_A3] = 0x257;
	periodicData[CAN_ORC_A3] = m_abyte_ORC_A3_Msg;

	periodicLength[CAN_PN14_STAT] = 8;
	periodicID[CAN_PN14_STAT] = 0x248;
	periodicData[CAN_PN14_STAT] = m_abyte_PN14_STAT_Msg;

	periodicLength[CAN_PTS_1] = 8;
	periodicID[CAN_PTS_1] = 0x1F6;
	periodicData[CAN_PTS_1] = m_abyte_PTS_1_Msg;

	periodicLength[CAN_PTS_2] = 8;
	periodicID[CAN_PTS_2] = 0x1E9;
	periodicData[CAN_PTS_2] = m_abyte_PTS_2_Msg;

	periodicLength[CAN_RFHUB_A2] = 6;
	periodicID[CAN_RFHUB_A2] = 0x1C0;
	periodicData[CAN_RFHUB_A2] = m_abyte_RFHUB_A2_Msg;

	periodicLength[CAN_RFHUB_A3] = 8;
	periodicID[CAN_RFHUB_A3] = 0x2A7;
	periodicData[CAN_RFHUB_A3] = m_abyte_RFHUB_A3_Msg;

	periodicLength[CAN_SBW_ROT1] = 5;
	periodicID[CAN_SBW_ROT1] = 0xAD;
	periodicData[CAN_SBW_ROT1] = m_abyte_SBW_ROT1_Msg;

	periodicLength[CAN_SCCM_STW_ANGL_STAT] = 8;
	periodicID[CAN_SCCM_STW_ANGL_STAT] = 0x23;
	periodicData[CAN_SCCM_STW_ANGL_STAT] = m_abyte_SCCM_STW_ANGL_STAT_Msg;

	periodicLength[CAN_STATUS_C_PTS] = 8;
	periodicID[CAN_STATUS_C_PTS] = 0x1EB;
	periodicData[CAN_STATUS_C_PTS] = m_abyte_STATUS_C_PTS_Msg;

	periodicLength[CAN_SWS_8] = 8;
	periodicID[CAN_SWS_8] = 0x22D;
	periodicData[CAN_SWS_8] = m_abyte_SWS_8_Msg;

	periodicLength[CAN_TCM_A7] = 8;
	periodicID[CAN_TCM_A7] = 0xAB;
	periodicData[CAN_TCM_A7] = m_abyte_TCM_A7_Msg;

	periodicLength[CAN_TGW_DATA_IC] = 8;
	periodicID[CAN_TGW_DATA_IC] = 0x328;
	periodicData[CAN_TGW_DATA_IC] = m_abyte_TGW_DATA_IC_Msg;

	periodicLength[CAN_TPM_A1] = 8;
	periodicID[CAN_TPM_A1] = 0x296;
	periodicData[CAN_TPM_A1] = m_abyte_TPM_A1_Msg;

	periodicLength[CAN_TRNS_SPD] = 8;
	periodicID[CAN_TRNS_SPD] = 0x85;
	periodicData[CAN_TRNS_SPD] = m_abyte_TRNS_SPD_Msg;

	periodicLength[CAN_TRNS_STAT] = 8;
	periodicID[CAN_TRNS_STAT] = 0x93;
	periodicData[CAN_TRNS_STAT] = m_abyte_TRNS_STAT_Msg;

	periodicLength[CAN_TRNS_STAT2] = 3;
	periodicID[CAN_TRNS_STAT2] = 0x125;
	periodicData[CAN_TRNS_STAT2] = m_abyte_TRNS_STAT2_Msg;

	periodicLength[CAN_VehCfg1] = 8;
	periodicID[CAN_VehCfg1] = 0x3E8;
	periodicData[CAN_VehCfg1] = m_abyte_VehCfg1_Msg;

	periodicLength[CAN_VehCfg2] = 8;
	periodicID[CAN_VehCfg2] = 0x3E9;
	periodicData[CAN_VehCfg2] = m_abyte_VehCfg2_Msg;

	periodicLength[CAN_VehCfg3] = 8;
	periodicID[CAN_VehCfg3] = 0x3EA;
	periodicData[CAN_VehCfg3] = m_abyte_VehCfg3_Msg;

	periodicLength[CAN_VehCfg5] = 8;
	periodicID[CAN_VehCfg5] = 0x44A;
	periodicData[CAN_VehCfg5] = m_abyte_VehCfg5_Msg;

	periodicLength[CAN_VehCfg6] = 8;
	periodicID[CAN_VehCfg6] = 0x44C;
	periodicData[CAN_VehCfg6] = m_abyte_VehCfg6_Msg;

	periodicLength[CAN_VehCfg7] = 8;
	periodicID[CAN_VehCfg7] = 0x381;
	periodicData[CAN_VehCfg7] = m_abyte_VehCfg7_Msg;

	periodicLength[CAN_VehCfg8] = 8;
	periodicID[CAN_VehCfg8] = 0x38A;
	periodicData[CAN_VehCfg8] = m_abyte_VehCfg8_Msg;

	periodicLength[CAN_VehCfgCSM1] = 8;
	periodicID[CAN_VehCfgCSM1] = 0x3B3;
	periodicData[CAN_VehCfgCSM1] = m_abyte_VehCfgCSM1_Msg;

	periodicLength[CAN_VehCfgCSM2] = 8;
	periodicID[CAN_VehCfgCSM2] = 0x3B4;
	periodicData[CAN_VehCfgCSM2] = m_abyte_VehCfgCSM2_Msg;

	periodicLength[CAN_VIN] = 8;
	periodicID[CAN_VIN] = 0x3E0;
	periodicData[CAN_VIN] = m_abyte_VIN_Msg;
}

osTimerId timer;
WM_HWIN openPanels(void)
{
	hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
	initArrays();
//	/* Start the TS Timer */
	osThreadDef(periodic_CAN_Timer, periodic_CAN_Timer, osPriorityNormal, 0, 2 * 1024);
	osThreadCreate (osThread(periodic_CAN_Timer), NULL);

	return hWin;
}
