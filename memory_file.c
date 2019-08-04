#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BSZ 48

int main(){
    FILE *fp;
    char buf[BSZ];
    char cushion_buf[BUFSIZ];

    // memset is used to fill a block of memory with a particular value
    memset(buf, 'a', BSZ-2);
    buf[BSZ-2] = '\0';  
    buf[BSZ-1] = 'X';

    if((fp = fmemopen(buf, BSZ, "w+")) == NULL){
        printf("fmemopen failed!");
        exit(0);
    }
    setbuf(fp, cushion_buf);
    // 为了验证在fmemopen的时候会将第一个字节设置为null
    printf("initial contents: %s \n", buf);
    fprintf(fp, "hello, world");
    printf("before flush: %s\n", buf);
    fflush(fp);
    printf("after fflush: %s\n", buf);
    printf("len of string in buf=%ld \n\n", (long)strlen(buf));

    memset(buf, 'b', BSZ-2);
    buf[BSZ-2] = '\0';
    buf[BSZ-1] = 'X';
    fprintf(fp, "hello, world");
    printf("before fseek: %s \n", buf);
    printf("before cushion buf is: %s \n", cushion_buf);
    fseek(fp, 0, SEEK_SET);
    printf("after fseek cushion buf is: %s \n", cushion_buf);
    printf("after fseek: %s \n", buf);
    printf("len of string in buf = %ld \n\n", (long)strlen(buf));
    
    memset(buf, 'c', BSZ-2);
    buf[BSZ-2] = '\0';
    buf[BSZ-1] = 'X';
    fprintf(fp, "hello world");
    printf("before close: %s \n", buf);
    fclose(fp);
    printf("after close,cushion: %s\n", cushion_buf);
    printf("after close: %s \n", buf);
    printf("len of string in buf = %ld \n", (long)strlen(buf));

    return 0;
}