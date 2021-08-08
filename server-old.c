#include "server.h"
#include "string.h"

int max_drink;
drink all_drink[20];
int connecting[MAX_CLIENTS];

int clientName2id(char *name){
  for (int i = 0; i < MAX_CLIENTS; i++){
    if(strcmp(name, client_set[i].name) == 0){
      return i;
    }
  }
  return -1;
}

void checkForDelivery(client_info *clt, int client_sock)
{
	int i = clientName2id(clt->name);
	int j;

	while (1)
	{
		for (j = 0; j < max_drink; j++)
		{
			char* brand = no2brand(all_drink, max_drink, j);

			if(!brand){
				brand = "Undefined";
			}

			int quantity = equipInfoAccess(1, j, &client_set[i]);
			printf("Checking for delivery of %s\n", clt->name);
			if (quantity < MIN_QUANTITY)
			{
				printf("Quantity of the drink %s is %d, which is less than allowed, delivering supply to %s\n", brand, quantity, clt->name);
				// printf("TEST: j=%d\n", j);
				deliveryMng(client_sock, j);
				equipInfoAccess(2, j, &client_set[i]);
			}else{
				printf("Quantity of the drink %s is %d, which is fine, not delivering more of this drink to %s\n", brand, quantity, clt->name);
				deliveryMng(client_sock, NO_DELIVER);
			}
		}
		sleep(3);
	}
}

int main(int argc, char *argv[])
{

	char *quantities = (char *)malloc(BUFF_SIZE);

	char *port_str = argv[1];
	int port = va_ser(argc, port_str);

	if (port == -1)
	{
		exit(0);
	}

	printf("Vending machine list:\n1.VM1\n2.VM2\n3.VM3\n");

	int server_sock, client_sock;
	char recv_data[BUFF_SIZE];
	int bytes_sent, bytes_received;

	struct sockaddr_in server;
	struct sockaddr_in client;

	int sin_size;
	pid_t pid;

	char max_drink_str[BUFF_SIZE];

	//Initilize VM connection status:
	for(int i=0; i < MAX_CLIENTS; i++){
		connecting[i] = 0;
	}

	readDrinkInfo(all_drink, &max_drink); 
	//read tu file csv vao va luu thong tin cua tat ca cac loai do uong vao all_drink, so luong cac loai do uong co trong kho vao max_drink

	// Construct a TCP socket to listen connection request
	if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("\nError: ");
		return 0;
	}
	// Bind address to socket
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		perror("\nError: ");
		return 0;
	}

	// Listen request from client
	if (listen(server_sock, BACKLOG) == -1)
	{
		perror("\nError: ");
		return 0;
	}

	// Communicate with client
	while (1)
	{

		sin_size = sizeof(struct sockaddr_in);
		client_sock = accept(server_sock, (struct sockaddr *)&client, &sin_size);
		if (client_sock == -1)
		{
			if (errno == EINTR){
				//handle ctr+C exit here
				// printf("Interrupted!");
				continue;
			}		
			else{
				perror("\nError: ");
			}		
		}

		if ((pid = fork()) == 0)
		{
			// printf("TEST: Client sock: %d - Server sock: %d\n",client_sock, server_sock);
			printf("TEST: Forked a new server!\n");
			printf("TEST: Close server_sock return: %d\n", close(server_sock));

			char client_name[100] = "";

			//Getting socket number and client name via network
			if (recv(client_sock, client_name, 100, 0) < 0)
			{
				printf("Cannot recv machine's name!");
				return 0;
			}
			client_info *recv_client = findClient(client_name);
			// printf("TEST: Recv client: %s\n", recv_client->name);

			if(recv_client == NULL){
				printf("Wrong client name or machine is in use!\n");
				send(client_sock, "DECLINE", strlen("DECLINE") + 1, 0);
				exit(0);
			}else{
				send(client_sock, "APRROVE", strlen("APPROVE") + 1, 0);
			}

			printf("You got a connection from %s. - Vending machine's name: %s\n",
				   inet_ntoa(client.sin_addr), client_name);


			printf("Quantity read from the database: %d %d %d\n",
				   equipInfoAccess(1, 0, recv_client),
				   equipInfoAccess(1, 1, recv_client),
				   equipInfoAccess(1, 2, recv_client));
			printf("Sending read quantity to the client...\n");

			sprintf(quantities, "%d %d %d",
					equipInfoAccess(1, 0, recv_client),
					equipInfoAccess(1, 1, recv_client),
					equipInfoAccess(1, 2, recv_client));
			send(client_sock, quantities, BUFF_SIZE, 0);

			sprintf(max_drink_str, "%d", max_drink);
			send(client_sock, max_drink_str, BUFF_SIZE, 0);

			// Fork a child process to check if the VM needs supply
			if (fork() == 0){
				checkForDelivery(recv_client, client_sock);
			}

			while (1){
				salesMng(client_sock, recv_client);
			}

			exit(0);
		}
		
		// handing zombie state (slide cua co Linh mon Network Programming - 05-MultithreadTCPServer phan Forking Server)
		signal(SIGCHLD, sig_child);
		close(client_sock);
	}
	close(server_sock);
	return 0;
}

