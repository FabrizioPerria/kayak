#include "panelsScreen.h"
#include "cmsis_os.h"
#include <string.h>
#include "main.h"

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
//static int speed;
//static int rpm;
//static int choiceCBCPT2;
static int periodicID[50];
static int periodicData[50];
static int periodicLength[50];
extern CAN_HandleTypeDef hcan1;
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
CanTxMsgTypeDef msg;

// USER START (Optionally insert additional static code)
static void sendCANMessage(uint32_t ID, uint32_t length, uint8_t* message)
{
	for(int i = 0; i < 8; ++i)
		msg.Data[i] = message[i];
	msg.DLC = length;
	msg.StdId = ID;
	msg.IDE = CAN_ID_STD;

	hcan1.pTxMsg = &msg;
//	if (HAL_CAN_Transmit(&hcan1, 10000) != HAL_OK)
//		ErrorHandler();
	HAL_CAN_Transmit(&hcan1, 100);
}

static void mainCallback(WM_MESSAGE *pMsg)
{
	WM_HWIN hItem;
	int     NCode;
	int     Id;
	uint8_t buffer[8];

	switch (pMsg->MsgId) {
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
			switch (NCode) {
			case WM_NOTIFICATION_VALUE_CHANGED:
				hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_0);
				periodicData[SPEED] = SLIDER_GetValue(hItem) * 128;
				break;
			}
			break;
		case ID_SLIDER_1: // Notifications sent by 'Slider'
			switch (NCode) {
			case WM_NOTIFICATION_VALUE_CHANGED:
				hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_1);
				periodicData[RPM] = SLIDER_GetValue(hItem);
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
					periodicData[CBC_PT2] = 3;
					break;
				case 2:
					periodicData[CBC_PT2] = 4;
					break;
				case 3:
					periodicData[CBC_PT2] = 5;
					break;
				default:
					periodicData[CBC_PT2] = 0;
					break;
				}
				break;
			}
			break;
		case ID_BUTTON_0: // Notifications sent by 'Up'
			switch (NCode) {
			case WM_NOTIFICATION_CLICKED:
				buffer[5] = 0x04;
				sendCANMessage(0x22D, 8, buffer);
				break;
			case WM_NOTIFICATION_RELEASED:
				sendCANMessage(0x22D, 8, buffer);
				break;
			}
			break;
		case ID_BUTTON_1: // Notifications sent by 'Right'
			switch (NCode) {
			case WM_NOTIFICATION_CLICKED:
				buffer[5] = 0x01;
				sendCANMessage(0x22D, 8, buffer);
				break;
			case WM_NOTIFICATION_RELEASED:
				sendCANMessage(0x22D, 8, buffer);
				break;
			}
			break;
		case ID_BUTTON_2: // Notifications sent by 'Left'
			switch (NCode) {
			case WM_NOTIFICATION_CLICKED:
				buffer[4] = 0x10;
				sendCANMessage(0x22D, 8, buffer);
				break;
			case WM_NOTIFICATION_RELEASED:
				sendCANMessage(0x22D, 8, buffer);
				break;
			}
			break;
		case ID_BUTTON_3: // Notifications sent by 'Down'
			switch (NCode) {
			case WM_NOTIFICATION_CLICKED:
				buffer[4] = 0x40;
				sendCANMessage(0x22D, 8, buffer);
				break;
			case WM_NOTIFICATION_RELEASED:
				sendCANMessage(0x22D, 8, buffer);
				break;
			}
			break;
		case ID_BUTTON_4: // Notifications sent by 'OK'
			switch (NCode) {
			case WM_NOTIFICATION_CLICKED:
				buffer[5] = 0x10;
				sendCANMessage(0x22D, 8, buffer);
				break;
			case WM_NOTIFICATION_RELEASED:
				sendCANMessage(0x22D, 8, buffer);
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

CanTxMsgTypeDef objt_CAN_AMB_TEMP_DISP_Msg;
	CanTxMsgTypeDef objt_CAN_APPL_ECU_IC_Msg;
	CanTxMsgTypeDef objt_CAN_ASBS_1_Msg;
	CanTxMsgTypeDef objt_CAN_BSM_A1_Msg;
	CanTxMsgTypeDef objt_CAN_CBC_PT1_Msg;
	CanTxMsgTypeDef objt_CAN_CBC_PT10_Msg;
	CanTxMsgTypeDef objt_CAN_CBC_PT2_Msg;
	CanTxMsgTypeDef objt_CAN_CBC_PT3_Msg;
	CanTxMsgTypeDef objt_CAN_CBC_PT4_Msg;
	CanTxMsgTypeDef objt_CAN_CBC_PT8_Msg;
	CanTxMsgTypeDef objt_CAN_CBC_PT9_Msg;
	CanTxMsgTypeDef objt_CAN_CFG_RQ_Msg;
	CanTxMsgTypeDef objt_CAN_Clock_Date_Msg;
	CanTxMsgTypeDef objt_CAN_COMPASS_A1_Msg;
	CanTxMsgTypeDef objt_CAN_D_RQ_IC_Msg;
	CanTxMsgTypeDef objt_CAN_DG_RQ_GLOBAL_UDS_Msg;
	CanTxMsgTypeDef objt_CAN_DIRECT_INFO_Msg;
	CanTxMsgTypeDef objt_CAN_DTCM_A1_Msg;
	CanTxMsgTypeDef objt_CAN_DTCM_B1_Msg;
	CanTxMsgTypeDef objt_CAN_ECM_A1_Msg;
	CanTxMsgTypeDef objt_CAN_ECM_A3_Msg;
	CanTxMsgTypeDef objt_CAN_ECM_B11_Msg;
	CanTxMsgTypeDef objt_CAN_ECM_B2_Msg;
	CanTxMsgTypeDef objt_CAN_ECM_B3_Msg;
	CanTxMsgTypeDef objt_CAN_ECM_B5_Msg;
	CanTxMsgTypeDef objt_CAN_ECM_B9_Msg;
	CanTxMsgTypeDef objt_CAN_ECM_CRUISE_MAP_Msg;
	CanTxMsgTypeDef objt_CAN_ECM_DIESEL_Msg;
	CanTxMsgTypeDef objt_CAN_ECM_INDICATORS_Msg;
	CanTxMsgTypeDef objt_CAN_EcuCfg10_Msg;
	CanTxMsgTypeDef objt_CAN_EcuCfg12_Msg;
	CanTxMsgTypeDef objt_CAN_EcuCfg4_Msg;
	CanTxMsgTypeDef objt_CAN_ENG_CFG_Msg;
	CanTxMsgTypeDef objt_CAN_EPS_A1_Msg;
	CanTxMsgTypeDef objt_CAN_ESP_A1_Msg;
	CanTxMsgTypeDef objt_CAN_ESP_A8_Msg;
	CanTxMsgTypeDef objt_CAN_ESP_B1_Msg;
	CanTxMsgTypeDef objt_CAN_GW_I_C1_Msg;
	CanTxMsgTypeDef objt_CAN_GW_LIN_I_C2_Msg;
	CanTxMsgTypeDef objt_CAN_GW_LIN_I_C4_Msg;
	CanTxMsgTypeDef objt_CAN_HCP_C1_Msg;
	CanTxMsgTypeDef objt_CAN_HCP_DISP_Msg;
	CanTxMsgTypeDef objt_CAN_NAV_DATA_Msg;
	CanTxMsgTypeDef objt_CAN_NET_CFG_INT_Msg;
	CanTxMsgTypeDef objt_CAN_NET_CFG_PT_Msg;
	CanTxMsgTypeDef objt_CAN_NM_CBC_Msg;
	CanTxMsgTypeDef objt_CAN_NM_EPS_Msg;
	CanTxMsgTypeDef objt_CAN_NM_ESC_Msg;
	CanTxMsgTypeDef objt_CAN_NM_ESL_Msg;
	CanTxMsgTypeDef objt_CAN_NM_HCP_Msg;
	CanTxMsgTypeDef objt_CAN_NM_IC_Msg;
	CanTxMsgTypeDef objt_CAN_NM_RF_HUB_Msg;
	CanTxMsgTypeDef objt_CAN_NM_SBWM_Msg;
	CanTxMsgTypeDef objt_CAN_NM_SCCM_Msg;
	CanTxMsgTypeDef objt_CAN_NM_TPM_Msg;
	CanTxMsgTypeDef objt_CAN_ORC_A1_Msg;
	CanTxMsgTypeDef objt_CAN_ORC_A3_Msg;
	CanTxMsgTypeDef objt_CAN_PN14_STAT_Msg;
	CanTxMsgTypeDef objt_CAN_PTS_1_Msg;
	CanTxMsgTypeDef objt_CAN_PTS_2_Msg;
	CanTxMsgTypeDef objt_CAN_RFHUB_A2_Msg;
	CanTxMsgTypeDef objt_CAN_RFHUB_A3_Msg;
	CanTxMsgTypeDef objt_CAN_SBW_ROT1_Msg;
	CanTxMsgTypeDef objt_CAN_SCCM_STW_ANGL_STAT_Msg;
	CanTxMsgTypeDef objt_CAN_STATUS_C_PTS_Msg;
	CanTxMsgTypeDef objt_CAN_SWS_8_Msg;
	CanTxMsgTypeDef objt_CAN_TCM_A7_Msg;
	CanTxMsgTypeDef objt_CAN_TGW_DATA_IC_Msg;
	CanTxMsgTypeDef objt_CAN_TPM_A1_Msg;
	CanTxMsgTypeDef objt_CAN_TRNS_SPD_Msg;
	CanTxMsgTypeDef objt_CAN_TRNS_STAT_Msg;
	CanTxMsgTypeDef objt_CAN_TRNS_STAT2_Msg;
	CanTxMsgTypeDef objt_CAN_VehCfg1_Msg;
	CanTxMsgTypeDef objt_CAN_VehCfg2_Msg;
	CanTxMsgTypeDef objt_CAN_VehCfg3_Msg;
	CanTxMsgTypeDef objt_CAN_VehCfg5_Msg;
	CanTxMsgTypeDef objt_CAN_VehCfg6_Msg;
	CanTxMsgTypeDef objt_CAN_VehCfg7_Msg;
	CanTxMsgTypeDef objt_CAN_VehCfg8_Msg;
	uint8_t m_abyte_CBC_PT9_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	CanTxMsgTypeDef objt_CAN_VehCfgCSM1_Msg;
	CanTxMsgTypeDef objt_CAN_VehCfgCSM2_Msg;
	CanTxMsgTypeDef objt_CAN_VIN_Msg;


	uint8_t m_abyte_AMB_TEMP_DISP_Msg[] = {0x70, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_APPL_ECU_IC_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_ASBS_1_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_BSM_A1_Msg[] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_CBC_PT1_Msg[] = {0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0A, 0x00};
	uint8_t m_abyte_CBC_PT10_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_CBC_PT2_Msg[] = {0x04, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_CBC_PT3_Msg[] = {0x82, 0x14, 0xB4, 0xB3, 0x46, 0x4A, 0x8A, 0x00};
	uint8_t m_abyte_CBC_PT4_Msg[] = {0xC8, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_CBC_PT8_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF};
	uint8_t m_abyte_CFG_RQ_Msg[] = {0x0A, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_Clock_Date_Msg[] = {0x18, 0x0E, 0x03, 0x07, 0xE1, 0x05, 0x19, 0x02};
	uint8_t m_abyte_COMPASS_A1_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_D_RQ_IC_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_DG_RQ_GLOBAL_UDS_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_DIRECT_INFO_Msg[] = {0x07, 0xD0, 0x0B, 0x47, 0x06, 0x00, 0x00, 0x00};
	uint8_t m_abyte_DTCM_A1_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_DTCM_B1_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_ECM_A1_Msg[] = {0x05, 0xDC, 0x07, 0xCC, 0x07, 0xCC, 0x00, 0xFF};
	uint8_t m_abyte_ECM_A3_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_ECM_B11_Msg[] = {0x00, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_ECM_B2_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_ECM_B3_Msg[] = {0x2C, 0x8C, 0x7B, 0x00, 0xDF, 0x00, 0xC8, 0x00};
	uint8_t m_abyte_ECM_B5_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_ECM_B9_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_ECM_CRUISE_MAP_Msg[] = {0x00, 0x78, 0x3F, 0x21, 0xCD, 0xCD, 0x00, 0x00};
	uint8_t m_abyte_ECM_DIESEL_Msg[] = {0x80, 0x01, 0x00, 0x00, 0x00, 0x09, 0x00, 0x07};
	uint8_t m_abyte_ECM_INDICATORS_Msg[] = {0x00, 0x00, 0x32, 0x53, 0x0E, 0x02, 0x58, 0x04};
	uint8_t m_abyte_EcuCfg10_Msg[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_EcuCfg12_Msg[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_EcuCfg4_Msg[] = {0x81, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_ENG_CFG_Msg[] = {0x26, 0x18, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_EPS_A1_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_ESP_A1_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_ESP_A8_Msg[] = {0x05, 0x55, 0x05, 0x55, 0x26, 0x80, 0x77, 0x33 };
	uint8_t m_abyte_ESP_B1_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_GW_I_C1_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xE9, 0x00, 0x00};
	uint8_t m_abyte_GW_LIN_I_C2_Msg[] = {0x0F, 0xEC, 0x07, 0xEC, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_GW_LIN_I_C4_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_HCP_C1_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_HCP_DISP_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_NAV_DATA_Msg[] = {0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_NET_CFG_INT_Msg[] = {0xFD, 0xFF, 0xFE, 0xFF, 0xFF, 0x80, 0x00, 0x00};
	uint8_t m_abyte_NET_CFG_PT_Msg[] = {0xF9, 0xFF, 0x9F, 0xFF, 0xF0, 0x00, 0x00, 0x00};
	uint8_t m_abyte_NM_CBC_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_NM_EPS_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_NM_ESC_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_NM_ESL_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_NM_HCP_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_NM_IC_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_NM_RF_HUB_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_NM_SBWM_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_NM_SCCM_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_NM_TPM_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_ORC_A1_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_ORC_A3_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_PN14_STAT_Msg[] = {0x00, 0x00, 0xFF, 0xFF, 0xFE, 0x03, 0xFF, 0xFF};
	uint8_t m_abyte_PTS_1_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_PTS_2_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_RFHUB_A2_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_RFHUB_A3_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_SBW_ROT1_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_SCCM_STW_ANGL_STAT_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_STATUS_C_PTS_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_SWS_8_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_TCM_A7_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_TGW_DATA_IC_Msg[] = {0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_TPM_A1_Msg[] = {0x01, 0x00, 0x00, 0x23, 0x22, 0x21, 0x20, 0x24};
	uint8_t m_abyte_TRNS_SPD_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_TRNS_STAT_Msg[] = {0x10, 0x04, 0x50, 0x00, 0x00, 0x00, 0x00, 0xFF};
	uint8_t m_abyte_TRNS_STAT2_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t m_abyte_VehCfg1_Msg[] = { 0x49, 0x31, 0xA0, 0x7A, 0x64, 0x0E, 0x00, 0x56 };
	uint8_t m_abyte_VehCfg2_Msg[] = { 0x21, 0x5D, 0x66, 0x30, 0x2A, 0x2C, 0xFF, 0xFF };
	uint8_t m_abyte_VehCfg3_Msg[] = { 0x71, 0x41, 0x27, 0xCA, 0x50, 0x36, 0x8F, 0xF5 };
	uint8_t m_abyte_VehCfg5_Msg[] = { 0xC1, 0x00, 0x32, 0x64, 0x96, 0xC8, 0xFA, 0xD4 };
	uint8_t m_abyte_VehCfg6_Msg[] = { 0x3D, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0x01, 0xFC };
	uint8_t m_abyte_VehCfg7_Msg[] = { 0x8D, 0x8D, 0x6B, 0x08, 0x41, 0x84, 0x00, 0x00 };
	uint8_t m_abyte_VehCfg8_Msg[] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	uint8_t m_abyte_VehCfgCSM1_Msg[] = {0x67, 0x29, 0x35, 0x01, 0xFF, 0xA3, 0xC7, 0x18};
	uint8_t m_abyte_VehCfgCSM2_Msg[] = {0xB7, 0x80, 0x00, 0x00, 0x00, 0x40, 0x06, 0x00};
	uint8_t m_abyte_VIN_Msg[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


void periodic_CAN_Timer(void)
{
	uint8_t buffer[8];
	//CBCPT2
	memset(buffer, 0, 8);
	buffer[0] = (uint8_t) (periodicData[CBC_PT2] & 0xFF);
	sendCANMessage(periodicID[CBC_PT2], periodicLength[CBC_PT2], buffer);
	//SPEED
	memset(buffer, 0, 8);
	buffer[4] = (uint8_t) ((periodicData[SPEED] >> 8) & 0xFF);
	buffer[5] = (uint8_t) (periodicData[SPEED] & 0xFF);
	sendCANMessage(periodicID[SPEED], periodicLength[SPEED], buffer);
	//RPM
	memset(buffer, 0, 8);
	buffer[0] = (uint8_t) ((periodicData[RPM] >> 8) & 0xFF);
	buffer[1] = (uint8_t) (periodicData[RPM] & 0xFF);
	sendCANMessage(periodicID[RPM], periodicLength[RPM], buffer);

	  static uint32_t ui32_Millisecond_Count = 0;
	  ui32_Millisecond_Count++;
	  if ((ui32_Millisecond_Count % 10) == 0)
	  {
	    //hcan1.pTxMsg = &objt_CAN_Tx_Msg;
	    //HAL_CAN_Transmit(&hcan1, 10000);
	    //objt_CAN_Tx_Msg.Data[0]++;
	    hcan1.pTxMsg = &objt_CAN_ECM_A1_Msg;
	    //HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_ECM_A3_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_SCCM_STW_ANGL_STAT_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	  }
	  if ((ui32_Millisecond_Count % (20-2)) == 0)  // start if for 20 msec send periodic
	  {
	    hcan1.pTxMsg = &objt_CAN_CBC_PT2_Msg;
	   // HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_DTCM_A1_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_ECM_CRUISE_MAP_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_ESP_A1_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_ESP_A8_Msg;
	    //HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_SBW_ROT1_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_TCM_A7_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_TRNS_SPD_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_TRNS_STAT_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	  }    // end if for 20 msec send periodic

	  if ((ui32_Millisecond_Count % (50 - 6)) == 0)  // start if for 50 msec send periodic
	  {
	    hcan1.pTxMsg = &objt_CAN_ESP_B1_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_CFG_RQ_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	  }    // end if for 50 msec send periodic

	  if ((ui32_Millisecond_Count % (100 - 12)) == 0)  // start if for 100 msec send periodic
	  {
	    hcan1.pTxMsg = &objt_CAN_CBC_PT3_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_CBC_PT9_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_ECM_B11_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_ECM_B2_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_ECM_B3_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_ECM_B9_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_ECM_DIESEL_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_ECM_INDICATORS_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_ENG_CFG_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_EPS_A1_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_HCP_C1_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_TRNS_STAT2_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_VIN_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	  }    // end if for 100 msec send periodic

	  if ((ui32_Millisecond_Count % (200 - 26)) == 0)  // start if for 200 msec send periodic
	  {
	    hcan1.pTxMsg = &objt_CAN_RFHUB_A2_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	  }    // end if for 200 msec send periodic

	  if ((ui32_Millisecond_Count % (500 - 60)) == 0)  // start if for 500 msec send periodic
	  {
	    hcan1.pTxMsg = &objt_CAN_BSM_A1_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_HCP_DISP_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_PN14_STAT_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_PTS_1_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_PTS_2_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_STATUS_C_PTS_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_VehCfg1_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	  }    // end if for 500 msec send periodic

	  if ((ui32_Millisecond_Count % (1000 - 120)) == 0)  // start if for 1000 msec send periodic
	  {
	    hcan1.pTxMsg = &objt_CAN_AMB_TEMP_DISP_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_ASBS_1_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_CBC_PT1_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_CBC_PT10_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_CBC_PT4_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_CBC_PT8_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_Clock_Date_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_COMPASS_A1_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_DIRECT_INFO_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_DTCM_B1_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_ECM_B5_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_GW_I_C1_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_GW_LIN_I_C2_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_GW_LIN_I_C4_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_ORC_A1_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_ORC_A3_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_RFHUB_A3_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_SWS_8_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_TGW_DATA_IC_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_TPM_A1_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	  }    // end if for 1000 msec send periodic

	  if ((ui32_Millisecond_Count % 1760) == 0)  // start if for 2000 msec send periodic
	  {
	    hcan1.pTxMsg = &objt_CAN_EcuCfg10_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_EcuCfg12_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_EcuCfg4_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_NET_CFG_INT_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_NET_CFG_PT_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_VehCfg2_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_VehCfg3_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_VehCfg5_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_VehCfg6_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_VehCfg7_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_VehCfg8_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_VehCfgCSM1_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	    hcan1.pTxMsg = &objt_CAN_VehCfgCSM2_Msg;
	    HAL_CAN_Transmit(&hcan1, 10000);
	  }    // end if for 2000 msec send periodic
}

void initArrays(void)
{
	periodicID[SPEED] = 0x79;
	periodicLength[SPEED] = 8;

	periodicID[RPM] = 0x25;
	periodicLength[RPM] = 8;

	periodicID[CBC_PT2] = 0x77;
	periodicLength[CBC_PT2] = 2;

	objt_CAN_AMB_TEMP_DISP_Msg.DLC = 4;
	objt_CAN_AMB_TEMP_DISP_Msg.StdId = 0x3C9;
	objt_CAN_AMB_TEMP_DISP_Msg.IDE = CAN_ID_STD;
	objt_CAN_APPL_ECU_IC_Msg.DLC = 8;
	objt_CAN_APPL_ECU_IC_Msg.StdId = 0x6C2;
	objt_CAN_APPL_ECU_IC_Msg.IDE = CAN_ID_STD;
	objt_CAN_ASBS_1_Msg.DLC = 8;
	objt_CAN_ASBS_1_Msg.StdId = 0x371;
	objt_CAN_ASBS_1_Msg.IDE = CAN_ID_STD;
	objt_CAN_BSM_A1_Msg.DLC = 6;
	objt_CAN_BSM_A1_Msg.StdId = 0x2D0;
	objt_CAN_BSM_A1_Msg.IDE = CAN_ID_STD;
	objt_CAN_CBC_PT1_Msg.DLC = 8;
	objt_CAN_CBC_PT1_Msg.StdId = 0x291;
	objt_CAN_CBC_PT1_Msg.IDE = CAN_ID_STD;
	objt_CAN_CBC_PT10_Msg.DLC = 8;
	objt_CAN_CBC_PT10_Msg.StdId = 0x265;
	objt_CAN_CBC_PT10_Msg.IDE = CAN_ID_STD;
	objt_CAN_CBC_PT2_Msg.DLC = 2;
	objt_CAN_CBC_PT2_Msg.StdId = 0x77;
	objt_CAN_CBC_PT2_Msg.IDE = CAN_ID_STD;
	objt_CAN_CBC_PT3_Msg.DLC = 8;
	objt_CAN_CBC_PT3_Msg.StdId = 0x12B;
	objt_CAN_CBC_PT3_Msg.IDE = CAN_ID_STD;
	objt_CAN_CBC_PT4_Msg.DLC = 8;
	objt_CAN_CBC_PT4_Msg.StdId = 0x283;
	objt_CAN_CBC_PT4_Msg.IDE = CAN_ID_STD;
	objt_CAN_CBC_PT8_Msg.DLC = 8;
	objt_CAN_CBC_PT8_Msg.StdId = 0x25F;
	objt_CAN_CBC_PT8_Msg.IDE = CAN_ID_STD;
	objt_CAN_CBC_PT9_Msg.DLC = 8;
	objt_CAN_CBC_PT9_Msg.StdId = 0x13F;
	objt_CAN_CBC_PT9_Msg.IDE = CAN_ID_STD;
	objt_CAN_CFG_RQ_Msg.DLC = 3;
	objt_CAN_CFG_RQ_Msg.StdId = 0x314;
	objt_CAN_CFG_RQ_Msg.IDE = CAN_ID_STD;
	objt_CAN_Clock_Date_Msg.DLC = 8;
	objt_CAN_Clock_Date_Msg.StdId = 0x350;
	objt_CAN_Clock_Date_Msg.IDE = CAN_ID_STD;
	objt_CAN_COMPASS_A1_Msg.DLC = 4;
	objt_CAN_COMPASS_A1_Msg.StdId = 0x358;
	objt_CAN_COMPASS_A1_Msg.IDE = CAN_ID_STD;
	objt_CAN_D_RQ_IC_Msg.DLC = 8;
	objt_CAN_D_RQ_IC_Msg.StdId = 0x742;
	objt_CAN_D_RQ_IC_Msg.IDE = CAN_ID_STD;
	objt_CAN_DG_RQ_GLOBAL_UDS_Msg.DLC = 8;
	objt_CAN_DG_RQ_GLOBAL_UDS_Msg.StdId = 0x441;
	objt_CAN_DG_RQ_GLOBAL_UDS_Msg.IDE = CAN_ID_STD;
	objt_CAN_DIRECT_INFO_Msg.DLC = 6;
	objt_CAN_DIRECT_INFO_Msg.StdId = 0x35C;
	objt_CAN_DIRECT_INFO_Msg.IDE = CAN_ID_STD;
	objt_CAN_DTCM_A1_Msg.DLC = 8;
	objt_CAN_DTCM_A1_Msg.StdId = 0x8D;
	objt_CAN_DTCM_A1_Msg.IDE = CAN_ID_STD;
	objt_CAN_DTCM_B1_Msg.DLC = 8;
	objt_CAN_DTCM_B1_Msg.StdId = 0x277;
	objt_CAN_DTCM_B1_Msg.IDE = CAN_ID_STD;
	objt_CAN_ECM_A1_Msg.DLC = 8;
	objt_CAN_ECM_A1_Msg.StdId = 0x25;
	objt_CAN_ECM_A1_Msg.IDE = CAN_ID_STD;
	objt_CAN_ECM_A3_Msg.DLC = 8;
	objt_CAN_ECM_A3_Msg.StdId = 0x35;
	objt_CAN_ECM_A3_Msg.IDE = CAN_ID_STD;
	objt_CAN_ECM_B11_Msg.DLC = 8;
	objt_CAN_ECM_B11_Msg.StdId = 0x2A4;
	objt_CAN_ECM_B11_Msg.IDE = CAN_ID_STD;
	objt_CAN_ECM_B2_Msg.DLC = 8;
	objt_CAN_ECM_B2_Msg.StdId = 0x137;
	objt_CAN_ECM_B2_Msg.IDE = CAN_ID_STD;
	objt_CAN_ECM_B3_Msg.DLC = 8;
	objt_CAN_ECM_B3_Msg.StdId = 0x127;
	objt_CAN_ECM_B3_Msg.IDE = CAN_ID_STD;
	objt_CAN_ECM_B5_Msg.DLC = 8;
	objt_CAN_ECM_B5_Msg.StdId = 0x279;
	objt_CAN_ECM_B5_Msg.IDE = CAN_ID_STD;
	objt_CAN_ECM_B9_Msg.DLC = 8;
	objt_CAN_ECM_B9_Msg.StdId = 0x157;
	objt_CAN_ECM_B9_Msg.IDE = CAN_ID_STD;
	objt_CAN_ECM_CRUISE_MAP_Msg.DLC = 8;
	objt_CAN_ECM_CRUISE_MAP_Msg.StdId = 0x81;
	objt_CAN_ECM_CRUISE_MAP_Msg.IDE = CAN_ID_STD;
	objt_CAN_ECM_DIESEL_Msg.DLC = 8;
	objt_CAN_ECM_DIESEL_Msg.StdId = 0x12D;
	objt_CAN_ECM_DIESEL_Msg.IDE = CAN_ID_STD;
	objt_CAN_ECM_INDICATORS_Msg.DLC = 8;
	objt_CAN_ECM_INDICATORS_Msg.StdId = 0x13D;
	objt_CAN_ECM_INDICATORS_Msg.IDE = CAN_ID_STD;
	objt_CAN_EcuCfg10_Msg.DLC = 8;
	objt_CAN_EcuCfg10_Msg.StdId = 0x3D7;
	objt_CAN_EcuCfg10_Msg.IDE = CAN_ID_STD;
	objt_CAN_EcuCfg12_Msg.DLC = 8;
	objt_CAN_EcuCfg12_Msg.StdId = 0x3D5;
	objt_CAN_EcuCfg12_Msg.IDE = CAN_ID_STD;
	objt_CAN_EcuCfg4_Msg.DLC = 8;
	objt_CAN_EcuCfg4_Msg.StdId = 0x3F3;
	objt_CAN_EcuCfg4_Msg.IDE = CAN_ID_STD;
	objt_CAN_ENG_CFG_Msg.DLC = 7;
	objt_CAN_ENG_CFG_Msg.StdId = 0x3E1;
	objt_CAN_ENG_CFG_Msg.IDE = CAN_ID_STD;
	objt_CAN_EPS_A1_Msg.DLC = 8;
	objt_CAN_EPS_A1_Msg.StdId = 0x128;
	objt_CAN_EPS_A1_Msg.IDE = CAN_ID_STD;
	objt_CAN_ESP_A1_Msg.DLC = 8;
	objt_CAN_ESP_A1_Msg.StdId = 0x83;
	objt_CAN_ESP_A1_Msg.IDE = CAN_ID_STD;
	objt_CAN_ESP_A8_Msg.DLC = 8;
	objt_CAN_ESP_A8_Msg.StdId = 0x79;
	objt_CAN_ESP_A8_Msg.IDE = CAN_ID_STD;
	objt_CAN_ESP_B1_Msg.DLC = 8;
	objt_CAN_ESP_B1_Msg.StdId = 0xDD;
	objt_CAN_ESP_B1_Msg.IDE = CAN_ID_STD;
	objt_CAN_GW_I_C1_Msg.DLC = 8;
	objt_CAN_GW_I_C1_Msg.StdId = 0x289;
	objt_CAN_GW_I_C1_Msg.IDE = CAN_ID_STD;
	objt_CAN_GW_LIN_I_C2_Msg.DLC = 8;
	objt_CAN_GW_LIN_I_C2_Msg.StdId = 0x281;
	objt_CAN_GW_LIN_I_C2_Msg.IDE = CAN_ID_STD;
	objt_CAN_GW_LIN_I_C4_Msg.DLC = 8;
	objt_CAN_GW_LIN_I_C4_Msg.StdId = 0x25D;
	objt_CAN_GW_LIN_I_C4_Msg.IDE = CAN_ID_STD;
	objt_CAN_HCP_C1_Msg.DLC = 8;
	objt_CAN_HCP_C1_Msg.StdId = 0x13B;
	objt_CAN_HCP_C1_Msg.IDE = CAN_ID_STD;
	objt_CAN_HCP_DISP_Msg.DLC = 8;
	objt_CAN_HCP_DISP_Msg.StdId = 0x1F9;
	objt_CAN_HCP_DISP_Msg.IDE = CAN_ID_STD;
	objt_CAN_NAV_DATA_Msg.DLC = 8;
	objt_CAN_NAV_DATA_Msg.StdId = 0x3BC;
	objt_CAN_NAV_DATA_Msg.IDE = CAN_ID_STD;
	objt_CAN_NET_CFG_INT_Msg.DLC = 8;
	objt_CAN_NET_CFG_INT_Msg.StdId = 0x3E3;
	objt_CAN_NET_CFG_INT_Msg.IDE = CAN_ID_STD;
	objt_CAN_NET_CFG_PT_Msg.DLC = 8;
	objt_CAN_NET_CFG_PT_Msg.StdId = 0x3E4;
	objt_CAN_NET_CFG_PT_Msg.IDE = CAN_ID_STD;
	objt_CAN_NM_CBC_Msg.DLC = 8;
	objt_CAN_NM_CBC_Msg.StdId = 0x401;
	objt_CAN_NM_CBC_Msg.IDE = CAN_ID_STD;
	objt_CAN_NM_EPS_Msg.DLC = 8;
	objt_CAN_NM_EPS_Msg.StdId = 0x41A;
	objt_CAN_NM_EPS_Msg.IDE = CAN_ID_STD;
	objt_CAN_NM_ESC_Msg.DLC = 8;
	objt_CAN_NM_ESC_Msg.StdId = 0x407;
	objt_CAN_NM_ESC_Msg.IDE = CAN_ID_STD;
	objt_CAN_NM_ESL_Msg.DLC = 8;
	objt_CAN_NM_ESL_Msg.StdId = 0x408;
	objt_CAN_NM_ESL_Msg.IDE = CAN_ID_STD;
	objt_CAN_NM_HCP_Msg.DLC = 8;
	objt_CAN_NM_HCP_Msg.StdId = 0x43A;
	objt_CAN_NM_HCP_Msg.IDE = CAN_ID_STD;
	objt_CAN_NM_IC_Msg.DLC = 8;
	objt_CAN_NM_IC_Msg.StdId = 0x402;
	objt_CAN_NM_IC_Msg.IDE = CAN_ID_STD;
	objt_CAN_NM_RF_HUB_Msg.DLC = 8;
	objt_CAN_NM_RF_HUB_Msg.StdId = 0x400;
	objt_CAN_NM_RF_HUB_Msg.IDE = CAN_ID_STD;
	objt_CAN_NM_SBWM_Msg.DLC = 8;
	objt_CAN_NM_SBWM_Msg.StdId = 0x409;
	objt_CAN_NM_SBWM_Msg.IDE = CAN_ID_STD;
	objt_CAN_NM_SCCM_Msg.DLC = 8;
	objt_CAN_NM_SCCM_Msg.StdId = 0x423;
	objt_CAN_NM_SCCM_Msg.IDE = CAN_ID_STD;
	objt_CAN_NM_TPM_Msg.DLC = 8;
	objt_CAN_NM_TPM_Msg.StdId = 0x403;
	objt_CAN_NM_TPM_Msg.IDE = CAN_ID_STD;
	objt_CAN_ORC_A1_Msg.DLC = 8;
	objt_CAN_ORC_A1_Msg.StdId = 0x1D0;
	objt_CAN_ORC_A1_Msg.IDE = CAN_ID_STD;
	objt_CAN_ORC_A3_Msg.DLC = 8;
	objt_CAN_ORC_A3_Msg.StdId = 0x257;
	objt_CAN_ORC_A3_Msg.IDE = CAN_ID_STD;
	objt_CAN_PN14_STAT_Msg.DLC = 8;
	objt_CAN_PN14_STAT_Msg.StdId = 0x248;
	objt_CAN_PN14_STAT_Msg.IDE = CAN_ID_STD;
	objt_CAN_PTS_1_Msg.DLC = 8;
	objt_CAN_PTS_1_Msg.StdId = 0x1F6;
	objt_CAN_PTS_1_Msg.IDE = CAN_ID_STD;
	objt_CAN_PTS_2_Msg.DLC = 8;
	objt_CAN_PTS_2_Msg.StdId = 0x1E9;
	objt_CAN_PTS_2_Msg.IDE = CAN_ID_STD;
	objt_CAN_RFHUB_A2_Msg.DLC = 6;
	objt_CAN_RFHUB_A2_Msg.StdId = 0x1C0;
	objt_CAN_RFHUB_A2_Msg.IDE = CAN_ID_STD;
	objt_CAN_RFHUB_A3_Msg.DLC = 8;
	objt_CAN_RFHUB_A3_Msg.StdId = 0x2A7;
	objt_CAN_RFHUB_A3_Msg.IDE = CAN_ID_STD;
	objt_CAN_SBW_ROT1_Msg.DLC = 5;
	objt_CAN_SBW_ROT1_Msg.StdId = 0xAD;
	objt_CAN_SBW_ROT1_Msg.IDE = CAN_ID_STD;
	objt_CAN_SCCM_STW_ANGL_STAT_Msg.DLC = 8;
	objt_CAN_SCCM_STW_ANGL_STAT_Msg.StdId = 0x23;
	objt_CAN_SCCM_STW_ANGL_STAT_Msg.IDE = CAN_ID_STD;
	objt_CAN_STATUS_C_PTS_Msg.DLC = 8;
	objt_CAN_STATUS_C_PTS_Msg.StdId = 0x1EB;
	objt_CAN_STATUS_C_PTS_Msg.IDE = CAN_ID_STD;
	objt_CAN_SWS_8_Msg.DLC = 8;
	objt_CAN_SWS_8_Msg.StdId = 0x22D;
	objt_CAN_SWS_8_Msg.IDE = CAN_ID_STD;
	objt_CAN_TCM_A7_Msg.DLC = 8;
	objt_CAN_TCM_A7_Msg.StdId = 0xAB;
	objt_CAN_TCM_A7_Msg.IDE = CAN_ID_STD;
	objt_CAN_TGW_DATA_IC_Msg.DLC = 8;
	objt_CAN_TGW_DATA_IC_Msg.StdId = 0x328;
	objt_CAN_TGW_DATA_IC_Msg.IDE = CAN_ID_STD;
	objt_CAN_TPM_A1_Msg.DLC = 8;
	objt_CAN_TPM_A1_Msg.StdId = 0x296;
	objt_CAN_TPM_A1_Msg.IDE = CAN_ID_STD;
	objt_CAN_TRNS_SPD_Msg.DLC = 8;
	objt_CAN_TRNS_SPD_Msg.StdId = 0x85;
	objt_CAN_TRNS_SPD_Msg.IDE = CAN_ID_STD;
	objt_CAN_TRNS_STAT_Msg.DLC = 8;
	objt_CAN_TRNS_STAT_Msg.StdId = 0x93;
	objt_CAN_TRNS_STAT_Msg.IDE = CAN_ID_STD;
	objt_CAN_TRNS_STAT2_Msg.DLC = 3;
	objt_CAN_TRNS_STAT2_Msg.StdId = 0x125;
	objt_CAN_TRNS_STAT2_Msg.IDE = CAN_ID_STD;
	objt_CAN_VehCfg1_Msg.DLC = 8;
	objt_CAN_VehCfg1_Msg.StdId = 0x3E8;
	objt_CAN_VehCfg1_Msg.IDE = CAN_ID_STD;
	objt_CAN_VehCfg2_Msg.DLC = 8;
	objt_CAN_VehCfg2_Msg.StdId = 0x3E9;
	objt_CAN_VehCfg2_Msg.IDE = CAN_ID_STD;
	objt_CAN_VehCfg3_Msg.DLC = 8;
	objt_CAN_VehCfg3_Msg.StdId = 0x3EA;
	objt_CAN_VehCfg3_Msg.IDE = CAN_ID_STD;
	objt_CAN_VehCfg5_Msg.DLC = 8;
	objt_CAN_VehCfg5_Msg.StdId = 0x44A;
	objt_CAN_VehCfg5_Msg.IDE = CAN_ID_STD;
	objt_CAN_VehCfg6_Msg.DLC = 8;
	objt_CAN_VehCfg6_Msg.StdId = 0x44C;
	objt_CAN_VehCfg6_Msg.IDE = CAN_ID_STD;
	objt_CAN_VehCfg7_Msg.DLC = 8;
	objt_CAN_VehCfg7_Msg.StdId = 0x381;
	objt_CAN_VehCfg7_Msg.IDE = CAN_ID_STD;
	objt_CAN_VehCfg8_Msg.DLC = 8;
	objt_CAN_VehCfg8_Msg.StdId = 0x38A;
	objt_CAN_VehCfg8_Msg.IDE = CAN_ID_STD;
	objt_CAN_VehCfgCSM1_Msg.DLC = 8;
	objt_CAN_VehCfgCSM1_Msg.StdId = 0x3B3;
	objt_CAN_VehCfgCSM1_Msg.IDE = CAN_ID_STD;
	objt_CAN_VehCfgCSM2_Msg.DLC = 8;
	objt_CAN_VehCfgCSM2_Msg.StdId = 0x3B4;
	objt_CAN_VehCfgCSM2_Msg.IDE = CAN_ID_STD;
	objt_CAN_VIN_Msg.DLC = 8;
	objt_CAN_VIN_Msg.StdId = 0x3E0;
	objt_CAN_VIN_Msg.IDE = CAN_ID_STD;


for (int i = 0; i < 8; i++)
{
	objt_CAN_AMB_TEMP_DISP_Msg.Data[i] = m_abyte_AMB_TEMP_DISP_Msg[i];
	objt_CAN_APPL_ECU_IC_Msg.Data[i] = m_abyte_APPL_ECU_IC_Msg[i];
	objt_CAN_ASBS_1_Msg.Data[i] = m_abyte_ASBS_1_Msg[i];
	objt_CAN_BSM_A1_Msg.Data[i] = m_abyte_BSM_A1_Msg[i];
	objt_CAN_CBC_PT1_Msg.Data[i] = m_abyte_CBC_PT1_Msg[i];
	objt_CAN_CBC_PT10_Msg.Data[i] = m_abyte_CBC_PT10_Msg[i];
	objt_CAN_CBC_PT2_Msg.Data[i] = m_abyte_CBC_PT2_Msg[i];
	objt_CAN_CBC_PT3_Msg.Data[i] = m_abyte_CBC_PT3_Msg[i];
	objt_CAN_CBC_PT4_Msg.Data[i] = m_abyte_CBC_PT4_Msg[i];
	objt_CAN_CBC_PT8_Msg.Data[i] = m_abyte_CBC_PT8_Msg[i];
	objt_CAN_CBC_PT9_Msg.Data[i] = m_abyte_CBC_PT9_Msg[i];
	objt_CAN_CFG_RQ_Msg.Data[i] = m_abyte_CFG_RQ_Msg[i];
	objt_CAN_Clock_Date_Msg.Data[i] = m_abyte_Clock_Date_Msg[i];
	objt_CAN_COMPASS_A1_Msg.Data[i] = m_abyte_COMPASS_A1_Msg[i];
	objt_CAN_D_RQ_IC_Msg.Data[i] = m_abyte_D_RQ_IC_Msg[i];
	objt_CAN_DG_RQ_GLOBAL_UDS_Msg.Data[i] = m_abyte_DG_RQ_GLOBAL_UDS_Msg[i];
	objt_CAN_DIRECT_INFO_Msg.Data[i] = m_abyte_DIRECT_INFO_Msg[i];
	objt_CAN_DTCM_A1_Msg.Data[i] = m_abyte_DTCM_A1_Msg[i];
	objt_CAN_DTCM_B1_Msg.Data[i] = m_abyte_DTCM_B1_Msg[i];
	objt_CAN_ECM_A1_Msg.Data[i] = m_abyte_ECM_A1_Msg[i];
	objt_CAN_ECM_A3_Msg.Data[i] = m_abyte_ECM_A3_Msg[i];
	objt_CAN_ECM_B11_Msg.Data[i] = m_abyte_ECM_B11_Msg[i];
	objt_CAN_ECM_B2_Msg.Data[i] = m_abyte_ECM_B2_Msg[i];
	objt_CAN_ECM_B3_Msg.Data[i] = m_abyte_ECM_B3_Msg[i];
	objt_CAN_ECM_B5_Msg.Data[i] = m_abyte_ECM_B5_Msg[i];
	objt_CAN_ECM_B9_Msg.Data[i] = m_abyte_ECM_B9_Msg[i];
	objt_CAN_ECM_CRUISE_MAP_Msg.Data[i] = m_abyte_ECM_CRUISE_MAP_Msg[i];
	objt_CAN_ECM_DIESEL_Msg.Data[i] = m_abyte_ECM_DIESEL_Msg[i];
	objt_CAN_ECM_INDICATORS_Msg.Data[i] = m_abyte_ECM_INDICATORS_Msg[i];
	objt_CAN_EcuCfg10_Msg.Data[i] = m_abyte_EcuCfg10_Msg[i];
	objt_CAN_EcuCfg12_Msg.Data[i] = m_abyte_EcuCfg12_Msg[i];
	objt_CAN_EcuCfg4_Msg.Data[i] = m_abyte_EcuCfg4_Msg[i];
	objt_CAN_ENG_CFG_Msg.Data[i] = m_abyte_ENG_CFG_Msg[i];
	objt_CAN_EPS_A1_Msg.Data[i] = m_abyte_EPS_A1_Msg[i];
	objt_CAN_ESP_A1_Msg.Data[i] = m_abyte_ESP_A1_Msg[i];
	objt_CAN_ESP_A8_Msg.Data[i] = m_abyte_ESP_A8_Msg[i];
	objt_CAN_ESP_B1_Msg.Data[i] = m_abyte_ESP_B1_Msg[i];
	objt_CAN_GW_I_C1_Msg.Data[i] = m_abyte_GW_I_C1_Msg[i];
	objt_CAN_GW_LIN_I_C2_Msg.Data[i] = m_abyte_GW_LIN_I_C2_Msg[i];
	objt_CAN_GW_LIN_I_C4_Msg.Data[i] = m_abyte_GW_LIN_I_C4_Msg[i];
	objt_CAN_HCP_C1_Msg.Data[i] = m_abyte_HCP_C1_Msg[i];
	objt_CAN_HCP_DISP_Msg.Data[i] = m_abyte_HCP_DISP_Msg[i];
	objt_CAN_NAV_DATA_Msg.Data[i] = m_abyte_NAV_DATA_Msg[i];
	objt_CAN_NET_CFG_INT_Msg.Data[i] = m_abyte_NET_CFG_INT_Msg[i];
	objt_CAN_NET_CFG_PT_Msg.Data[i] = m_abyte_NET_CFG_PT_Msg[i];
	objt_CAN_NM_CBC_Msg.Data[i] = m_abyte_NM_CBC_Msg[i];
	objt_CAN_NM_EPS_Msg.Data[i] = m_abyte_NM_EPS_Msg[i];
	objt_CAN_NM_ESC_Msg.Data[i] = m_abyte_NM_ESC_Msg[i];
	objt_CAN_NM_ESL_Msg.Data[i] = m_abyte_NM_ESL_Msg[i];
	objt_CAN_NM_HCP_Msg.Data[i] = m_abyte_NM_HCP_Msg[i];
	objt_CAN_NM_IC_Msg.Data[i] = m_abyte_NM_IC_Msg[i];
	objt_CAN_NM_RF_HUB_Msg.Data[i] = m_abyte_NM_RF_HUB_Msg[i];
	objt_CAN_NM_SBWM_Msg.Data[i] = m_abyte_NM_SBWM_Msg[i];
	objt_CAN_NM_SCCM_Msg.Data[i] = m_abyte_NM_SCCM_Msg[i];
	objt_CAN_NM_TPM_Msg.Data[i] = m_abyte_NM_TPM_Msg[i];
	objt_CAN_ORC_A1_Msg.Data[i] = m_abyte_ORC_A1_Msg[i];
	objt_CAN_ORC_A3_Msg.Data[i] = m_abyte_ORC_A3_Msg[i];
	objt_CAN_PN14_STAT_Msg.Data[i] = m_abyte_PN14_STAT_Msg[i];
	objt_CAN_PTS_1_Msg.Data[i] = m_abyte_PTS_1_Msg[i];
	objt_CAN_PTS_2_Msg.Data[i] = m_abyte_PTS_2_Msg[i];
	objt_CAN_RFHUB_A2_Msg.Data[i] = m_abyte_RFHUB_A2_Msg[i];
	objt_CAN_RFHUB_A3_Msg.Data[i] = m_abyte_RFHUB_A3_Msg[i];
	objt_CAN_SBW_ROT1_Msg.Data[i] = m_abyte_SBW_ROT1_Msg[i];
	objt_CAN_SCCM_STW_ANGL_STAT_Msg.Data[i] = m_abyte_SCCM_STW_ANGL_STAT_Msg[i];
	objt_CAN_STATUS_C_PTS_Msg.Data[i] = m_abyte_STATUS_C_PTS_Msg[i];
	objt_CAN_SWS_8_Msg.Data[i] = m_abyte_SWS_8_Msg[i];
	objt_CAN_TCM_A7_Msg.Data[i] = m_abyte_TCM_A7_Msg[i];
	objt_CAN_TGW_DATA_IC_Msg.Data[i] = m_abyte_TGW_DATA_IC_Msg[i];
	objt_CAN_TPM_A1_Msg.Data[i] = m_abyte_TPM_A1_Msg[i];
	objt_CAN_TRNS_SPD_Msg.Data[i] = m_abyte_TRNS_SPD_Msg[i];
	objt_CAN_TRNS_STAT_Msg.Data[i] = m_abyte_TRNS_STAT_Msg[i];
	objt_CAN_TRNS_STAT2_Msg.Data[i] = m_abyte_TRNS_STAT2_Msg[i];
	objt_CAN_VehCfg1_Msg.Data[i] = m_abyte_VehCfg1_Msg[i];
	objt_CAN_VehCfg2_Msg.Data[i] = m_abyte_VehCfg2_Msg[i];
	objt_CAN_VehCfg3_Msg.Data[i] = m_abyte_VehCfg3_Msg[i];
	objt_CAN_VehCfg5_Msg.Data[i] = m_abyte_VehCfg5_Msg[i];
	objt_CAN_VehCfg6_Msg.Data[i] = m_abyte_VehCfg6_Msg[i];
	objt_CAN_VehCfg7_Msg.Data[i] = m_abyte_VehCfg7_Msg[i];
	objt_CAN_VehCfg8_Msg.Data[i] = m_abyte_VehCfg8_Msg[i];
	objt_CAN_VehCfgCSM1_Msg.Data[i] = m_abyte_VehCfgCSM1_Msg[i];
	objt_CAN_VehCfgCSM2_Msg.Data[i] = m_abyte_VehCfgCSM2_Msg[i];
	objt_CAN_VIN_Msg.Data[i] = m_abyte_VIN_Msg[i];
}
}

osTimerId timer;
WM_HWIN openPanels(void)
{
	initArrays();
	/* Create Touch screen Timer */
	osTimerDef(periodic_CAN_Timer, periodic_CAN_Timer);
	timer = osTimerCreate(osTimer(periodic_CAN_Timer), osTimerPeriodic, NULL);
	/* Start the TS Timer */
	osTimerStart(timer, 1);
	hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
	return hWin;
}
