#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
// 定数・変数定義
#define MAX_PROG_SIZE 65536
#define ESC 27
#define LF   10
#define CR  13
char fname[4096], buf[4096], prog[MAX_PROG_SIZE], inbuf;
int fd, fd0, len, is_run, i;
struct termios tio, temp_term, save_term;
speed_t BAUDRATE = B9600;
char *s0 = "\rprog=\"";
char *s1 = "\";\rf = open(\"./main.py\", \"w\")\rf.write(prog)\rf.close()\r";


// "picoterm.c" ラズピコ用のシンプルなプログラム開発ツール
// コンパイル：
// gcc -o picoterm picoterm.c

// 使用法：
// ./picoterm (USBシリアルデバイスファイル名)
// PCとラズピコをUSB接続して上記のコマンドを入力すると
// MicroPythonのコンソールが表示される。
// そのときにESCキーを押すとファイルのアップロード又は終了を
// 選択することができる。
 
void _PSTART();
void _1741608787_in();
void _583816654_in();
int ARGC;
char **ARGV;
int main(int argc, char** argv){
ARGC = argc;
ARGV = argv;
_PSTART();
return 0;
}
void _PSTART(){
_583816654_in();
_1741608787_in();
}
void _1741608787_in(){
// ターミナルモード
  if(ARGC<2) return;


  // 現在の端末設定を取得
  if(tcgetattr(fileno(stdin), &save_term) == -1){
    printf("tcgetattr failure\n");
    goto exit1;
  }
  else{
    temp_term = save_term;
  }

  // 端末設定

  // 受信したCRを無視
  temp_term.c_iflag &= IGNCR;

  // カノニカルモードを外す
  temp_term.c_lflag &= ~ICANON;

  // 入力をエコーしない
  temp_term.c_lflag &= ~ECHO;

  // シグナルを無効化
  temp_term.c_lflag &= ~ISIG;

  // 何文字受け取ったらreadが返るか
  temp_term.c_cc[VMIN] = 1;

  // 何秒経ったらreadが返るか
  temp_term.c_cc[VTIME] = 0;

  if(tcsetattr(fileno(stdin), TCSANOW, &temp_term) == -1){
    printf("tcsetattr(temp_term) failure\n");
    goto exit1;
  }

  // Open modem device for reading and writing */
  fd=open(ARGV[1], O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd < 0) {printf("open error\n"); return;}

  //load configuration
  tcgetattr(fd,&tio);
  tio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  cfsetispeed(&tio, BAUDRATE);  //set baudrate
  cfsetospeed(&tio, BAUDRATE);
  tio.c_lflag &= ~(ECHO | ICANON);  //non canonical, non echo back
  tio.c_cc[VMIN]=0;  //non blocking
  tio.c_cc[VTIME]=0;
  cfmakeraw(&tio);  // RAWモード
  tcsetattr(fd,TCSANOW,&tio);  //store configuration


  // clean the modem line and activate the settings for the port
  tcflush(fd, TCIFLUSH);
  ioctl(fd, TCSETS, &tio);

printf("start picoterm:\n");

  // 送受信処理ループ
  is_run = 1;
  while(is_run) {
    do{
      len = read(fd, buf, sizeof(buf));
      if(len > 0){
        write(1, buf, len);
      }
     } while(len > 0);
     inbuf = getchar();
     fflush(stdin);

     // ESCコマンド
     if(inbuf==ESC){
       char c;
       printf("\n*** ESC Command ***\nu: upload program\nq: quit\ncommand? ");
       c = getchar();printf("%c\n", c);
       switch(c){
       
       // プログラムのアップロード
       case 'u':
         printf("*** UPLOAD \"main.py\" ***\nfile name? ");
         scanf("%s", fname); printf("%s\n", fname);
         fd0 = open(fname, O_RDONLY);
         if(fd0 == -1){
           printf("file open error\n");
         }
         else{
           *prog = '\0';
           i = read(fd0, prog, MAX_PROG_SIZE);
           close(fd0);
           if(i >= 0){
             write(fd, s0, strlen(s0));
             len = read(fd, buf, sizeof(buf));
             prog[i] = '\0';
             for(i = 0; prog[i] != '\0'; i++){
               char esc = '\\';
               char d0 = ((int)(prog[i] >> 6) & 0x03) + '0';
               char d1 = ((int)(prog[i] >> 3) & 0x07) + '0';
               char d2 = ((int)(prog[i] >> 0) & 0x07) + '0';
               write(fd, &esc, 1);
               write(fd, &d0, 1);
               write(fd, &d1, 1);
               write(fd, &d2, 1);
               usleep(1000);
               len = read(fd, buf, sizeof(buf));
             }
             write(fd, s1, strlen(s1));
             len = read(fd, buf, sizeof(buf));
           }
         }
         break;
         
       // 終了
       case 'q':
         is_run = 0;
         break;
         
       default:
         printf("unknown command.\n");
       }
       fflush(stdin);
     }

     // 通常の文字入力
     else{
       write(fd, &inbuf, 1);
       if(inbuf == LF){
         inbuf=CR;
         write(fd, &inbuf, 1);
       }
     }
     usleep(30000);
  }

  printf("\n*** QUIT ***\n");
  // デバイスのクローズ
  close(fd);

  // 端末設定をもとに戻す
  exit1:
  if(tcsetattr(fileno(stdin), TCSANOW, &save_term) == -1){
    printf("tcsetattr(save_term) failure\n");
  }
  exit(0);
}
void _583816654_in(){
// コマンド実行モード
  if(ARGC<4) return;
  if(strcmp(ARGV[2],"--command")!=0) return;


  // Open modem device for reading and writing */
  fd=open(ARGV[1], O_RDWR | O_NOCTTY | O_NONBLOCK);
  if(fd < 0) {printf("open error\n"); return;}

  //load configuration
  tcgetattr(fd,&tio);
  tio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  cfsetispeed(&tio, BAUDRATE);  //set baudrate
  cfsetospeed(&tio, BAUDRATE);
  tio.c_lflag &= ~(ECHO | ICANON);  //non canonical, non echo back
  tio.c_cc[VMIN]=0;  //non blocking
  tio.c_cc[VTIME]=0;
  cfmakeraw(&tio);  // RAWモード
  tcsetattr(fd,TCSANOW,&tio);  //store configuration


  // clean the modem line and activate the settings for the port
  tcflush(fd, TCIFLUSH);
  ioctl(fd, TCSETS, &tio);

  for(i = 3; i < ARGC; i++){
    int j, l;
    l = strlen(ARGV[i]);
    for(j = 0; j < l; j++){
      inbuf = ARGV[i][j];
      write(fd, &inbuf, 1);
    }
    inbuf = ' ';
    write(fd, &inbuf, 1);
  }
  inbuf=CR;
  write(fd, &inbuf, 1);

  // デバイスのクローズ
  close(fd);
  usleep(1000000);

  exit(0);
}
