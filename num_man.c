int Atoi(const char *str, int *result);

static int is_digit(char c){
    if(c > '0' && c < '9'){
        return 1;
    }else{
        return 0;
    }
}

int Atoi(const char *str, int *result){
    int total = 0;
    char sign;

    // 去掉头部的空格
    while(*str == ' '){
        str++;
    }
    // 取第一个符号判断是否为数字
    // 如果不为数字但是为+,-则为数字符号
    // 否则报错退出
    if(!is_digit(*str)){
        sign = *str;
        if(!(sign == '+' || sign == '-')){
            printf("format is error!");
            return -1;
        }else{
            str++;
        }
    }
    // 循环：
    // 如果符号为数字进入循环，随后进行移动指针
    while(is_digit(*str)){
        // 求和结果取新值：目前求和结果*10+当前数字
        total = total * 10 + ((unsigned)*str - '0');
        str++;
    }
    // 去除尾部的0
    while((*str) == ' '){
        str++;
    }
    // 判断尾部是否仍然有不合法的数字
    if((unsigned)(*str) != 0 && (*str) != '\n'){
        printf("illegal num in trail \n");
        return -1;
    }
    // 转化最终结果的正负
    if(sign == '-'){
        total = -total;
    }
    *result = total;
    return 0;
}
