#include<stdio.h>
#include<string.h>

//用来测试如何分割获取用户输入的命令


void main(){
  char file[512] = "put a.txt";
  char *arg;
  arg = strtok(file, " \t\n");
  printf("%s\n",arg);
  arg = strtok(NULL, " \t\n");
  printf("%s\n",arg);
  return;
}
