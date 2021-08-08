#include "client.h"

int pipe_p2c[2], pipe_c2p[2];
int *quantities;
char name[100] = "";

void equipMain(int client_sock, int max_drink)
{
	/*Reading information from data base 
	and display then allows the user to choose bewtween options*/

    char stri[BUFF_SIZE];	
    int x;

	while (1)
	{
		//Receiving supplies for each drinks from the server
		for (int i = 0; i < max_drink; i++){	
			recv(client_sock, stri, BUFF_SIZE, 0);
			if(atoi(stri) == NO_DELIVER){
				printf("Nothing to deliver\n");
			}else{
				printf("Commodity delivery is in progress. Please wait 10s...\n");
				//Read the quantities in VM from the server
				sscanf(stri, "%d", &x);
				readCache(quantities);
				quantities[x] += 10;
				writeCache(quantities);
				// sleep(3);
			}
            // sleep(10);
		}

		menu_home(quantities);

        /*Reading the user's choice
		    c: the user's choice, read from stdin*/
        int c;
        char c_str[10], c_char, i;
		fseek(stdin, 0, SEEK_END);
		readCache(quantities);
		scanf("%s", &c_str);
		c_char = c_str[0];
		c = (int)(c_char - '0');
		(c < 0) ? c : -1;
		switch (c_char)
		{
			case '0':
				menu_bye();
				send(client_sock, "shut_down", strlen("shut_down") + 1, 0);
				kill(0, SIGKILL);
				exit(0);
			case '1':
			case '2':
			case '3':
				if (quantities[c - 1] <= 0)
				{
					printf("\nSold out!\n\n");
					continue;
				}
				else
				{
					quantities[c - 1] -= 1;
					writeCache(quantities);
					break;
				}
			default:
				printf("\nWrong option!\n\n");
				continue;
		}

		char buff[BUFF_SIZE];
		sprintf(buff, "%d", c - 1);

		pipe(pipe_p2c);
		pipe(pipe_c2p);

		pid_t pid = 0;

		switch (pid = fork())
		{
			case -1:
				perror("processGenerate fork");
				exit(1);
			case 0:
				commoditySales(client_sock);
				exit(0);
			default:
				close(pipe_p2c[0]);
				write(pipe_p2c[1], buff, strlen(buff) + 1);
				close(pipe_p2c[1]);

				wait(NULL);

				char tmp[BUFF_SIZE];

				close(pipe_c2p[1]);
				read(pipe_c2p[0], tmp, BUFF_SIZE);
				close(pipe_c2p[0]);

				printf("\n%s\n\n", tmp);
		}
	}

}

void commoditySales(int client_sock)
{
	char recv_str[BUFF_SIZE];

	close(pipe_p2c[1]);
	read(pipe_p2c[0], recv_str, BUFF_SIZE);
	close(pipe_p2c[0]);

	char send_str[BUFF_SIZE] = "Thank you for chosing us, your purchase is being processed, please wait for a few seconds...\n";

	close(pipe_c2p[0]);
	write(pipe_c2p[1], send_str, strlen(send_str) + 1);
	close(pipe_c2p[1]);

	send(client_sock, recv_str, strlen(recv_str) + 1, 0);

}

void writeCache(int *quantities)
{
	char a[100] = "cache_";
	FILE *f = fopen(strcat(a, name), "w");
	int i = 0;
	while (i < 3)
	{
		fprintf(f, "%d\n", quantities[i]);
		i++;
	}
	fclose(f);
}

void readCache(int *quantities)
{
	char a[100] = "cache_";
	FILE *f = fopen(strcat(a, name), "r");
	int i = 0;
	while (i < 3)
	{
		fscanf(f, "%d\n", quantities + i);
		i++;
	}
	fclose(f);
}

int main(int argc, char *argv[])
{
	char *quantities_str;
	int server_port = 0;
	char server_ip[16] = "";

	int client_sock;
	struct sockaddr_in server_addr;
	char max_drink_str[BUFF_SIZE], is_valid_client[BUFF_SIZE];
	int max_drink;

    //Global variable: quantities. String form: quantities_str
    quantities = (int *)malloc(3 * sizeof(int));
    quantities_str = (char *)malloc(BUFF_SIZE);
	
    //Check validity of input
	va_cli(argc, argv, server_ip, &server_port, &name);

	//Construct socket
	client_sock = socket(AF_INET, SOCK_STREAM, 0);

	//Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = inet_addr(server_ip);

	//Request to connect server
	if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
	{
		printf("\nError! Cannot connect to sever!\nClient exits imediately!\n");
		return 0;
	}

	if (send(client_sock, name, 100, 0) < 0)
	{
		printf("Cannot send machine's name\nClient exits imediately!\n");
		return 0;
	}
	
	//Server answer if the client name (VM name) is valid
	int byte_receive = recv(client_sock, is_valid_client, BUFF_SIZE-1, 0);
	is_valid_client[byte_receive] = '\0';
	if (strcmp(is_valid_client, "DECLINE") == 0){
		printf("Wrong client name or machine is in use!\n");
		menu_bye();
		kill(0, SIGKILL);
		exit(0);
	}else{
		printf("Client name is valid.\n");
	}

	recv(client_sock, quantities_str, BUFF_SIZE, 0);
	sscanf(quantities_str, "%d %d %d", quantities, quantities + 1, quantities + 2);
	writeCache(quantities);
	printf("Received quantities from server %d %d %d\n", quantities[0], quantities[1], quantities[2]);
	
	recv(client_sock, max_drink_str, BUFF_SIZE, 0);
	max_drink = atoi(max_drink_str);
	printf("Number of drinks: %d\n", max_drink);
	
	equipMain(client_sock, max_drink);
	close(client_sock);
	return 0;
}
