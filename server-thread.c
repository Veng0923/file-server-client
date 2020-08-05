#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#include "bao.h"
#include "log.h"

#define ADDRESS "127.0.0.1"
#define PORT 8889
#define EPOLL_SIZE 100

extern int errno;

struct epoll_events
{
  int size;
};

int create_socket()
{
  // int on = 1;
  struct sockaddr_in socket_address;

  int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  socket_address.sin_family = AF_INET;
  socket_address.sin_port = htons(PORT);
  socket_address.sin_addr.s_addr = inet_addr(ADDRESS);

  // setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

  int binded = bind(server_socket, (struct sockaddr *)&socket_address,
                    sizeof(socket_address));
  if (binded == -1){
    // perror("bind");
    print_error(35, "bind error");
    return -1;
  }

  int listend = listen(server_socket, 1);
  if (listend){
    // perror("listen");
    print_error(43, "listen error");
    return -1;
  }
  return server_socket;
}

void *handle_client(void *client_sock)
{
  int client_socket = *(int *)client_sock;
  char buff[1024];
  while (1){
    memset(buff, 0, sizeof(buff));
    int ret = read(client_socket, buff, 1024);
    if (ret == -1 || ret == 0){
      // perror("read error");
      print_error(58,"read error");
      close(client_socket);
      break;
    }
    printf("client(%d): %s\n", client_socket, buff);
    if (strcmp(buff, "end") == 0){

      close(client_socket);
      printf("client: %d exit!\n", client_socket);
      break;
    }
    else if (strcmp(buff, "upload") == 0){
      FILE *fp;
      int file;
      struct Protocol recv_file;
      char file_path[50];

      memset(file_path, 0, sizeof(file_path));
      memset(&recv_file, 0, sizeof(recv_file));
      printf("client(%d): upload file\n", client_socket);
      memset(buff, 0, sizeof(buff));
      int file_name = read(client_socket, buff, sizeof(buff));
      if (file_name == -1){
        print_error(__LINE__ - 2, "read error");
        break;
      }
      strcpy(file_path, buff);
      remove(file_path);
      fp = fopen(file_path, "a+b");
      printf("uploading!!!!\n");
      while ((file = read(client_socket, &recv_file, sizeof(recv_file))) > 0){
        
        if (recv_file.status == STATUS_FILE){
          fwrite(&recv_file.body,sizeof(char),recv_file.size,fp);
          fflush(fp);
          memset(&recv_file,0,sizeof(recv_file));
          strcpy(buff,"success");
          write(client_socket,buff,sizeof(buff));
        }
        else if (recv_file.status == STATUS_END){
          break;
        }
      }
      fclose(fp);
      printf("client(%d): uploaded.\n", client_socket);
    }
    else if (strcmp(buff, "download") == 0){
      int file;
      FILE *fp;
      struct Protocol bye;

      printf("client(%d): download file\n", client_socket);
      memset(buff, 0, sizeof(buff));
      int download_file = read(client_socket, buff, sizeof(buff));
      fp = fopen(buff, "rb");
      struct Protocol p_file;
      while ((file = fread(p_file.body, sizeof(char), sizeof(p_file.body)/sizeof(char),fp))>0){
        p_file.status = STATUS_FILE;
        p_file.size = file;
        write(client_socket, &p_file, sizeof(p_file));
        read(client_socket,buff,sizeof(buff));
        if(strcmp(buff,"success") == 0){
          continue;
        }
      }
      fclose(fp);
      bye.status = STATUS_END;
      write(client_socket, &bye, sizeof(bye));
    }
    strcpy(buff, "received!");
    write(client_socket, buff, strlen(buff));
  }
  close(client_socket);
}
int wait_client(int server_socket)
{
  struct sockaddr_in client_address;
  struct epoll_events set;

  socklen_t client_address_len = sizeof(client_address);

  set.size = 1;
  printf("wait client!\n");
  int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
  printf("client %d,connected!\n", client_socket);

  if (client_socket == -1){
    print_error(__LINE__ - 4, "accept error");
    close(client_socket);
    exit(1);
  }

  return client_socket;
}
int main(int argc, char const *argv[])
{
  int server_socket = create_socket();
  while (1){
    int client_socket = wait_client(server_socket);
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_client, (void *)&client_socket);
    pthread_detach(thread_id);
  }

  close(server_socket);
  return 0;
}
