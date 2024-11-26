#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <mysql/mysql.h>

#define BUF_SIZE 100
#define NAME_SIZE 40
#define ARR_CNT 6

void* send_msg(void* arg);
void* recv_msg(void* arg);
void error_handling(char* msg);

char name[NAME_SIZE] = "[Default]";
char msg[BUF_SIZE];

int main(int argc, char* argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread, mysql_thread;
    void* thread_return;

    if (argc != 4) {
        printf("Usage : %s <IP> <port> <name>\n", argv[0]);
        exit(1);
    }

    sprintf(name, "%s", argv[3]);

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr*) & serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    sprintf(msg, "[%s:PASSWD]", name);
    write(sock, msg, strlen(msg));
    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
    pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);

    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);

    close(sock);
    return 0;
}

void* send_msg(void* arg)
{
    int* sock = (int*)arg;
    int str_len;
    int ret;
    fd_set initset, newset;
    struct timeval tv;
    char name_msg[NAME_SIZE + BUF_SIZE + 2];

    FD_ZERO(&initset);
    FD_SET(STDIN_FILENO, &initset);

    fputs("Input a message! [ID]msg (Default ID:ALLMSG)\n", stdout);
    while (1) {
        memset(msg, 0, sizeof(msg));
        name_msg[0] = '\0';
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        newset = initset;
        ret = select(STDIN_FILENO + 1, &newset, NULL, NULL, &tv);
        if (FD_ISSET(STDIN_FILENO, &newset))
        {
            fgets(msg, BUF_SIZE, stdin);
            if (!strncmp(msg, "quit\n", 5)) {
                *sock = -1;
                return NULL;
            }
            else if (msg[0] != '[')
            {
                strcat(name_msg, "[ALLMSG]");
                strcat(name_msg, msg);
            }
            else
                strcpy(name_msg, msg);
            if (write(*sock, name_msg, strlen(name_msg)) <= 0)
            {
                *sock = -1;
                return NULL;
            }
        }
        if (ret == 0)
        {
            if (*sock == -1)
                return NULL;
        }
    }
}

void* recv_msg(void* arg)
{
    MYSQL* conn;
    MYSQL_ROW sqlrow;
    int res;
    char sql_cmd[200] = { 0 };
    char* host = "localhost";
    char* user = "iot";
    char* pass = "pwiot";
    char* dbname;  // 데이터베이스 이름은 사용자 이름에 따라 결정

    int* sock = (int*)arg;
    int i;
    char* pToken;
    char* pArray[ARR_CNT] = { 0 };

    char name_msg[NAME_SIZE + BUF_SIZE + 1];
    int str_len;

    int illu;
    double temp;
    double humi;
    conn = mysql_init(NULL);

    puts("MYSQL startup");
    if (!(mysql_real_connect(conn, host, user, pass, NULL, 0, NULL, 0)))  // 처음에 DB 연결할 때 데이터베이스를 지정하지 않음
    {
        fprintf(stderr, "ERROR : %s[%d]\n", mysql_error(conn), mysql_errno(conn));
        exit(1);
    }
    else
        printf("Connection Successful!\n\n");

    while (1) {
        memset(name_msg, 0x0, sizeof(name_msg));
        str_len = read(*sock, name_msg, NAME_SIZE + BUF_SIZE);
        if (str_len <= 0)
        {
            *sock = -1;
            return NULL;
        }
        name_msg[str_len] = 0;
        fputs(name_msg, stdout);

        pToken = strtok(name_msg, "[:@]");  // `:`, `@` 구분자를 사용하여 토큰 분리
        i = 0;
        while (pToken != NULL)
        {
            pArray[i] = pToken;
            if (++i >= ARR_CNT)
                break;
            pToken = strtok(NULL, "[:@]");  // 토큰 계속해서 분리
        }
		// printf("i: %d\n", i);
		// printf("pArray[2] : %s\n", pArray[2]);
		// printf("pArray[5] : %s\n", pArray[5]);
		
		
        if (!strcmp(pArray[2], "SENSOR") && (i == 6)) {
            illu = atof(pArray[3]);         // 조도 값
            temp = (int)(atof(pArray[4]) * 0.95 + 0.5);  // 온도 값 (보정)
            humi = atof(pArray[5]);         // 습도 값
			printf("test : %d\n", i);
            // 현재 날짜와 시간 구하기
            time_t t = time(NULL);
            struct tm tm_info;
            char date_str[20], time_str[20];
            localtime_r(&t, &tm_info);

            strftime(date_str, sizeof(date_str), "%Y-%m-%d", &tm_info);  // 날짜
            strftime(time_str, sizeof(time_str), "%H:%M:%S", &tm_info);  // 시간
			

            // 사용자 아이디에 맞는 데이터베이스 선택
            if (strcmp(pArray[1], "user1") == 0) {
                dbname = "SensorData_user1";  // user1 -> SensorData_user1
            }
            else if (strcmp(pArray[1], "user2") == 0) {
                dbname = "SensorData_user2";  // user2 -> SensorData_user2
            }
            else if (strcmp(pArray[1], "user3") == 0) {
                dbname = "SensorData_user3";  // user3 -> SensorData_user3
            }
            else if (strcmp(pArray[1], "user4") == 0) {
                dbname = "SensorData_user4";  // user4 -> SensorData_user4
            }
            else {
                // 기본적으로 user1 데이터베이스 사용
                dbname = "SensorData_user1";
            }

            // 해당 데이터베이스로 변경
            if (mysql_select_db(conn, dbname)) {
                fprintf(stderr, "ERROR : %s[%d]\n", mysql_error(conn), mysql_errno(conn));
                exit(1);
            }

            // sensor_data 테이블에 데이터 삽입
            sprintf(sql_cmd, "INSERT INTO sensor_data (date, time, gas, humi) VALUES ('%s', '%s', %d, %d)",
                    date_str, time_str, illu, (int)humi);

            res = mysql_query(conn, sql_cmd);
            if (!res)
                printf("inserted %lu rows\n", (unsigned long)mysql_affected_rows(conn));
            else
                fprintf(stderr, "ERROR: %s[%d]\n", mysql_error(conn), mysql_errno(conn));
        }
        else 
            continue;
    }
    mysql_close(conn);
}

void error_handling(char* msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
