#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/param.h>
#include "rio.h"

#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

ssize_t rio_readn(int fd, char *buf, size_t n)
{
    int nleft = n;
    int readnum;
    extern int errno;

    while (nleft > 0)
    {
        readnum = read(fd, buf, nleft);
        if (readnum < 0 && errno == EINTR)
        {
            continue;
        }
        if (readnum < 0)
        {
            return -1;
        }
        else if (readnum == 0)
        {
            return (n - nleft);
        }
        else
        {
            nleft -= readnum;
            buf += readnum;
        }
    }
    return n;
}

ssize_t rio_writen(int fd, char *buf, size_t n)
{
    int nleft = n;
    int writenum;
    extern int errno;

    while (nleft > 0)
    {
        writenum = write(fd, buf, n);
        if (writenum < 0 && errno == EINTR)
        {
            continue;
        }
        else if (writenum < 0)
        {
            return -1;
        }
        else
        {
            nleft -= writenum;
            buf += writenum;
        }
    }
    return n;
}

ssize_t rio_readnb(Rio_buffer *rio_buffer, char *buf, size_t n)
{
    //  * 初始化剩余需要读取的字节数量为nleft = n
    size_t nleft = n;
    int min_use;
    void *dest;
    //  * 循环: nleft>0就进入循环
    while (nleft > 0)
    {
        //  * 判断中间缓冲是否有缓冲数据：rio_buffer->bytes_num > 0
        if (rio_buffer->bytes_num > 0)
        {

            //  * 获取缓冲区可用的bytes的数量 rio_buffer->bytes_num

            //  * 取 rio_buffer->bytes_num 和 nleft 的最小值 min_use = min(nleft, rio_buffer->bytes_num)
            min_use = MIN(nleft, rio_buffer->bytes_num);
            //  * 拷贝 min_use 大小到目标buf中
            dest = memcpy(buf, rio_buffer->ptr, min_use);
            if (dest != buf)
            {
                // 拷贝失败
                return -1;
            }
            //  * 减少nleft的值: nleft -= min_use;
            nleft -= min_use;
            //  * 将缓冲区的指针指向新的位置: rio_buffer->ptr += min_use
            rio_buffer->ptr += min_use;
            //  * 将缓冲区记录的可用数量进行减少min_use: rio_buffer->bytes_num -= min_use
            rio_buffer->bytes_num -= min_use;
            //  * 如果nleft == 0, 则读取已达期望数量, 可以退出, 方法直接返回 (n-nleft)
            if (nleft == 0)
            {
                return n;
            }
        }
        //  * 否则,继续循环while(1):
        while (1)
        {
            //  *  -> 循环重新读取sizeof(rio_buffer->buf)的数据
            int readnum;
            readnum = read(rio_buffer->fd, rio_buffer->buf, sizeof(rio_buffer->buf));
            //  *  -> 如果返回=0, 则表示已经取完数据, 方法返回 (expected_num - nleft)
            if (readnum == 0)
            {
                return (n - nleft);
            }
            //  *  -> 如果返回<0, 如果为打断异常,则需要进行重新读取,执行continue;
            else if (readnum < 0 && errno == EINTR)
            {
                continue;
            }
            //  *     如果返回<0, 又非打断异常, 返回 -1
            else if (readnum < 0)
            {
                fprintf(stderr, "read data failed %s \n", strerror(errno));
                return -1;
            }
            //  *  -> 如果返回>0,
            else if (readnum > 0)
            {
                //  *      重置rio_buffer的值：
                //  *          .read_ptr的值指向最开始的数据
                rio_buffer->ptr = &rio_buffer->buf[0];
                //  *          .bytes_num的值设置为读取返回的值
                rio_buffer->bytes_num = readnum;
                //  *      退出循环
                break;
            }
        }
    }
    return (n - nleft);
}

