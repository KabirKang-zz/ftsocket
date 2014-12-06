/*
Name: Kabir Kang
Course: CS 372
Date: 11/26/14
Project: FTP via TCP Socketing: Project 2
Description: This pfogram acts as the server taking FTP requests, delivdring files, or ls lists
Use: compile with g++ ftserver.cpp -o ftserver
./ftserver <port>
 */

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
using namespace std;

struct sockaddr_in servaddr,dataaddr,address;
int listenfd, controlfd,datafd;
int controlPort;
char cmd[501];
int DATAPORT = 0;
int dataConnect(int);

/*
Description: Establishes the socket for the control connection.
 */
void startup(int port)
{
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&servaddr, '0', sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);
  bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
  cout << "Expecting client on control socket..."<<endl; 
  listen(listenfd, 10); 

}
/*
Description: The client will make a request for either a list or a get, so this function handles such a request.
 */
void handleRequest()
{
  while(1)
    {
      controlfd = accept(listenfd,(struct sockaddr*)NULL,NULL);
      //cout << "connected" << endl;
      cout << "ftserver and ftclient connected..." << endl;
      recv(controlfd,cmd,500,0);
      if (strcmp(cmd,"list")==0)
	{
	  string sendDir;
	  DIR * directory;
	  struct dirent * entry; 
	  //cout <<"LISTING"<<endl;
	  cout << "Client has requested list." << endl;
	  datafd = dataConnect(controlfd);
	  if((directory = opendir("./")) == NULL) {
	    perror("Error opening directory");
	    exit(EXIT_FAILURE);
	  }
	  while((entry = readdir(directory)) != NULL) {
	    if((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0)) {
	      //	      cout << entry->d_name << endl;
	      sendDir += entry->d_name;
	      sendDir += " ";
	      const char* buf = sendDir.c_str();
	      send(datafd,buf,strlen(buf),0);
	      }
	  }
	}
      else if(atoi(cmd) != -1) // If cmd is not list, nor is it -1, it can only be a request.
	{
	  cout << "File requested" << endl;
	  int filefd,numread;
	  char c;
	  char* message;
	  string invalid = "Invalid filename: file does not exist";
	  const char* fileBuf = invalid.c_str();
	  //cout << "Aw so you wanna play like dat" <<endl;
	  datafd = dataConnect(controlfd);
	  if((filefd = open(cmd, O_RDONLY)) == -1) {
	    send(datafd, fileBuf,strlen(fileBuf),0);
	      close(datafd);
	      perror("Error opening file");
	    exit(EXIT_FAILURE);
	  }// FILE VALIDATION

	  char myArray[1000];
	  ifstream file(cmd);
	  if(file.is_open())
	    {
	      for(int i = 0; i < 1000; ++i)
		{
		  file >> myArray[i];
		}
	    }	  

	  send(datafd,myArray,strlen(myArray),0);
	  if(numread == -1) {
	    perror("Error reading from file");
	    exit(EXIT_FAILURE);
	  }

	  close(filefd);
	  close(datafd);
    }
      
      close(controlfd);
      close(datafd);
      //cout << "Ports freed" <<endl;
    }
}
/*
Description: Establishes a connection on the data port
 */
int dataConnect(int ctrlfd) {
  int datafd;
  unsigned int length;

  //Get peer's address:
  length = sizeof(address);
  if(getpeername(ctrlfd, (struct sockaddr *) &address, &length) == -1) {
    perror("Error getting peer's address");
    close(ctrlfd);
    exit(EXIT_FAILURE);
  }
    
  //Change to the data port:
  address.sin_port = htons(DATAPORT);
  //cout << DATAPORT << endl;
  //Create a new socket:
  if((datafd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    perror("Error creating socket");
    exit(EXIT_FAILURE);
  }

  //Connect to peer via that socket:
  while(connect(datafd, (struct sockaddr *) &address, sizeof(address)) == -1) {
    //cout << "Accessing port..." << endl;
    }
   
  return datafd;
}
/*
Description: The main function that creates a connection with the client and continues from there. 
 */
int main(int argc,char** argv)
{
  if(argc!=2)
    {
      cerr<<"Please include a port number" << endl;
      return 1; 
    }

  controlPort = atoi(argv[1]);
  DATAPORT = controlPort+1;
  cout << "Server starting on Host A" << endl;
  startup(controlPort);

  handleRequest();

  return 0;
}
