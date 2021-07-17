#include "extra.h"
void va_cli(int argc, char *argv[], char *server_ip, int *server_port, char *name)
{
	if (argc == 4)
	{
		struct sockaddr_in tmp_addr;
		if (inet_pton(AF_INET, argv[1], &(tmp_addr.sin_addr)) == 0)
		{
			printf("IP Address is invalid\n");
			exit(0);
		}
		else
		{
			strcpy(server_ip, argv[1]);
		}

		int i;
		char *port_str = argv[2];
		for (i = 0; port_str[i] != '\0'; i++)
		{
			if (!isdigit(port_str[i]))
			{
				printf("Port is invalid\n!");
				exit(0);
			}
		}

		if (port_str[i] == '\0')
		{
			*server_port = atoi(port_str);
		}
		strcpy(name, argv[3]);
	}
	else
	{
		printf("ERROR!!! Syntax like: ./client ip port machine_name\n");
		exit(0);
	}
}

/*
 * check argument valid in server 
 */

int va_ser(int argc, char* port_str)
{
	if (argc == 2)
	{
		for (int i = 0; port_str[i] != '\0'; i++)
		{
			if (!isdigit(port_str[i]))
			{
				printf("Invalid port: Port must be a number!\n");
				return -1;
			}
		}

		return atoi(port_str);

	}
	else
	{
		printf("ERROR!!! Wrong syntax. Please follow this syntax: \'./s <port_number>\'\n");
		return -1;
	}
}

/*
 * handle sigchild
 */

void sig_child(int signo)
{
	pid_t pid;
	int stat;
	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
	{
		printf("Child %d terminated\n", pid);
	}
}

void throwMallocException()
{ // hope it will work
	fprintf(stderr, "failed to locate !\n");
	exit(-1);
}