ssize_t rio_readlineb(Rio_buffer *rio_buffer, char *buf, size_t max)
{
    // 记录应用缓存当前读取位置的指针
    char *usrbuf = buf;
    // 初始化中间读取
    char c[1];
    // 初始化变量读取字节数readnum
    int readnum = 0;
    // 循环(1)，上限max-1个字节
    int i;
    // 空出一个字节用来放置NULL
    for (i = 0; i < max - 1; i++)
    {
        // 一次读取一个字节, 将字节塞入应用缓存中
        int readresult = rio_readnb(rio_buffer, c, 1);
        // 如果读取rio_read失败返回-1
        if (readresult == -1)
        {
            fprintf(stderr, "rio_readnb failed!%s \n", strerror(errno));
            return -1;
        }
        // 如果读取返回0,则退出循环
        else if (readresult == 0)
        {
            break;
        }
        // 否则
        else
        {
            // .将数据拷贝到目标应用缓存中去
            memcpy(usrbuf, c, 1);
            // .用户缓存指针+1
            usrbuf++;
            // .readnum进行+1
            readnum++;
            // .判断字节是否为 '\n'，如果为 '\n', 退出循环
            if (c[0] == '\n')
            {
                break;
            }
        }
    }
    // 在读取数据后一个字节塞入NULL字节
    if (readnum > 0)
    {
        buf[readnum] = '\0';
    }
    return readnum;
}

ssize_t rio_writenb(Rio_buffer *rio_buffer, char *buf, size_t n)
{
    //  * 初始化nleft变量为n
    size_t nleft = n;
    int bleft;
    int min_use;
    char *current_ptr = buf;
    //  * 循环(如果有数据需要写入(nleft > 0)):
    while (nleft > 0)
    {
        //  * .获取中间缓冲剩余空间 bleft 为缓冲的长度减去readsnum的数量
        bleft = sizeof(rio_buffer->buf) - (rio_buffer->bytes_num);
        //  * .求bleft和nleft的最小值 min_use
        min_use = MIN(bleft, nleft);
        if (min_use > 0)
        {
            //  * .拷贝min_use数目字节到rio_buffer的缓冲中去
            memcpy(rio_buffer->ptr, current_ptr, min_use);
            //  * ..更新rio_buffer中readsnum的数量(readsnum += min_use)
            rio_buffer->bytes_num += min_use;
            //  * ..指针+min_use,readsnum+min_use
            rio_buffer->ptr += min_use;
            //  * ..nleft的值进行更新,减去min_use
            nleft -= min_use;
            //  * ..bleft的值减去min_use
            bleft -= min_use;
        }
        //  * ..如果bleft的值为0,则执行刷新写入操作，将数据写入
        if (bleft == 0)
        {
            rio_flush(rio_buffer);
        }
        //  * ..如果bleft的值不为0,则不进行写入,数据暂存缓存之中, return n
        if (bleft > 0)
        {
            break;
        }
    }
    return n;
}

int rio_flush(Rio_buffer *rio_buffer)
{
    printf("start to execute flush!\n");
    int writenum;
    int nleft = rio_buffer->bytes_num;
    while (nleft > 0)
    {
        writenum = write(rio_buffer->fd, rio_buffer->buf, rio_buffer->bytes_num);
        if (writenum == -1 && (errno == EINTR))
        {
            continue;
        }
        else if (writenum == -1)
        {
            fprintf(stderr, "write failed!err:%s \n", strerror(errno));
            return -1;
        }
        else
        {
            nleft -= writenum;
            rio_buffer->ptr += writenum;
        }
    }
    // 刷新成功,重置缓冲
    rio_buffer->ptr = &rio_buffer->buf[0];
    rio_buffer->bytes_num = 0;
    return 0;
}

void rio_initbuffer(Rio_buffer *rio_buffer, int fd)
{
    rio_buffer->bytes_num = 0;
    rio_buffer->ptr = &(rio_buffer->buf[0]);
    rio_buffer->fd = fd;
}

