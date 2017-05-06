#ifndef __K_STORAGE_H
#define __K_STORAGE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"   

   /* Exported constants --------------------------------------------------------*/
#define FILE_LIST_DEPDTH                     	20
#define FILE_NAME_SIZE      	             	64
#define FILE_EXT_SIZE						  	4
#define FILE_ATTRIB_SIZE					 	10
#define FILE_LENGTH_SIZE					  	8
#define PATH_MAX_SIZE						 	60
#define FILE_MAX_LEVEL                        	3
#define FILE_MAX_EXT_SIZE                     	3
   
#define FILETYPE_DIR                             0
#define FILETYPE_FILE                            1

#define NUM_DISK_UNITS                           2
#define USB_DISK_UNIT                            0
#define SDCARD_DISK_UNIT						 1
/* Exported types ------------------------------------------------------------*/
typedef enum
{
  USBDISK_DISCONNECTION_EVENT = 1,  
  USBDISK_CONNECTION_EVENT,  
}   
STORAGE_EventTypeDef;

typedef enum {
	FILE_NOT_FOUND = 0,
	NO_DISKS,
	FILE_FOUND
}DIALOG_FILE;


typedef struct _FILELIST_LineTypeDef {
  uint8_t               type;
  uint8_t               name[FILE_NAME_SIZE];

} FILELIST_LineTypeDef;

typedef struct _FILELIST_FileTypeDef {
  FILELIST_LineTypeDef  file[FILE_LIST_DEPDTH] ;
  uint16_t              ptr;
} FILELIST_FileTypeDef;

typedef struct _DriveDescriptor {
	FATFS fs;
	char path[4];
} DriveDescriptor;

typedef struct _fileInfo {
	char root[PATH_MAX_SIZE];
	char name[FILE_NAME_SIZE];
	char ext[FILE_EXT_SIZE];
	char attrib[FILE_ATTRIB_SIZE];
	char size[FILE_LENGTH_SIZE];
	int Cmd;
} FileInfo;

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void     StorageInit(void);
uint8_t  StorageGetStatus (uint8_t unit);
uint32_t StorageGetCapacity (uint8_t unit);
uint32_t StorageGetFree (uint8_t unit);
int      StorageGetFileInfo(FileInfo *file);
const char* StorageGetPath(uint8_t unit);

#ifdef __cplusplus
}
#endif

#endif /*__K_STORAGE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
