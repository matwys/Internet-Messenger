#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 4444
char buffer[128];
char wbuffer[16];

struct user{
    char login[128]; 
    char warunek[128];
    char file[128];
};
    struct user arr_user[6] = {
                                {"","1.txt","f1.txt"},
                                {"","2.txt","f2.txt"},
                                {"","3.txt","f3.txt"},
                                {"","4.txt","f4.txt"},
                                {"","5.txt","f5.txt"},
                                {"","6.txt","f6.txt"}
                            };

int ReadLoginsFromFile(){
    FILE *filePointer;
    char ch;
    filePointer = fopen("loginList", "r");
    if (filePointer == NULL)
    {
        printf("File is not available \n");
        return 1;
    }
    else
    {
        for(int i = 0; i < 6; i++){
            for(int j = 0; j < 6; j++){
                ch = fgetc(filePointer);
                if( ch == EOF || ch == '\n' || ch == ' ' || j == 5){
                    arr_user[i].login[j]= '\0';
                    break;
                } else {
                    arr_user[i].login[j]=ch;
                }
            }
        }
    }
    fclose(filePointer);
    printf("Dostępne loginy:\n");
    for(int i = 0; i < 6; i++){
        printf("%s\n", arr_user[i].login);
    }
    return 0;
}
void MakeFile(){
    FILE * fp;
    for(int i = 0; i<6;i++){
        fp = fopen(arr_user[i].warunek, "w");
        fprintf(fp, "%s", "0");
        fclose(fp);
        fp = fopen (arr_user[i].file, "w");
        fprintf(fp, "%s", " ");
        fclose(fp);
    }
}
void Zalogowany0(int n){
    FILE * fp;
    fp = fopen(arr_user[n].warunek, "w");
    fprintf(fp, "%s", "0");
    fclose(fp);
}
void Zalogowany1(int n){
    FILE * fp;
    fp = fopen(arr_user[n].warunek, "w");
    fprintf(fp, "%s", "1");
    fclose(fp);
}

int CzyZalogowany(int n){
    FILE* fp = fopen(arr_user[n].warunek, "r");
 
    // Number of characters read
    size_t num_read = fread(wbuffer, sizeof(char), 8, fp);
     
    // We must explicitly NULL terminate the buffer with '\0'
    wbuffer[num_read] = '\0';
    fclose(fp);
    if(strcmp(wbuffer, "0") == 0) return 0;
    else return 1;
}
void Dopisz(int nadawca,int odbiorca){
    FILE * fp;
    fp = fopen (arr_user[odbiorca].file, "a");
    fprintf(fp, "%s:\n", arr_user[nadawca].login);
    fprintf(fp, "%s\n\n", buffer);
    fclose(fp);
}
void Odczyt(int odbiorca, int for_i){
    FILE * fp;
    fp = fopen (arr_user[odbiorca].file, "r");
    fseek( fp, 128*for_i, SEEK_SET );
    size_t num_read = fread(buffer, sizeof(char), 128, fp);    
    buffer[num_read] = '\0';
    fclose(fp);
}

void Wyczysc(int odbiorca){
    FILE * fp;
    fp = fopen (arr_user[odbiorca].file, "w");
    fprintf(fp, "%s", " ");
    fclose(fp);
}

int WielkoscSchowka(int odbiorca){
    FILE * fp;
    fp = fopen (arr_user[odbiorca].file, "r");
    fseek(fp, 0, SEEK_END);
    int lengthOfFile = ftell(fp);
    fclose(fp);
    return lengthOfFile;
}
void Odbieranie(){
}

void Wysylanie(){
}

