# include<stdlib.h>
# include<stdio.h>
# include<string.h>

#define DATASIZE 512
char *list = ".";
char buffer[1024];
/*
test c 文件
 */

void test(char *filename);
int main(){
  struct tftp_packet{
    unsigned short optcode;   //操作码
    unsigned short optcode1;
    union{
        unsigned short block;  //文件中的块号
        unsigned short errcode;//差错码
        char filename[2];      //指针数组分别指向文件名和mode
    };
    char data[DATASIZE];       //报文中用来存储数据的字符数组

  };
   struct TFTPHeader{
       short opcode;
    };

   struct TFTPWRRQ{
      struct TFTPHeader header;
      struct TFTPHeader header2;
      char *filename;
      char *mode;
      short tiem;
      short times;
    };



    struct test1{
      char *filename;
      char *mode;
      struct TFTPHeader header;
      struct TFTPHeader header1;
      struct TFTPHeader header2;
      struct TFTPHeader header4;
    };


    /*int len = sizeof(struct tftp_packet);
    int len2 = sizeof(struct TFTPWRRQ);
    int len3 = sizeof(struct test1);

    printf("WRRQ :%d\n",len);

    printf("HEADER:%d\n",len2);
    printf("test1:%d\n",len3);*/

    test("server.c");


    return 1;

}

void test(char *filename){
  char filepath[256];
  char content[1024];
  int blocksize = 10;
  memset(filepath, 0, sizeof(filepath));
  memset(buffer, 0, sizeof(buffer));
  strcpy(filepath, list);
  if (filename[0] != '/'){
      strcat(filepath, "/");
  }
  strcat(filepath, filename);
  printf("%s\n", filepath);
  /*FILE *fp = fopen(filepath, "r");
  if(fp == NULL){
      printf("File not exists!\n");
      return;
  }
  int s_size = fread(content, 1, blocksize, fp);
  printf("%d\n", s_size);
  printf("%s\n", content);*/
  FILE *fp_log = fopen("./server_log.txt", "a");
  if (fp_log == NULL){
      
      printf("打开日志文件失败,请检查.\n");
      return;
  }
  fprintf(fp_log, "客户端：%d 执行文件下载操作:%s\n",blocksize, "server.c");

  //fclose(fp);
  fclose(fp_log);
  return;
}
