#include <cstdio>
#include <unistd.h>
//#include <fcntl.h>
#include <cstring>
#include <errno.h>
#include <time.h>

#include "common.h"

void Log(const char *const str)
{
    //用fd的方式在沈坚服务器上会出现errno==13,permission denied
    // int fd=open(str,O_WRONLY|O_CREAT|O_APPEND);
    // if(fd==-1)
    // {
    //     printf("打开日志文件失败:%d\n",errno);
    //     return;
    // }
    // write(fd,LOG_PATHNAME,strlen(str));
    // close(fd);
    FILE *fp=fopen(LOG_PATHNAME,"a");
    if(fp==NULL)
    {
        printf("打开日志文件失败:%d\n",errno);
        return;
    }

    time_t curtime = time(NULL);
    tm *ptm = localtime(&curtime);
    char gettime[21];   //包括一个空格，一个尾零
    sprintf(gettime, "%d-%02d-%02d %02d:%02d:%02d ", ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    fwrite(gettime,1,strlen(gettime),fp);
    fwrite(str,1,strlen(str),fp);
    fclose(fp);
    printf(str);
}