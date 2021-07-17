#include "server.h"

int max_drink;//so luong cac loai do uong
drink all_drink[30]; //array chua thong tin cua tung loai do uong

void checkForDelivery(client_info *clt, int client_socket)
{
	int i = clientName2id(clt->name);
	int j;

	while (1)
	{
		for (j = 0; j < max_drink; j++)
		{
			int quantity = equipInfoAccess(1, j, &client_set[i]);
			printf("Checking for dilivery, machine index: %d\n", i);
			if (quantity < MIN_QUANTITY)
			{
				printf("Quantity of the drink is %d less than allowed, delivering supply to the vending machine VM%d\n", quantity, i+1);
				deliveryMng(client_socket, j);
				equipInfoAccess(2, j, &client_set[i]);
			}
		}
		sleep(10);
	}
}

client_info *findClient(char name[])
{
	printf("Client access with the name: %s\n", name);
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

int main (int argc, char *argv[]){
	//Validating port format
	char* port_str = argv[1];
	int port = va_ser(argc, port_str);

	if(port == -1){
		exit(0);
	}else{
		//Store information about drink into variables all_drink and max_drink
		readDrinkInfo(all_drink, &max_drink);

		//Creating a socket descriptor
		int server_socket = socket(AF_INET, SOCK_STREAM, 0);
			//protocol faminly: AF_INET      IPv4 Internet protocol
			//socket type: SOCK_STREAM 	Provides sequenced, reliable, two-way, connection-based
			//  						byte streams.  An out-of-band data transmission mechanism
			//  						may be supported.

		if(server_socket == -1){
			perror("Error: ");
			return 0;
		}else{
			//declare the socket variable
			struct sockaddr_in server, client;

			bzero(&server, sizeof(server));
				// The bzero() function erases the data in the n bytes of the memory
				// starting at the location pointed to by s, by writing zeros (bytes
				// containing '\0') to that area.

			//Bind the internal port and external IP to the server socket 
			server.sin_family = AF_INET;
			server.sin_port = htons(port);
			server.sin_addr.s_addr = htonl(INADDR_ANY);
				// s_addr: the IP from that our socket can accept client_socket
				// INADDR_ANY: accept any IP

			//Bind the socket descriptor to the server socket socket variable
			if(bind(server_socket, (struct sockaddr*)&server, sizeof(server)) == -1){
				// The bind() function binds a unique local name to the socket with descriptor 
				// socket. After calling socket(), a descriptor does not have a name associated 
				// with it. However, it does belong to a particular address family as specified 
				// when socket() is called. The exact format of a name depends on the address family.
				perror("Error: ");
				return 0;
			}

			if (listen(server_socket, BACKLOG) == -1){
				// The backlog argument defines the maximum length to which the
				// queue of pending connections for sockfd may grow.  If a
				// connection request arrives when the queue is full, the client may
				// receive an error with an indication of ECONNREFUSED or, if the
				// underlying protocol supports retransmission, the request may be
				// ignored so that a later reattempt at connection succeeds
				perror("Error: ");
				return 0;
			}
			
			// handing zombie state
			signal(SIGCHLD, sig_child);

			//Communicate with client
			char *drink_quantities = (char*)malloc(BUFF_SIZE);
			int client_socket, bytes_sent, bytes_received, server_socket;
			char recv_data[BUFF_SIZE];
			int  sin_size = sizeof(struct sockaddr_in);
			pid_t pid;

			while (1){
				client_socket = accept(server_socket, (struct sockaddr*) & client, &sin_size);
				if (client_socket == -1){
					if (errno = EINTR){
						// EINTR: Systen Interrupt
						continue;
					}else{
						perror("Error: ");
					}
				}

				pid = fork();

				if (pid == 0){
					close(server_socket);
					print("Connection accepted from client with address:%s\n", inet_ntoa(client.sin_addr));

					char client_name[30] = "";

					if(recv(client_socket, client_name, sizeof(client_name), 0) < 0){
						print("Error retrieving client machine's name");
						return 0;
					}else{
						client_info *recv_client = findClient(client_name);

						printf("Reading from the inventory (database): %d %d %d\n",
							equipInfoAccess(1, 0, recv_client),
							equipInfoAccess(1, 1, recv_client),
							equipInfoAccess(1, 2, recv_client));

						sprintf(drink_quantities, "%d %d %d",
							equipInfoAccess(1, 0, recv_client),
							equipInfoAccess(1, 1, recv_client),
							equipInfoAccess(1, 2, recv_client));

						send(client_socket, drink_quantities, BUFF_SIZE, 0);
					}

				}
			}
			
		}

		printf("--------------------\n");
		printf("List of available vending machines:\n");
		printf("1. VM1\n2. VM2\n 3. VM3\n");
		printf("--------------------\n");

	}	
}