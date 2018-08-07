# include<stdio.h>
# include<stdlib.h>

void usage(){
  printf("欢迎使用TFTP服务器端！\n");
  printf("--------------------------------------------\n");
  printf("使用指南：\n");
  printf("\nUsage:  [port]\n");
  printf("可用端口号:1024-65535\n");
  printf("如果你未设置服务器端的端口号,端口号默认为7341。\n");

}


int main(int argc, char **argv){
    usage();
    return 1;
}
