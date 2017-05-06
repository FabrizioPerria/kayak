#include "storage.h"

static struct {
	U32 Mask;
	char c;
} _aAttrib[] = {
	{ AM_RDO, 'R' },
	{ AM_HID, 'H' },
	{ AM_SYS, 'S' },
	{ AM_DIR, 'D' },
	{ AM_ARC, 'A' },
};
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

DriveDescriptor drives[_VOLUMES];

FATFS USBDISK_FatFs;         /* File system object for USB disk logical drive */
FATFS SDCARD_FatFs;

USBH_HandleTypeDef  hUSB_Host;
osMessageQId StorageEvent;
DIR dir;
static char         acAttrib[FILE_ATTRIB_SIZE];
static char         acExt[FILE_EXT_SIZE];
#if _USE_LFN
static char lfn[_MAX_LFN];
#endif

static uint32_t StorageStatus[NUM_DISK_UNITS];
/* Private function prototypes -----------------------------------------------*/
static void StorageThread(void const * argument);
static void USBH_UserProcess  (USBH_HandleTypeDef *phost, uint8_t id);
static void _GetExt(char * pFile, char * pExt, int toCutName);
/* Private functions ---------------------------------------------------------*/


/**
	* @brief  Storage drives initialization
	* @param  None
	* @retval None
	*/
void StorageInit(void){
	/* Link the USB Host disk I/O driver */
	FATFS_LinkDriver(&USBH_Driver, drives[USB_DISK_UNIT].path);
	drives[USB_DISK_UNIT].fs = USBDISK_FatFs;

	/* Init Host Library */
	USBH_Init(&hUSB_Host, USBH_UserProcess, 0);

	/* Create USB background task */
	osThreadDef(STORAGE_Thread, StorageThread, osPriorityLow, 0, 512);
	osThreadCreate (osThread(STORAGE_Thread), NULL);

	/* Create Storage Message Queue */
	osMessageQDef(osqueue, 10, uint16_t);
	StorageEvent = osMessageCreate (osMessageQ(osqueue), NULL);

	/* Add Supported Class */
	USBH_RegisterClass(&hUSB_Host, USBH_MSC_CLASS);

	/* Start Host Process */
	USBH_Start(&hUSB_Host);

	/* Setup SD card */
	drives[SDCARD_DISK_UNIT].fs = SDCARD_FatFs;
	FATFS_LinkDriver(&SD_Driver, drives[SDCARD_DISK_UNIT].path);
	f_mount(&SDCARD_FatFs,drives[SDCARD_DISK_UNIT].path, 1);
	StorageStatus[SDCARD_DISK_UNIT] = BSP_SD_IsDetected();
}

static void StorageThread(void const * argument){
	osEvent event;

	while(1){
		event = osMessageGet( StorageEvent, osWaitForever );
		if( event.status == osEventMessage ){
			switch(event.value.v){
			case USBDISK_CONNECTION_EVENT:
				f_mount(&USBDISK_FatFs,drives[USB_DISK_UNIT].path,  0);
				StorageStatus[USB_DISK_UNIT] = 1;
				break;

			case USBDISK_DISCONNECTION_EVENT:
				f_mount(NULL, drives[USB_DISK_UNIT].path, 0);
				StorageStatus[USB_DISK_UNIT] = 0;
				break;
			}
		}
	}
}

const char* StorageGetPath(uint8_t unit){
	return drives[unit].path;
}

uint8_t StorageGetStatus (uint8_t unit){
	osDelay(20);
	if(unit == SDCARD_DISK_UNIT)
		StorageStatus[unit] = BSP_SD_IsDetected();
	return StorageStatus[unit];
}

uint32_t StorageGetCapacity (uint8_t unit){
	uint32_t   tot_sect = 0;
	FATFS *fs;

	fs = &(drives[unit].fs);
	tot_sect = (fs->n_fatent - 2) * fs->csize;
	return (tot_sect);
}

uint32_t StorageGetFree (uint8_t unit){
	uint32_t   fre_clust = 0;
	FATFS *fs ;
	FRESULT res = FR_INT_ERR;

	fs = &(drives[unit].fs);
	res = f_getfree(drives[unit].path, (DWORD *)&fre_clust, &fs);
	if(res == FR_OK){
		return (fre_clust * fs->csize);
	}

	return 0;
}

static void USBH_UserProcess (USBH_HandleTypeDef *phost, uint8_t id){
	switch (id){
	case HOST_USER_SELECT_CONFIGURATION:
	break;

	case HOST_USER_DISCONNECTION:
		osMessagePut ( StorageEvent, USBDISK_DISCONNECTION_EVENT, 0);
		break;

	case HOST_USER_CLASS_ACTIVE:
		osMessagePut ( StorageEvent, USBDISK_CONNECTION_EVENT, 0);
		break;
	}
}

static void _GetExt(char * pFile, char * pExt, int toCutName) {
	int Len;
	int i;
	int j;

	/* Search beginning of extension */
	Len = strlen(pFile);
	for (i = Len; i > 0; i--) {
		if (*(pFile + i) == '.') {
			if(toCutName){
				*(pFile + i) = '\0';     /* Cut extension from filename */
			}
			break;
		}
	}

	/* Copy extension */
	j = 0;
	while (*(pFile + ++i) != '\0') {
		*(pExt + j) = *(pFile + i);
		j++;
	}
	*(pExt + j) = '\0';          /* Set end of string */
}

int StorageGetFileInfo(FileInfo *file){
	char                c;
	int                 i;
	char               tmp[PATH_MAX_SIZE];
	FRESULT res = FR_INT_ERR;
	char *fn;
	FILINFO fno;

	fno.lfname = lfn;
	fno.lfsize = sizeof(lfn);

	switch (file->Cmd){
	case CHOOSEFILE_FINDFIRST:
		f_closedir(&dir);

		/* reformat path */
		memset(tmp, 0, PATH_MAX_SIZE);
		strcpy(tmp, file->root);

		for(i= PATH_MAX_SIZE; i > 0 ; i--){
			if(tmp[i] == '/'){
				tmp[i] = 0 ;
				break;
			}
		}

		res = f_opendir(&dir, tmp);
		if (res == FR_OK){
			res = f_readdir(&dir, &fno);
		}
		break;

	case CHOOSEFILE_FINDNEXT:
		res = f_readdir(&dir, &fno);
		break;
	}

	if (res == FR_OK){
		if(*fno.lfname == 0){
			strcpy(fno.lfname, fno.fname);
		}
		fn = fno.lfname;

		if(fn[0] == 0){
			//all the files have been listed...now show the list
			f_closedir(&dir);
			return FR_NO_FILE;
		}

		for (i = 0; i < GUI_COUNTOF(_aAttrib); i++){
			if (fno.fattrib & _aAttrib[i].Mask){
				c = _aAttrib[i].c;
			} else {
				c = '-';
			}
			acAttrib[i] = c;
		}

		if((fno.fattrib & AM_DIR) == AM_DIR){
			acExt[0] = 0;
		} else {
			_GetExt(fn, acExt, 1);
		}

		strcpy(file->attrib, acAttrib);
		strcpy(file->name, fn);
		strcpy(file->ext, acExt);
		sprintf(file->size, "%d", (int)fno.fsize);
	}
	return res;
}

