#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/time.h>
#include<unistd.h>
#include<string.h>

const int LEN = 1024;
int fds[2];          //只监测标准输入与输出这两个文件描述符
int main()
{
  int std_in = 0;
  int std_out = 1;
  int fds_max = 1;
  fd_set reads, writes;
  struct timeval timeout;

  fds[0] = std_in;
  fds[1] = std_out;

  while(1)
  {
    FD_ZERO(&reads);
    FD_ZERO(&writes);
    FD_SET(std_in, &reads);          //标准输入关注的是读事件
    FD_SET(NULL, &writes);       //标准输出关注的是写事件
    timeout.tv_sec = 3;         timeout.tv_usec = 0;
    switch( select(1, &reads, &writes, NULL, &timeout))
    {
      case 0:
        printf("select time out ......\n");
        break;
      case -1:
        perror("select");
        break;
      default:
        if(FD_ISSET(fds[0], &reads))       //可以从标准输入中读
        {
          char buf[LEN];
          memset(buf, '\0', LEN);
          fgets(buf,12,stdin);
          printf("echo: %s", buf);

          if(strncmp(buf, "quit", 4) == 0)
          {
            exit(0);
          }
        }
        if(FD_ISSET(fds[1], &writes))
        {
          char* buf = "write is ready.......\n";
          printf("%s", buf);
        }
      break;
    }
  }
}
