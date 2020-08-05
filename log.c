#include <stdio.h>
#include <time.h>

char *print_error(int line, char *message){
  time_t rawtime;
  struct tm *time_info;
  char buff_time[128];

  time(&rawtime);
  time_info = localtime(&rawtime);
  strftime(buff_time,sizeof(buff_time),"%Y-%m-%d %H:%M:%S", time_info);
  printf("%s:[line %d] %s\n", buff_time, line, message);
}