client_info *findClient(char name[])
{
	//Set connection status
	int client_id = clientName2id(name);
	if(connecting[client_id] == 1){
		return NULL;
	}

	printf("Client access with the name: %s, status: %d\n", name, connecting[client_id]);
	connecting[client_id] = 1;
	int i;

	for (i = 0; i < 3; i++)
	{
		if (strcmp(client_set[i].name, name) == 0)
		{
			return &client_set[i];
		}
	}
	printf("Invalid client name received\n");
	return NULL;
}

void salesMng(int client_sock, client_info *clt)
{

	/* handle information from client */
	// printf("Sales Management function is running\n");
	char recv_data[BUFF_SIZE];
	int client_id, drink_id;
	int byte_receive = recv(client_sock,
							recv_data,
							BUFF_SIZE - 1, 0);
	recv_data[byte_receive] = '\0';
	client_id = clientName2id(clt->name);

	if (strcmp(recv_data, "shut_down") == 0){
		connecting[client_id] = 0;
		exit(0);
	}
	printf("\nReceive: %s\n", recv_data);
	sscanf(recv_data, "%d", &drink_id);
	equipInfoAccess(0, drink_id, clt);
}

int equipInfoAccess(int action, int num, client_info *clt)
{
	/* 
	Interacting with the database
     action = 0: write history for bought
     action = 1: read quantities in database
     action = 2: write history and update inventory after delivery
   */
  	// printf("TEST: Recv client: %s\n", clt->name);
	time_t t;
	struct tm *info;
	time(&t);
	info = localtime(&t);
	int client_id;
	int drink_id;
	client_id = clientName2id(clt->name);
	// printf("TEST: Recv client ID: %d\n", client_id);
	int *quantities = readInventoryInfo(max_drink);
	int i = 0;

	if (action == 0)
	{
		printf("Writing history for bought...\n");
		FILE *f = fopen(salesHistory, "a");
		if (f == NULL)
		{
			printf("Cannot open file sales history\n");
			exit(-1);
		}
		//cache[num] += 1;
		fprintf(f, "%s: %s Bought: %s\n",
				clt->name,
				asctime(info),
				no2brand(all_drink, max_drink, num));
		fclose(f);
		// updateInventoryInfo(quantities, cache, max_drink);
		drink_id = client_id * max_drink + num;
		// printf("Quantity of the requested drink (before bought): %d\n", quantities[drink_id]);
		quantities[client_id * max_drink + num] -= 1;
		// printf("Current quantity of the requested drink (after bought): %d\n", quantities[drink_id]);
		writeInventoryInfo(all_drink, max_drink, quantities);
		return -1;
	}

	else if (action == 1)
	{
		// printf("Reading quantities in the database...\n");
		return quantities[client_id * max_drink + num];
	}

	else if (action == 2)
	{
		//cache[num] -= 10;
		FILE *f = fopen(salesHistory, "a");
		if (f == NULL)
		{
			printf("Cannot open file saleshistory\n");
			exit(-1);
		}
		fprintf(f, "Machine name: %s\n", clt->name);
		fprintf(f, "%s: %s Delivered: %s + 10 \n",
				clt->name,
				asctime(info),
				no2brand(all_drink, max_drink, num));
		fclose(f);
		quantities[client_id * MAX_CLIENTS + num] += 10;
		printf("Writing history and updating database after delivery... %d\n", client_id * MAX_CLIENTS + num);
		writeInventoryInfo(all_drink, max_drink, quantities);
		return -1;
	}
	else
		exit(-1);
}

void deliveryMng(int client_sock, int j)
{
	// printf("TEST: Deliver Client sock: %d\n", client_sock);
	char str[BUFF_SIZE];
	sprintf(str, "%d", j);
	send(client_sock, str, BUFF_SIZE, 0);
	printf("Delivering...drink id is %s\n\n", str);
}
