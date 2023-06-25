#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <softPwm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVO 1
#define IN 0
#define OUT 1

#define LOW 0
#define HIGH 1
#define POUT_WHITE 4
#define POUT_RED 26

#define VALUE_MAX 40

#define CHANNEL 0 // 아날로그 입력 채널

#define PORT 7777
#define BUF_SIZE 1024

char em[2]="0";
int str_len;

bool press_flag = true;
int servo_command;

int fd;
unsigned char spiData[3];

static int GPIOExport(int pin) {
#define BUFFER_MAX 3
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open export for writing!\n");
        return(-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return(0);
}

static int GPIODirection(int pin, int dir) {
    static const char s_directions_str[]  = "in\0out";

#define DIRECTION_MAX 35
    char path[DIRECTION_MAX];
    int fd;

    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio direction for writing!\n");
        return(-1);
    }

    if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
        fprintf(stderr, "Failed to set direction!\n");
        return(-1);
    }

    close(fd);
    return(0);
}

static int GPIOWrite(int pin, int value) {
    static const char s_values_str[] = "01";

    char path[VALUE_MAX];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio value for writing!\n");
        return(-1);
    }

    if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
        fprintf(stderr, "Failed to write value!\n");
        return(-1);
    }

    close(fd);
    return(0);
}

static int GPIOUnexport(int pin) {
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open unexport for writing!\n");
        return(-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return(0);
}

static int GPIORead(int pin){
    char path[VALUE_MAX];
    char value_str[3];
    int fd;
    
    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_RDONLY);
    if(-1 == fd){
        fprintf(stderr, "Failed to open gpio value for reading!\n");
        return(-1);
    }
    
    if(-1 == read(fd, value_str, 3)) {
        fprintf(stderr, "Failed to read value!\n");
        close(fd);
        return(-1);
    }
    
    close(fd);
    
    return(atoi(value_str));
    
}

void error_handling(char *message){
    fputs(message,stderr);
    fputc('\n',stderr);
    exit(1);
}

static int servoControl(void) {
    //Enable GPIO pins
    if (-1 == GPIOExport (POUT_WHITE)) 
        return(1);
    
    //Set GPIO directions
    if (-1 == GPIODirection(POUT_WHITE, OUT)) 
        return(2); 

    float dir = 0.1;
    float pos = 0;
    softPwmCreate(SERVO, 0, 200);

    printf("Servo Control Start\n");
    
    GPIOWrite(POUT_WHITE, HIGH);
    while (pos < 90) {
        pos += dir;
        softPwmWrite(SERVO, pos);
        delay(10);
    }
    printf("Door Open\n");
    
    for (int i = 5; i > 0; i--) {
        printf("문이 닫히기 까지 %d초 남았습니다.\n", i);
        delay(1000);
    }

    while (pos > 0) {
        pos -= dir;
        softPwmWrite(SERVO, pos);
        delay(10);
    }
    printf("\nDoor Close\n");
    GPIOWrite(POUT_WHITE, LOW);
    printf("end Servo\n");
    return 0;
}

static int forcedOpen (void* arg) {
    int sock = (int)arg;

    printf("문이 강제 개방되었습니다!!\n");
    printf("문을 닫아주세요.\n");
        //Enable GPIO pins
    if (-1 == GPIOExport (POUT_RED)) 
        return(1);
    
    //Set GPIO directions
    if (-1 == GPIODirection(POUT_RED, OUT)) 
        return(2); 
    
    GPIOWrite(POUT_RED, HIGH);

    write(sock, em, sizeof(em));

    while(1) {
        char received_command[2];
        read(sock, received_command, sizeof(received_command));
        if (atoi(received_command) == 2)
            break;
    }
    printf("\n문열림 상황 종료\n");
    GPIOWrite(POUT_RED, LOW);
    
    return 0;
}

void *pressValuePrint (void *arg) {
    while(1) {
        while(press_flag) {
           spiData[0] = 0x01; // Start bit (항상 같음)
           spiData[1] = ((CHANNEL+8)<<4); // Channel 선택 (0~7)
           spiData[2] = 0x00; // Don't care (항상 같음)

           wiringPiSPIDataRW(CHANNEL, spiData, 3); // SPI 통신으로 데이터 수신

           int adcValue = ((spiData[1]&3) << 8) + spiData[2]; // 아날로그 값 계산

           printf("ADC value: %d\n", adcValue);
           delay(3000);

           if (adcValue < 10 && press_flag) {
                forcedOpen(arg);
                printf("forced open situation finished!!\n");
           }
        }
    }
}

void *pressCheck (void *arg) {
    char msg[2];

    int serv_sock, clnt_sock =1;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;

    // 서버 소켓 생성
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    // 서버 소켓 주소 설정
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi("7777"));

    // 서버 소켓에 주소 바인딩
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    // 클라이언트 연결을 수락
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    while(1) {
        
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        
        if (clnt_sock == -1)
            error_handling("accept() error");

        char received_command[2];
        read(clnt_sock, received_command, sizeof(received_command));
        servo_command = atoi(received_command);
        if (servo_command == 1) {
            press_flag = false;
            servoControl();
            press_flag = true;
        }
        servo_command = 0;
    }
}

int main(int argc, char *argv[]) {

    char msg[2];

    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    // 상황실 클라이언트 소켓 세팅
    clnt_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (clnt_sock == -1)
        error_handling("socket() error");

    // 서버 연결 시도
    if (connect(clnt_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    pthread_t t_press, t_press_check;
    fd = wiringPiSPISetup(CHANNEL, 1000000);

    if(fd == -1) {
        printf("SPI setup failed!");
        return 2;
    }

    if (wiringPiSetup() == -1) {
        printf("Wiring Pi setup failed!");
        return -1;
    }

    pthread_create(&t_press, NULL, pressValuePrint, (void*)clnt_sock);
    pthread_create(&t_press_check, NULL, pressCheck, NULL);

    pthread_join(t_press, NULL);
    pthread_join(t_press_check, NULL);

    close(clnt_sock);

    return 0;
}