int main(){
    if(0<ReadLoginsFromFile()){
        exit(1);
    }
    MakeFile();
    
	int sockfd, ret;
	 struct sockaddr_in serverAddr;

	int newSocket;
	struct sockaddr_in newAddr;

	socklen_t addr_size;

	
	pid_t childpid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Server Socket is created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	//serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in binding.\n");
		exit(1);
	}
	printf("[+]Bind to port %d\n", 4444);

	if(listen(sockfd, 10) == 0){
		printf("[+]Listening....\n");
	}else{
		printf("[-]Error in binding.\n");
	}


	while(1){
		newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
		if(newSocket < 0){
			exit(1);
		}
		printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

		if((childpid = fork()) == 0){
			close(sockfd);

            //logowanie
            int id = -1;
            int zalogowany = 0;
			while(id == -1){
                bzero(buffer, sizeof(buffer));
				recv(newSocket, buffer, 128, 0);
                int komunikat = 0;
                for(int i = 0;i<6;i++){
                    if( strcmp( buffer, arr_user[i].login) == 0){
                        if(CzyZalogowany(i)){
                            send(newSocket, "j", strlen("j"), 0);
                            komunikat = 1;
                        } else {
                            Zalogowany1(i);
                            id = i;
                            send(newSocket, "1", strlen("1"), 0);
                            zalogowany = 1;
                        }
                    }
                }
				if(strcmp(buffer, ":exit") == 0){
					printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
					break;
				}
                if(id == -1 && komunikat == 0){
                    bzero(buffer, sizeof(buffer));
                    send(newSocket, "z", strlen("z"), 0);
					bzero(buffer, sizeof(buffer));
				}
			}

			while(1 == zalogowany){
				recv(newSocket, buffer, 128, 0);
				if(strcmp(buffer, ":exit") == 0){
					printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                    Zalogowany0(id);
                    zalogowany = 0;
					break;
				}
                else if(strcmp(buffer, ":send") == 0){
					send(newSocket, "1", strlen("1"), 0);
                    int id_odpiorcy = -1;
                    while(id_odpiorcy == -1){
                        bzero(buffer, sizeof(buffer));
                        recv(newSocket, buffer, 128, 0);
                        for(int i = 0;i<6;i++){
                            if( strcmp( buffer, arr_user[i].login) == 0){
                                id_odpiorcy = i;
                                if(WielkoscSchowka(id_odpiorcy) < 8000){
                                    send(newSocket, "check", strlen("check"), 0);
                                    bzero(buffer, sizeof(buffer));
                                    recv(newSocket, buffer, 128, 0);//dane do zapisu
                                    Dopisz(id,id_odpiorcy);
					                send(newSocket, "1", strlen("1"), 0);
                                    bzero(buffer, sizeof(buffer));
                                } else {
                                    send(newSocket, "ow", strlen("ow"), 0);
                                    bzero(buffer, sizeof(buffer));
                                }
                                break;
                            }
                        }
                        if(id_odpiorcy == -1){
                            send(newSocket, "0", strlen("0"), 0);
                        }
                    }
				}
                else if(strcmp(buffer, ":recv") == 0){
                    int for_i = (WielkoscSchowka(id)-1) /128;
                    Odczyt(id, 0);
                    send(newSocket, buffer, strlen(buffer), 0);
                    bzero(buffer, sizeof(buffer));
                    recv(newSocket, buffer, 128, 0);//get "check" or "end"
                    if(strcmp(buffer, "end")==0){
                        send(newSocket, buffer, strlen(buffer), 0);
                        bzero(buffer, sizeof(buffer));
                    } else {
                        bzero(buffer, sizeof(buffer));
                        for(int i =1; i<for_i+1;i++){
                            send(newSocket, "true", strlen("true"), 0);
                            recv(newSocket, buffer, 128, 0); //get "check"
                            bzero(buffer, sizeof(buffer));
                            Odczyt(id, i);
                            send(newSocket, buffer, strlen(buffer), 0);
                            bzero(buffer, sizeof(buffer));
                            recv(newSocket, buffer, 128, 0); //get "check"
                            bzero(buffer, sizeof(buffer));
                        }
                        send(newSocket, "false", strlen("false"), 0);
                        Wyczysc(id);
                    }
                }
			}
		}

	}

	close(newSocket);


	return 0;
}