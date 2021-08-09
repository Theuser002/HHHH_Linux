#include "server.h"
#include "string.h"

int max_drink;		 //The number of types of drink
drink all_drink[30]; //Array that stores information of all types of drinks

int clientName2id(char *name)
{
	//Return the id of the Vending Machine (VM) that the client connect to
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (strcmp(name, client_set[i].name) == 0)
		{
			return i;
		}
	}
	return -1;
}

void checkForDelivery(client_info *clt, int client_sock)
{
	//Constantly check if the drink in the Vending machine is lower than allowed, then decide to deliver supply
	int i = clientName2id(clt->name);
	int j;
	char *brand;
	int quantity;

	while (1)
	{
		for (j = 0; j < max_drink; j++)
		{
			brand = no2brand(all_drink, max_drink, j);
			if (!brand)
			{
				brand = "Undefined";
			}

			quantity = equipInfoAccess(1, j, &client_set[i]);
			printf("Checking for delivery of %s\n", clt->name);
			if (quantity < MIN_QUANTITY)
			{
				printf("Quantity of the drink %s is %d, which is less than allowed, delivering supply to %s\n", brand, quantity, clt->name);
				deliveryMng(client_sock, j);
				equipInfoAccess(2, j, &client_set[i]);
				sleep(10);
			}
			else
			{
				printf("Quantity of the drink %s is %d, which is fine, not delivering more of this drink to %s\n", brand, quantity, clt->name);
				deliveryMng(client_sock, NO_DELIVER);
				sleep(3);
			}
			// sleep(3);
		}
	}
}

client_info *findClient(char name[])
{
	//Return a client_info object respective to the correct client, client name = VM name
	int client_id = clientName2id(name);
	int i;

	printf("\n\n\n---------------New client access with the name: %s\n\n", name);
	for (i = 0; i < 3; i++)
	{
		if (strcmp(client_set[i].name, name) == 0)
		{
			return &client_set[i];
		}
	}
	
	//Vending machine name (== client name) not found!
	return NULL;
}

void salesMng(int client_sock, client_info *clt)
{
	//Receive the option from client and process it
	printf("Sales Management function is running\n\n");
	char recv_data[BUFF_SIZE];
	int client_id, drink_id;

	int byte_receive = recv(client_sock, recv_data, BUFF_SIZE - 1, 0);
	recv_data[byte_receive] = '\0';
	client_id = clientName2id(clt->name);

	//Shutdown case (client chooses to exit)
	if (strcmp(recv_data, "shut_down") == 0)
	{
		exit(0);
	}
	printf("\nOption received: %s\n", recv_data);
	sscanf(recv_data, "%d", &drink_id);
	equipInfoAccess(0, drink_id, clt);
}

int equipInfoAccess(int action, int num, client_info *clt)
{
	/* 
	Interact with the database
    	0: Write purchase history
		1: Read quantites of drink from the database
     	2: Write delivery history and update the database (inventory) after delivery
   */

	time_t t;
	struct tm *info;
	int client_id, drink_id, i = 0;
	int *quantities;

	time(&t);
	info = localtime(&t);
	client_id = clientName2id(clt->name);
	quantities = readInventoryInfo(max_drink);
	drink_id = client_id * max_drink + num;

	if (action == 0)
	{
		printf("Writing purchase history...\n");
		FILE *f = fopen(salesHistory, "a");
		if (f == NULL)
		{
			printf("Cannot open file sales history!\n");
			exit(-1);
		}
		fprintf(f, "%s: %s Purchased: %s\n", clt->name, asctime(info), no2brand(all_drink, max_drink, num));
		fclose(f);

		printf("Quantity of the requested drink (before bought): %d\n", quantities[drink_id]);

		quantities[client_id * max_drink + num] -= 1;
		printf("Current quantity of the requested drink (after bought): %d\n", quantities[drink_id]);

		writeInventoryInfo(all_drink, max_drink, quantities);
		return -1;
	}
	else if (action == 1)
	{
		printf("Reading quantities in the database...\n");
		return quantities[drink_id];
	}
	else if (action == 2)
	{
		FILE *f = fopen(salesHistory, "a");
		if (f == NULL)
		{
			printf("Cannot open file sales history\n");
			exit(-1);
		}
		fprintf(f, "Machine name: %s\n", clt->name);
		fprintf(f, "%s: %s Delivered: %s + 10 \n",
				clt->name,
				asctime(info),
				no2brand(all_drink, max_drink, num));
		fclose(f);

		quantities[drink_id] += 10;
		printf("Writing delivery history and updating database... %d\n", client_id * MAX_CLIENTS + num);

		writeInventoryInfo(all_drink, max_drink, quantities);
		return -1;
	}
	else
	{
		exit(-1);
	}
}

