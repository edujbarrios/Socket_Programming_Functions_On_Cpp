#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <cstdio>
#include <stdlib.h>
#include <iostream>
 
/*for getting file size using stat()*/
#include<sys/stat.h>
 
/*for sendfile()*/
#include<sys/sendfile.h>
 
/*for O_RDONLY*/
#include<fcntl.h>

using namespace std;
 
int main(int argc,char *argv[])
{
	struct sockaddr_in server, client;
	struct stat obj;
	socklen_t sock2, len;
	char buf[100], command[5], filename[20];
	int k, i, size, c;
	int filehandle, sock1;
	sock1 = socket(AF_INET, SOCK_STREAM, 0);
	if(sock1 == -1)
	{
		printf("Socket creation failed");
		exit(1);
	}
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(3002);
	server_address.sin_addr.s_addr = INADDR_ANY;

	k = bind(sock1,(struct sockaddr*)&server_address, sizeof(server_address));
	if(k == -1)
	{
		printf("Binding error");
		exit(1);
	}
	k = listen(sock1,1);
	if(k == -1)
	{
		printf("Listen failed");
		exit(1);
	}
	len = sizeof(client);
	sock2 = accept(sock1,(struct sockaddr*)&client, &len);
	i = 1;

	while(1){
		recv(sock2, buf, 100, 0);
		sscanf(buf, "%s", command);

		if(!strcmp(command,"RETR"))
		{
			sscanf(buf, "%s%s", filename, filename);
			stat(filename, &obj);
			filehandle = open(filename, O_RDONLY);
			size = obj.st_size;
			if(filehandle == -1)
			size = 0;
			send(sock2, &size, sizeof(int), 0);
			if(size)
			sendfile(sock2, filehandle, NULL, size);
			remove(filename);
		}

		else if(!strcmp(command, "STOR"))
		{
			int c = 0, len;
			sscanf(buf+strlen(command), "%s", filename);
			recv(sock2, &size, sizeof(int), 0);
			i = 1;
			while(1)
			{
				filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);
				if(filehandle == -1)
				{
					sprintf(filename + strlen(filename), "%d", i);
				}
				else
				{
					break;
				}
			}
			char *f = (char*)malloc(size);
			recv(sock2, f, size, 0);
			c = write(filehandle, f, size);
			close(filehandle);
			send(sock2, &c, sizeof(int), 0);
		}

		else if(!strcmp(command, "PWD"))
		{
			system("pwd>temp.txt");
			i = 0;
			FILE*f = fopen("temp.txt","r");
			while(!feof(f))
			buf[i++] = fgetc(f);
			buf[i-1] = '\0';
			fclose(f);
			send(sock2, buf, 100, 0);
		}

		else if(!strcmp(command, "LIST"))
		{
			system("ls >temps.txt");
			i = 0;
			stat("temps.txt",&obj);
			size = obj.st_size;
			send(sock2, &size, sizeof(int),0);
			filehandle = open("temps.txt", O_RDONLY);
			sendfile(sock2,filehandle,NULL,size);
		}

		else if(!strcmp(command, "CD"))
		{
			if(chdir(buf+3) == 0)
				c = 1;
			else
				c = 0;
			send(sock2, &c, sizeof(int), 0);
		}
		
		else if(!strcmp(command, "BYE") || !strcmp(command, "QUIT"))
		{
			printf("FTP server quitting..\n");
			i = 1;
			send(sock2, &i, sizeof(int), 0);
			exit(0);
		}
	}

	return 0;
}