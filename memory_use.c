#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>

void main(void){
    // 虚拟内存每个片4k, 4096字节
    // MAP_ANON 表示被映射的对象是一个匿名文件(什么是匿名文件,就是全部为0)
    int size = 1024;
    void *ptr;
    // 分配size长度的连续的片
    ptr = mmap(NULL, size, PROT_WRITE, MAP_PRIVATE|MAP_ANON, 0, 0);
    printf("mmap allocated address is %lx \n", ptr);
    munmap(ptr, size);

    ptr = sbrk(0);
    printf("sbrk(0) returned address is: %ld \n", ptr);

    ptr = sbrk(-1);
    printf("sbrk(-1) returned address is:%ld \n", ptr);

    ptr = sbrk(0);
    printf("sbrk(0) returned address is:%ld \n", ptr);
}