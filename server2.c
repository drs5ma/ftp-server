#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
/*
* // IPv4 AF_INET sockets:
*
* struct sockaddr_in {
*   short            sin_family;   // e.g. AF_INET, AF_INET6
*   unsigned short   sin_port;     // e.g. htons(3490)
*   struct in_addr   sin_addr;     // see struct in_addr, below
*   char             sin_zero[8];  // zero this if you want to
* }
*
* struct in_addr {
*    unsigned long s_addr;          // load with inet_pton()
* }
*
* struct sockaddr {
*    unsigned short    sa_family;    // address family, AF_xxx
*    char              sa_data[14];  // 14 bytes of protocol address
* }
*/

char *response_150 = "150 file status okay, about to open data connection\r\n";
char *response_200 = "200 command okay\r\n";
char *response_215 = "215 UNIX Type: L8\r\n";
char *response_220 = "220 service ready for new user\r\n";
char *response_221 = "221 service closing control connection\r\n";
char *response_226 = "226 closing data connection\r\n";
char *response_331 = "331 User okay, password required\r\n";

char *response_504 = "504 command not implemented\r\n";
int main(int argc, char **argv){

  // create new TCP/IP socket for control and data connections from client
  int data_socket; //dont init quite yet, wait for port command
  int server_socket;
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(server_socket==-1){
    printf("socket couldn't be created\n");
    return 0;}

  // populate the server sockaddr_in struct 
  struct sockaddr_in data_addr; //dont init quite yet
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(atoi(argv[1]));

  // bind server socket to port specified
  if( bind(server_socket, 
	   (struct sockaddr *)&server_addr, 
	   sizeof(server_addr)) < 0){
    printf("could not bind socket to port\n");
    return 0;} 
  
  // listen on the port
  int backlog = 3;// does this change to 1 since only supporting one client @ a time?
  listen(server_socket, backlog);

  // accept incoming connections
  struct sockaddr_in client;
  int sockaddr_size = sizeof(struct sockaddr_in);
  printf("Server online  waiting for new connections on port %d  ... \n", atoi(argv[1]));
  int client_socket = accept(server_socket, 
			 (struct sockaddr *)&client,  
			 (socklen_t*)&sockaddr_size);
  if(client_socket<0){
    printf("connection failed\n");return 0;}
  

  // parse+print connection info
  char *client_ip = inet_ntoa(client.sin_addr);
  int client_port = ntohs(client.sin_port);
  printf("Connection accepted from %s port %d\n", client_ip, client_port);
  
  // and send back the Service read for new user code
  send(client_socket, response_220, strlen(response_220), 0);

  // prepare for client loop
  int message_size = 1024;
  char client_message[message_size];
  int readlen = 0;

  //clear the client_message 
  memset(client_message, 0, message_size); 
  // and read the first message from client
  readlen  = read(client_socket, client_message, message_size);

  //print the client_message
  int i;printf("Client Msg: "); 
  for(i=0;i<readlen;i+=1){printf("%c", client_message[i]);}

  /*
  *  Client loop
  */
  while(readlen){

    /* 
    *  Any username/password combination will be accepted
    */
    if(strncmp("USER", client_message,4)==0){
      send(client_socket,response_331 , strlen(response_331), 0);}
    else if(strncmp("PASS", client_message,4)==0){
      send(client_socket, response_200, strlen(response_200) ,0);}


    /* 
    *  "Remote system type is UNIX."
    *  "Using binary mode to transfer files."
    */
    else if(strncmp("SYST", client_message,4)==0){
      send(client_socket, response_215, strlen(response_215) ,0);
    }

    /*
    *  open a data connection from port 21 
    */
    else if(strncmp("PORT", client_message,4)==0){
      int p1,p2;
      sscanf(client_message, "PORT 127,0,0,1,%d,%d\n",&p1,&p2);
      printf("p1 %d, p2 %d\n", p1, p2);
      int portno =  p1*256+p2;
      //we need to open a data connection
      data_socket = socket(AF_INET, SOCK_STREAM, 0);
      if(data_socket==-1){
      	printf("data socket couldn't be created\n");
      	return 0;}
      //construct the addr to connect to 
      data_addr.sin_family = AF_INET;
      data_addr.sin_addr.s_addr = INADDR_ANY;
      data_addr.sin_port = htons(portno);
      //connect to the client specified port
      if(connect(data_socket, (struct sockaddr *)&data_addr, sizeof(data_addr))<0){
      	printf("error connecting...\n");
      }
      else{
	printf("connected \n"); //send a reassuring 200 response okay
	send(client_socket, response_200, strlen(response_200)  ,0);	
      }
    }
    else if(strncmp("LIST", client_message,4)==0){
      send(client_socket,response_150,strlen(response_150) ,0);
      char* msgdata = "directory listingasdfasdfasdfblah\r\nfile 1 34 kb\r\nfile 4 fkls\r\n";
      send(data_socket,msgdata, strlen(msgdata), 0 );    
      close(data_socket);
      send(client_socket, response_226, strlen(response_226), 0);
    }

    else if(strncmp("QUIT", client_message, 4)==0){
      //send them a 221 closing control conneciton command
      send(client_socket, response_221, strlen(response_221), 0);
    }
    else{
      //send them a 504 command not implemented i guess
      send(client_socket, response_504, strlen(response_504), 0);
    }

   // clear the client_message buffer and get another client_message
   memset(client_message, 0, message_size);
   readlen = recv(client_socket, client_message, message_size, 0);
   int i;printf("Client Msg: "); // print client message
   for(i=0;i<readlen;i+=1){printf("%c", client_message[i]);}
  }
  
  return 0;

}
