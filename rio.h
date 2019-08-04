#include <stddef.h>
#include <stdio.h>

#define MAXBUF 128
typedef struct rio_buffer {
    int fd;
    char buf[MAXBUF];
    char *ptr;
    size_t bytes_num;
} Rio_buffer;

/**
 * No buffer read and write
 */
ssize_t rio_readn(int fd, char *buf, size_t n);
ssize_t rio_writen(int fd, char *buf, size_t n);

/**
 * Buffre read and write
 */
ssize_t rio_readlineb(Rio_buffer *rio_buffer, char *buf, size_t max);
ssize_t rio_writenb(Rio_buffer *rio_buffer, char *buf, size_t n);
ssize_t rio_readnb(Rio_buffer *rio_buffer, char *buf, size_t n);
int rio_flush(Rio_buffer *rio_buffer);
