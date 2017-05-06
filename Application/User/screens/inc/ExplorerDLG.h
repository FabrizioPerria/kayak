#ifndef APPLICATION_USER_SCREENS_INC_EXPLORERDLG_H_
#define APPLICATION_USER_SCREENS_INC_EXPLORERDLG_H_

#include "DIALOG.h"
#include "LISTVIEW.h"
#include "storage.h"

#define RET_OK			0
#define RET_IS_FILE 	1
#define RET_IS_DIR		2
#define RET_FAIL		3

WM_HWIN CreateExplorer(FileInfo *fileInfo);

#endif /* APPLICATION_USER_SCREENS_INC_EXPLORERDLG_H_ */