int main()
{
    FILE *testfile;
    int i;
    testfile = tmpfile();
    Rio_buffer rio_buffer;
    rio_initbuffer(&rio_buffer, fileno(testfile));

    struct timeval start;
    struct timeval end;
    char buf[1] = {'a'};
    unsigned long timer;

    gettimeofday(&start, NULL);
    for (i = 0; i < 110; i++)
    {
        if (i % 20 == 0)
        {
            buf[0] = '\n';
            rio_writenb(&rio_buffer, buf, 1);
            buf[0] = 'a';
        }
        else
        {
            printf("execute num:%d\n", i);
            rio_writenb(&rio_buffer, buf, 1);
        }
    }

    gettimeofday(&end, NULL);
    timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    printf("execute time interval:%d \n", timer);

    rio_flush(&rio_buffer);

    lseek(fileno(testfile), 0, SEEK_SET);
    Rio_buffer r_rio_buffer;
    rio_initbuffer(&r_rio_buffer, rio_buffer.fd);

    char line_buf[128];
    int t_readnum;
    for (;;)
    {
        t_readnum = rio_readlineb(&r_rio_buffer, line_buf, 128);
        if (t_readnum == 0)
        {
            break;
        }
        printf("readline result is:%s", line_buf);
    }

    // FILE *testfile;
    // int i;
    // int j;
    // int randnum;
    // srand(time(0));
    // char *buf;
    // //  * 测试readline
    // //  * 新建临时文件
    // testfile = tmpfile();
    // //  * 向临时文件写入100行数据
    // //  * 循环,下标为i,自增:
    // //  * 获取文件的路径和名称
    // for (i = 0; i < 100; i++)
    // {
    //     //  * 如果为第一行，输入每个字符为i的数据,小于128个
    //     if (i == 0)
    //     {
    //         for (j = 0; j < 127; j++)
    //         {
    //             fprintf(testfile, "%d", (j % 10));
    //         }
    //         fprintf(testfile, "\n");
    //     }
    //     //  * 如果为第二行，输入每个字符为i的数据,大于128个数据
    //     else if (i == 1)
    //     {
    //         for (j = 0; j < 200; j++)
    //         {
    //             fprintf(testfile, "%d", (j % 10));
    //         }
    //         fprintf(testfile, "\n");
    //     }
    //     //  * 其他行随机生成任意长度的数据[1-200], 第一个字符为数目, 后面为对应字符为i的数据
    //     else
    //     {
    //         randnum = (rand() % 200) + 1;
    //         fprintf(testfile, "%d,", randnum);
    //         for (j = 0; j < randnum; j++)
    //         {
    //             fprintf(testfile, "%d", (j % 10));
    //         }
    //         fprintf(testfile, "\n");
    //     }
    //     //  * 循环结束
    // }
    // fflush(testfile);
    // // 重置文件游标位置
    // lseek(fileno(testfile), 0, SEEK_SET);
    // int t_readnum;
    // Rio_buffer rio_buffer;
    // int t_fd = fileno(testfile);
    // rio_initbuffer(&rio_buffer, t_fd);
    // char t_buf[1024];
    // // 打印出通过rio_readlineb返回的所有行

    // char lastc;
    // while ((t_readnum = rio_readlineb(&rio_buffer, (char *)t_buf, 127)) > 0)
    // {
    //     lastc = t_buf[strlen(t_buf) - 1];
    //     if(lastc == '\n'){
    //         t_buf[strlen(t_buf) - 1] = '\0';
    //     }
    //     printf("%s,返回长度为:%d, 是否有换行:%d\n", t_buf, strlen(t_buf), ((char)lastc) =='\n');

    // }
    //  * 定位文件游标到第一行
    //  * 调用rio接口readline,打印每行返回的结果
    //  * 删除临时文件

    /**
     * 测试readbn
     */
    // FILE *testfile;
    // int num;
    // int fno;
    // testfile = tmpfile();
    // char buf[10];
    // setvbuf(testfile, NULL, _IONBF, 0);
    // for (num = 0; num < 100; num++)
    // {
    //     fputc('a' + num, testfile);
    // }

    // fno = fileno(testfile);
    // off_t offset = lseek(fno, 0, SEEK_SET);

    // printf("offset:%c \n", offset);

    // memset(buf, 0, 10);
    // int readnum;
    // readnum = rio_readn(fno, buf, 10);
    // int i;
    // for (i = 0; i < 10; i++)
    // {
    //     printf("%c \n", buf[i]);
    // }
    // printf("readnum is:%d \n", readnum);
    // /**
    //  * 测试写功能
    //  */
    // lseek(fno, 0, SEEK_SET);
    // ssize_t writenum;
    // char wbuf[100];
    // for (i = 0; i < 100; i++)
    // {
    //     wbuf[i] = 'c' + i;
    // }
    // writenum = rio_writen(fno, wbuf, 100);
    // printf("write num is:%d \n", writenum);

    // lseek(fno, 0, SEEK_SET);
    // for (i = 0; i < 100; i++)
    // {
    //     // printf("%c \n", fgetc(testfile));
    // }
    // printf("test e: \n");
    // for (i = 0; i < 100; i++)
    // {
    //     printf("%c \n", 'e' + i);
    // }
}