void deliveryMng(int client_sock, int j)
{
	char str[BUFF_SIZE];
	sprintf(str, "%d", j);
	send(client_sock, str, BUFF_SIZE, 0);
	printf("Delivering... drink_id is: %s\n\n", str);
}

int main(int argc, char *argv[])
{
	char *port_str = argv[1];
	int port = va_ser(argc, port_str);

	if (port == -1)
	{
		exit(0);
	}
	else
	{
		char *quantities = (char *)malloc(BUFF_SIZE);
		printf("Vending machine list:\n1.VM1\n2.VM2\n3.VM3\n");

		struct sockaddr_in server, client;
		char recv_data[BUFF_SIZE];
		int server_sock, client_sock, bytes_sent, bytes_received;
		int sin_size = sizeof(struct sockaddr_in);
		pid_t pid;
		char max_drink_str[BUFF_SIZE];

		//Store drinks info to variable
		readDrinkInfo(all_drink, &max_drink);

		//Construct a TCP socket to listen to connection request from client
		if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
			perror("\nError: ");
			return 0;
		}

		//Bind address to socket
		bzero(&server, sizeof(server));
		server.sin_family = AF_INET;
		server.sin_port = htons(port);
		server.sin_addr.s_addr = htonl(INADDR_ANY);

		if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
		{
			perror("\nError: ");
			return 0;
		}
		
		//Listen request from client
		if (listen(server_sock, BACKLOG) == -1)
		{
			perror("\nError: ");
			return 0;
		}
		//Communicate with client
		while (1)
		{
			//Waiting for connection
			if ((client_sock = accept(server_sock, (struct sockaddr *)&client, &sin_size)) == -1)
			{
				if (errno == EINTR){ continue; }
				else{ perror("\nError: "); }
			}
			
			if ((pid = fork()) == 0)
			{
				close(server_sock);
				char client_name[100] = "";

				//Get socket number and client name via network
				if (recv(client_sock, client_name, 100, 0) < 0)
				{
					printf("Cannot recv machine's name!");
					return 0;
				}

				client_info *recv_client = findClient(client_name);
				//Check client's validity
				if (recv_client == NULL)
				{
					printf("Invalid client (vending machine) name!\n");
					send(client_sock, "DECLINE", strlen("DECLINE") + 1, 0);
					exit(0);
				}
				else
				{
					send(client_sock, "APRROVE", strlen("APPROVE") + 1, 0);
				}

				printf("You got a connection from %s. - Vending machine's name: %s\n",
					   inet_ntoa(client.sin_addr), client_name);

				//Read drinks quantity from the database and send to client
				printf("Sending drinks quantity to the client...\n"); 
				sprintf(quantities, "%d %d %d",
						equipInfoAccess(1, 0, recv_client),
						equipInfoAccess(1, 1, recv_client),
						equipInfoAccess(1, 2, recv_client));
				send(client_sock, quantities, BUFF_SIZE, 0);

				//Also send the max_drink (number of drink types) to client
				sprintf(max_drink_str, "%d", max_drink);
				send(client_sock, max_drink_str, BUFF_SIZE, 0);

				//Fork a child process to check if the VM needs supply
				if (fork() == 0){ checkForDelivery(recv_client, client_sock); }

				//Process client's choices
				while (1){ salesMng(client_sock, recv_client); }

				//exit the child process
				exit(0);
			}

			//handing zombie state (slide cua co Linh mon Network Programming - 05-MultithreadTCPServer phan Forking Server)
			signal(SIGCHLD, sig_child);
			close(client_sock);
		}
		close(server_sock);
	}
	return 0;
}