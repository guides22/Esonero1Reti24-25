/*
 ============================================================================
 Name        : Client.c
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

int main(int argc, char *argv[]) {
#if defined WIN32
	//Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup().\n");
		return 0;
	}
#endif
	//client socket creation
	int c_socket;
	c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (c_socket < 0) {
		printf("Socket creation failed.\n");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}

	//set connection settings
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr(PROTO_IP); //server IP
	sad.sin_port = htons(PROTO_PORT); //server port
	if (connect(c_socket,(struct sockaddr *) &sad, sizeof(sad)) < 0) {
		printf( "Failed to connect.\n" );
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}

	printf("Connected with server.\n");
	printf("Enter the command for type and length of password you want to generate('n','a','m' or 's' + length(>=6 and <=32))(enter 'q' to end): ");
	char command[64];
	scanf("%s", command);
	char type = command[0];       //first char is the type of password
	int length = atoi(&command[1]); //convert next chars in an integer for length
	while(type != 'q') {
		while ((type!='n' && type!='a' && type!='m' && type!='s') || length < MIN_PASSWORD_LENGTH || length > MAX_PASSWORD_LENGTH) {
			printf("Error: Invalid type or length. Enter another command: \n");
			scanf("%s", command);
			type = command[0];
			length = atoi(&command[1]);
		}

		//send the command to server
		printf("Sending command...\n");
		if (send(c_socket, command, strlen(command), 0) != strlen(command)) {
			printf("send() sent a different number of bytes than expected.\n");
			closesocket(c_socket);
			clearwinsock();
			return -1;
		}
		printf("Command sent.\n");
		//receive the password from server
		char password[64];
		memset(password, 0, sizeof(password)); //reset password
		printf("Receiving password from server...\n");
		if ((recv(c_socket, password, sizeof(password), 0)) <= 0) {
			printf("recv() failed.\n");
			closesocket(c_socket);
			clearwinsock();
			return -1;
		}
		printf("Password generated from server: %s.\n", password);
		printf("Enter the command for type and length of password you want to generate('n','a','m' or 's' + length(>=6 and <=32))(enter 'q' to end): ");
		scanf("%s", command);
		type = command[0];
		length = atoi(&command[1]);
	}

	//close connection
	printf("Char 'q' entered, connection closed.\n");
	closesocket(c_socket);
	clearwinsock();
	return 0;
} // main end
