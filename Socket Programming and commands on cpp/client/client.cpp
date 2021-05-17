#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <cstdio>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdio>
#include <iostream>
 
/*for getting file size using stat()*/
#include<sys/stat.h>
 
/*for sendfile()*/
#include<sys/sendfile.h>
 
/*for O_RDONLY*/
#include<fcntl.h>

#define COMMAND(cmd) strcmp(command, cmd)==0

using namespace std;

int main(int argc,char *argv[])
{
	char choice[256], arg[256];
	FILE *fd;
	struct stat obj;
	char buf[100], command[5], filename[20], *f;
	int k, size, status;
	int filehandle;

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1)
	{
		printf("socket creation failed");
		exit(1);
	}
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(3002);
	connect(sock, (struct sockaddr*)&server_address, sizeof(server_address));
	int i = 1;

	while(1)
	{
		printf("Enter a choice:\n1- RETR\n2- STOR\n3- PWD\n4- LIST\n5- CD\n6- USER\n7- SYST\n8- PASV\n9- QUIT\n");
		cin >> choice;
		if(strcmp(choice, "USER") == 0){
			int name = system("whoami");
			printf("331 User name ok, need password\n");
			cout << endl;
		}
		else if(strcmp(choice, "RETR") == 0){
			printf("Enter filename to RETR: ");
			scanf("%s", filename);
			strcpy(buf, "RETR ");
			strcat(buf, filename);
			send(sock, buf, 100, 0);
			recv(sock, &size, sizeof(int), 0);
			if(!size)
			{
				printf("No such file on the remote directory\n\n");
				break;
			}
			f = (char *)malloc(size);
			recv(sock, f, size, 0);
			while(1)
			{
				filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);
				if(filehandle == -1)
				{
					sprintf(filename + strlen(filename), "%d", i);//needed only if same directory is used for both server and client
				}
				else 
				{
					break;
				}
			}
			write(filehandle, f, size);
			close(filehandle);
			strcpy(buf, "cat ");
			strcat(buf, filename);
			printf("Get file successfully\n");
			//system(buf);
		}

		else if(strcmp(choice, "STOR") == 0){
			printf("Enter filename to STOR to server: ");
			scanf("%s", filename);
			filehandle = open(filename, O_RDONLY);
			if(filehandle == -1)
			{
				printf("No such file on the local directory\n\n");
				break;
			}
			strcpy(buf, "STOR ");
			strcat(buf, filename);
			send(sock, buf, 100, 0);
			stat(filename, &obj);
			size = obj.st_size;
			send(sock, &size, sizeof(int), 0);
			sendfile(sock, filehandle, NULL, size);
			recv(sock, &status, sizeof(int), 0);
			if(status)
				printf("File stored successfully\n");
			else
				printf("File failed to be stored to remote machine\n");
			remove(filename);
		}

		else if(strcmp(choice, "PWD") == 0){
			strcpy(buf, "PWD");
			send(sock, buf, 100, 0);
			recv(sock, buf, 100, 0);
			printf("The path of the remote directory is: %s\n", buf);
		}

		else if(strcmp(choice, "LIST") == 0){
			strcpy(buf, "LIST");
			send(sock, buf, 100, 0);
			recv(sock, &size, sizeof(int), 0);
			f = (char*)malloc(size);
			recv(sock, f, size, 0);
			filehandle = creat("temp.txt", O_WRONLY);
			write(filehandle, f, size);
			close(filehandle);
			printf("The remote directory listing is as follows:\n");
			system("cat temp.txt");
		}

		else if(strcmp(choice, "CD") == 0){
			strcpy(buf, "CD ");
			printf("Enter the path to change the remote directory: ");
			scanf("%s", buf + 3);
			send(sock, buf, 100, 0);
			recv(sock, &status, sizeof(int), 0);
			if(status)
				printf("Remote directory successfully changed\n");
			else
				printf("Remote directory failed to change\n");
		}

		else if(strcmp(choice, "SYST") == 0)
		{
			printf("215 UNIX Type: L8.\n");
		}
		
		else if(strcmp(choice, "PASV") == 0)
		{
			printf("227 Entering PASV mode.\n");
		}

		else if(strcmp(choice, "QUIT") == 0){
			strcpy(buf, "QUIT");
			send(sock, buf, 100, 0);
			recv(sock, &status, 100, 0);
			if(status)
			{
				printf("Server closed\nQuitting..\n");
				exit(0);
			}
			printf("Server failed to close connection\n");
		}

		else
		{
			printf("intern server error\n");
		}
	}	
	return 0;
}