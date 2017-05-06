#include "canData.h"
#include "stm32f7xx_hal.h"

#define MAX_LINE 100

int parseLogInternal(FileInfo file, CanNode** head)
{
	FIL logFile;
	int lines = 0;
	int wrongLines = 0;
	char fileName[PATH_MAX_SIZE];
	float start_timeStamp = -1;
	CanNode* currentIndex = NULL;

	memset(fileName, 0, PATH_MAX_SIZE);
	sprintf(fileName, "%s%s.%s", file.root, file.name, file.ext);

	char buffer[MAX_LINE];
	if (f_open(&logFile, (TCHAR const*)fileName, FA_READ) == FR_OK) {
		while (!f_eof(&logFile)) {
			memset(buffer,0,MAX_LINE);
			f_gets(buffer, MAX_LINE, &logFile);
			if (buffer[0] < '0' || buffer[0] > '9') {
				wrongLines++;
				continue;
			}

			long timeStamp = 0;
			int decimal = 0;

			CanNode *newNode = (CanNode*)pvPortMalloc(sizeof(CanNode));
			newNode->message.IDE = CAN_ID_STD;

			char bufferData[24];
			memset(bufferData, 0, 24);

			int res = sscanf(buffer, "%d.%3d%*d %*d %x %*c%*c %*c %d %[^\n]s\n",
					(int*)&timeStamp, (int*)&decimal,
					(int*)&newNode->message.StdId,
					(int*)&newNode->message.DLC,
					bufferData);

			if (res != 5) {
				vPortFree(newNode);
				continue;
			}

			timeStamp *= 1000;
			timeStamp += decimal;

			if(start_timeStamp < 0)
				start_timeStamp = timeStamp;

			newNode->delay = timeStamp - start_timeStamp;

			for (int i = 0; i < 8; ++i) {
				if(i < newNode->message.DLC)
					sscanf(	&bufferData[i*3], "%2x ",
							(unsigned int*)&newNode->message.Data[i]);
				else
					newNode->message.Data[i] = 0;
			}

			if (*head == NULL) {
				*head = newNode;
				currentIndex = *head;
			} else {
				currentIndex->next = newNode;
				currentIndex = currentIndex->next;
			}

			currentIndex->next = NULL;

			lines++;
		}

		f_close(&logFile);
		return lines;
	}
	return 0;
}

int parseLog(FileInfo file, CanNode** head)
{
	return parseLogInternal(file,head);
}

void cleanUpCANData(CanNode* head)
{
	CanNode* index = NULL;
	while (head != NULL) {
		index = head;
		head = head->next;
		vPortFree(index);
	}
}
