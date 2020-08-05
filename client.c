#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "bao.h"
#include "log.h"

#define PORT 8889
#define SERVER_IP "127.0.0.1"

extern int errno;

int main(int argc, char const *argv[])
{
  struct sockaddr_in socket_address;
  char buff[1024];

  // 创建客户端socket描述符
  int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (client_socket == -1){
    // perror("socket error");
    print_error(20,"socket_error");
    return -1;
  }

  //  构建服务器地址地址
  memset(&socket_address, 0, sizeof(socket_address));
  socket_address.sin_family = AF_INET;
  socket_address.sin_addr.s_addr = inet_addr(SERVER_IP);
  socket_address.sin_port = htons(PORT);
  int addrlen = sizeof(socket_address);

  // 与服务器建立连接
  int connected = connect(client_socket, (struct sockaddr *)&socket_address, addrlen);
  if (connected == -1){
    // perror("connected error");
    print_error(__LINE__ - 3,"connected error");
    return -1;
  }

  printf("success connect server\n");
  while (1){
    int count = 0;
    printf("请输入你向发送的消息(end:退出;upload:上传文件;download:下载文件):\n");
    scanf("%s", buff);
    write(client_socket, buff, strlen(buff));
    if (strcmp(buff, "end") == 0){
      break;
    }
    else if (strcmp(buff, "upload") == 0){
      FILE *fp;
      char file_path[50];
      int file;
      struct Protocol bye;
      char *file_name;

      memset(buff, 0, sizeof(buff));
      memset(file_path, 0, sizeof(file_path));
      printf("请输入你要发送文件名:\n");
      scanf("%s", buff);
      sprintf(file_path, "./client-files/%s", buff);
      fp = fopen(file_path, "rb");
      if(!fp){
        print_error(__LINE__ - 2, "file not exist");
        continue;
      }
      write(client_socket, buff, sizeof(buff));
      struct Protocol p_file;
      memset(p_file.body,0,sizeof(p_file.body));
      while ((file = fread(p_file.body, sizeof(char), sizeof(p_file.body)/sizeof(char), fp)) > 0){
        p_file.status = STATUS_FILE;
        p_file.size = file;
        fflush(fp);
        int w_result = write(client_socket, &p_file, sizeof(p_file));
        read(client_socket,buff,sizeof(buff));
        if(strcmp(buff,"success") == 0){
          continue;
        }
      }
      fclose(fp);
      bye.status = STATUS_END;
      write(client_socket, &bye, sizeof(bye));
    }
    else if (strcmp(buff, "download") == 0){
      int file;
      struct Protocol recv_file;
      FILE *fp;
      char file_path[20];

      printf("请输入下载的文件名:\n");
      memset(&buff, 0, sizeof(buff));
      scanf("%s", buff);
      write(client_socket, buff, sizeof(buff));
      sprintf(file_path, "./client-files/%s", buff);
      fp = fopen(file_path, "w");
      if (!fp){
        continue;
      }else{
        fclose(fp);
      }
      fp = fopen(file_path, "ab");
      while (file = read(client_socket, &recv_file, sizeof(recv_file)) > 0){
        if (recv_file.status == STATUS_FILE){
          fwrite(&recv_file.body,sizeof(char),recv_file.size,fp);
          fflush(fp);
          strcpy(buff, "success");
          write(client_socket,buff,sizeof(buff));
        }else if (recv_file.status == STATUS_END){
          break;
        }
      }

      fclose(fp);
    }
    memset(buff, 0, sizeof(buff));
    int ret = read(client_socket, buff, sizeof(buff));
    printf("server: %s\n", buff);
  }
  close(client_socket);
  return 0;
}
