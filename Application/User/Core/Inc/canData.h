#ifndef __CAN_DATA
#define __CAN_DATA

typedef struct CanNode{
	CanTxMsgTypeDef message;
	unsigned int delay;
	struct CanNode *next;
} CanNode;

//return the number of messages detected in the log
int parseLog(FileInfo file, CanNode** head);

#endif
