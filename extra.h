#ifndef EXTRA_H
#define EXTRA_H
#include "lib.h"
#include "menu.h"
#include "string.h"

#define CLIENTS_INFO_FILENAME "clients.txt"

void va_cli(int argc, char *argv[], char *server_ip, int *server_port, char *name);

//validate the input to start the server
int va_ser(int argc, char* port_str);



void sig_child(int signo);

void throwMallocException();
/*
 * check argument valid in client 
 */

#endif
