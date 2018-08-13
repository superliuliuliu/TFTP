#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>



/*
 * DEMO  用于从文本中逐行分割读取数据
 */
 int is_ip_limit(){
     FILE * fp = fopen("./ip.txt", "r");//客户端将不允许连接的客户端ip存储到ip.txt中
     char ip[512];
     if (fp == NULL){
         printf("文本文件不存在\n");
         return -1;
     }
     while(fscanf(fp,"%[^\n]%*c\n", ip) != EOF){
         if (strcmp(ip, "47.94.211.34") == 0){
             printf("test");
             fclose(fp);
             return -1;
         }
     }
     fclose(fp);
     return 1;

 }

void main(){
  FILE * fp = fopen("./ip.txt", "r");
  char ip[512];
  if (fp == NULL){
      printf("文本文件不存在\n");
      return;
  }
  while(fscanf(fp,"%[^\n]%*c\n", ip) != EOF){
    printf("%s\n", ip);
    printf("%d\n", strlen(ip));
    //forbid_ip_number++;
  }
  fclose(fp);
  return;
}
