/*
 ============================================================================
 Name        : Server.c
 Author      : Guillaume Desaphy
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif
#include <stdio.h>
#include "protocol.h"

void clearwinsock() {
	#if defined WIN32
	WSACleanup();
	#endif
}

void generate_numeric(int length, char *password) {
    const char *numbers = "0123456789";
    for (int i = 0; i < length; i++) {
        password[i] = numbers[rand() % 10];
    }
    password[length] = '\0';
}

void generate_alpha(int length, char *password) {
    const char *lowercase = "abcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < length; i++) {
        password[i] = lowercase[rand() % 26];
    }
    password[length] = '\0';
}

void generate_mixed(int length, char *password) {
    const char *charset = "abcdefghijklmnopqrstuvwxyz0123456789";
    for (int i = 0; i < length; i++) {
        password[i] = charset[rand() % 36];
    }
    password[length] = '\0';
}

void generate_secure(int length, char *password) {
    const char *charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[]{}|;:,.<>?";
    int charset_size = strlen(charset);
    for (int i = 0; i < length; i++) {
        password[i] = charset[rand() % charset_size];
    }
    password[length] = '\0';
}

int main(int argc, char *argv[]) {
#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup().\n");
		return 0;
	}
#endif
	//socket creation
	int my_socket;
	my_socket=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (my_socket<0) {
		printf("Socket creation failed.\n");
		clearwinsock();
		return -1;
	}

	//connection settings
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad)); //ensures that extra bytes contain 0
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr(PROTO_IP);
	sad.sin_port = htons(PROTO_PORT);
	if (bind(my_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
			printf("bind() failed.\n");
			closesocket(my_socket);
			clearwinsock();
			return -1;
		}

	//listen settings
	if (listen(my_socket, QLEN) < 0) {
			printf("listen() failed.\n");
			closesocket(my_socket);
			clearwinsock();
			return -1;
		}

	//accept new connection
	struct sockaddr_in cad; //structure for the client address
	int client_socket;       //socket descriptor for the client
	int client_len;          //the size of the client address
	printf("Waiting for a client to connect...\n\n");
	while (1) {
			client_len = sizeof(cad); //set the size of the client address
			if ((client_socket = accept(my_socket, (struct sockaddr*) &cad, &client_len)) < 0) {
				printf("accept() failed.\n");
				closesocket(client_socket);
				continue;
			} else {
				printf("New connection from %s:%d.\n", inet_ntoa(cad.sin_addr), ntohs(cad.sin_port));
				//receive command from client
				char msg[64];
				char password[MAX_PASSWORD_LENGTH];
				while(1) {
					memset(msg, 0, sizeof(msg)); //reset the command
					memset(password, 0, sizeof(password)); //reset the password
					printf("Receiving command from client...\n");
					int bytes_received = recv(client_socket, msg, sizeof(msg), 0);
					if (bytes_received <= 0) { //connection closed or error
						if (bytes_received == 0) {
							printf("Client disconnected.\n");
						} else {
							printf("recv() failed.\n");
						}
						closesocket(client_socket);
						break; //exit the loop for this client
					}
					printf("Command received from %s:%d: %s.\n", inet_ntoa(cad.sin_addr), ntohs(cad.sin_port), msg);
					char type = msg[0];       //first char is the type of password
					int length = atoi(&msg[1]); //convert next chars in an integer for length
					//generate password
					switch (type) {
						case 'n': generate_numeric(length, password); break;
						case 'a': generate_alpha(length, password); break;
						case 'm': generate_mixed(length, password); break;
						case 's': generate_secure(length, password); break;
						default: strcpy(password, "Error: Invalid type.\n"); break;
					}
					printf("Password generated: %s.\n", password);
					printf("Sending password...\n");
					//send the password
					if (send(client_socket, password, sizeof(password), 0) != sizeof(password)) {
						printf("send() sent a different number of bytes than expected.\n");
						closesocket(client_socket);
						break;
					}
					printf("Password sent.\n");
					printf("------------------------------------------------------------\n");
				}
			}
			printf("Waiting for a client to connect...\n\n");
		}

	closesocket(my_socket);
	clearwinsock();
	return 0;
} // main end
