#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[], char *env[]){
    int i;
    for(i = 0; i < argc; i++){
        printf("arg%d, value:%s \n", i, argv[i]);
    }

    int j = 0;
    while(env[j] != 0){
        printf("%s \n", env[j]);
        j++;
    }
    printf("SHELL:%s \n", getenv("SHELL"));
    setenv("SHELL", "abc", 1);
    printf("after set SHELL:%s \n", getenv("SHELL"));
    unsetenv("SHELL");
    return 0;
}