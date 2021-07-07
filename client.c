#include "extra.h"

int pipe_p2c[2];
int pipe_c2p[2];
int* figures;
char name[100] = "";

void equipMain(int client_sock);
void commoditySales(int client_sock);
void writeCache(int* figures);
void readCache(int* figures);

typedef struct mess_buffer {
	long mesg_type;
	char mesg_text[BUFF_SIZE];
} message;


int main(int argc, char *argv[]) {
	figures = (int*) malloc(3 * sizeof(int));
	char* figures_str = (char*) malloc(BUFF_SIZE);
	int server_port = 0;
	char server_ip[16] = "";
	int stat;

	int client_sock;	
	struct sockaddr_in server_addr;
	
	// Check validity of input
	va_cli(argc, argv, server_ip, &server_port, &name);
	
	// Construct socket
	client_sock = socket(AF_INET, SOCK_STREAM, 0);
	
	// Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = inet_addr(server_ip);

	// Request to connect server
	if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0) {
		printf("\nError! Cannot connect to sever!\nClient exit imediately!\n");
		return 0;
	}

	if (send(client_sock, name, 100, 0) < 0) {
		printf("Cannot send machine's name\nClient exit imediately!\n");
		return 0;
	}

	recv(client_sock, figures_str, BUFF_SIZE, 0);
	sscanf(figures_str, "%d %d %d", figures, figures + 1, figures + 2);
	writeCache(figures);
	printf("Received figures from server %d %d %d\n", figures[0], figures[1], figures[2]);
	
	pid_t pid = fork();

	if (pid == 0) { 
		// Child process: Initiate a new session for a newcomer
		equipMain(client_sock);
	} else { 
		// Parent process: Check for the numbers of figures
		while(1) {
			char stri[BUFF_SIZE];		
			recv(client_sock, stri, BUFF_SIZE, 0);
			int x;

			// Read the figures in VM from the server
			sscanf(stri, "%d", &x);
			printf("\nCommodity delivery is in progress. Please wait for 10s.\n");
			readCache(figures);
			figures[x] += 10;
			writeCache(figures);

			kill(pid, SIGSTOP);
			pid_t pid2 = fork();
			if (pid2 == 0) {
				char i;
				while(1)
					scanf("%c", &i);
			}

			sleep(10);
			fseek(stdin, 0, SEEK_END);
			kill(pid2, SIGKILL);
			menu_home(figures);
			kill(pid, SIGCONT);
		}
	}
	
	close(client_sock);
	return 0;
}

void equipMain(int client_sock) {
	while(1) {
		menu_home(figures); 
		int c;
		fseek(stdin, 0, SEEK_END);
		
		scanf("%d", &c);
		readCache(figures);
		
		// if (figures[c-1] <= 0 && c != 4){
		// 	printf("sold out!\n");
		// 	continue;
		// } else if (c != 4) 
		// 	figures[c-1] -= 1; 

		// writeCache(figures);
		switch(c) {
			case 0:
				menu_bye();
				send(client_sock, "shut_down", strlen("shut_down") + 1, 0);
				kill(0, SIGKILL);
				exit(0);
			case 1:
			case 2:
			case 3:
				if (figures[c-1] <= 0) {
					printf("Sold out!\n");
					continue;
				} else {
					figures[c-1]--;
					writeCache(figures);
					break;
				}
			default:
				printf("\nItem number not available!\nSelect from 1 to 3 to buy drinks, or 0 to exit.\n\n");
				continue;
		}

		char buff[BUFF_SIZE];
		sprintf(buff, "%d", c-1);
		
		pipe(pipe_p2c);
		pipe(pipe_c2p);

		pid_t pid = 0;
			
		switch (pid = fork()) {
			case -1:
				perror("processGenerate fork");
				exit(1);
			case 0:
				//printf("childProcess start\n");
				commoditySales(client_sock);
				exit(0);
			default:
				//printf("parentProcess start\n");

				close(pipe_p2c[0]);
				write(pipe_p2c[1], buff, strlen(buff) + 1);
				close(pipe_p2c[1]);

				wait(NULL);
				
				char tmp[BUFF_SIZE];
				
				close(pipe_c2p[1]);
				read(pipe_c2p[0], tmp, BUFF_SIZE);
				close(pipe_c2p[0]);

				printf("\n\n%s\n\n",tmp);
		} // end switch fork()
	} // end while
}

void commoditySales(int client_sock) {
	char recv_str[BUFF_SIZE];

	close(pipe_p2c[1]);
	read(pipe_p2c[0], recv_str, BUFF_SIZE);
	close(pipe_p2c[0]);

	char send_str[BUFF_SIZE] = "Thank you for chosing us\n";

	close(pipe_c2p[0]);
	write(pipe_c2p[1], send_str, strlen(send_str) + 1);
	close(pipe_c2p[1]);

	send(client_sock, recv_str, strlen(recv_str) + 1, 0);
}

void writeCache(int* figures) {
	char a[100] = "cache_";
	FILE *f = fopen(strcat(a, name) , "w");
	int i = 0;
	while (i < 3){
		fprintf(f, "%d\n", figures[i]);
		i++;
	}
	fclose(f);
}

void readCache(int* figures){
	char a[100] = "cache_";
	FILE *f = fopen(strcat(a, name) , "r");
	int i = 0;
	while (i < 3){
		fscanf(f, "%d\n", figures+i);
		i++;
	}
	fclose(f);
}
