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

int main(int argc, char **argv){

  // create new TCP/IP socket
  int server_socket;
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(server_socket==-1){
    printf("socket couldn't be created\n");
    return 0;}

  // populate sockaddr_in struct
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(atoi(argv[1]));

  // bind socket to port
  if( bind(server_socket, 
	   (struct sockaddr *)&server_addr, 
	   sizeof(server_addr)) < 0){
    printf("could not bind socket to port\n");
    return 0;} 
  
  // listen on port
  int backlog = 3;// why three?
  listen(server_socket, backlog);

  // create a new client_socket accept incoming connections
  printf("Server online  waiting for new connections on port %d  ... \n", atoi(argv[1]));
  struct sockaddr_in client;
  int c = sizeof(struct sockaddr_in);
  int client_socket = accept(server_socket, 
			 (struct sockaddr *)&client,  
			 (socklen_t*)&c);
  if(client_socket<0){
    printf("connection failed\n");return 0;}
  // parse+print connection info
  char *client_ip = inet_ntoa(client.sin_addr);
  int client_port = ntohs(client.sin_port);
  printf("Connection accepted from %s port %d\n", client_ip, client_port);
  
  //send back the Service read for new user code
  send(client_socket, "220\r\n", 5, 0);


  // send something to client
  // char *response = "Hi client, your connected.\n";
  //write(client_socket, response, strlen(response));
  
  // put client in an echo loop
  int message_size = 1024;
  char client_message[message_size];
  printf("server made it here\n");

  //int readlen = 0;
  //readlen  = read(client_socket, client_message, message_size);
  //printf("buffer: %s\n", client_message);
  //while(readlen){
  //readlen = recv(client_socket, client_message, message_size, 0);
  // write(client_socket, client_message, readlen);
  // int i;printf("Client Msg: ");
  // for(i=0;i<readlen;i+=1){printf("%c", client_message[i]);}
  //}
  
  return 0;

}
