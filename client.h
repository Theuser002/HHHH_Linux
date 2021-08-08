#include "extra.h"
#include "lib.h"
#include "stdlib.h"

typedef struct mess_buffer
{
	long mesg_type;
	char mesg_text[BUFF_SIZE];
} message;

void equipMain(int client_sock, int max_drink);
void commoditySales(int client_sock);
void writeCache(int *figures);
void readCache(int *figures);
