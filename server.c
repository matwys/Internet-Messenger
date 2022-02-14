#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define MAX_NUM_OF_USERS 20
#define PORT 4444
char buffer[128];
char wbuffer[16];
int num_of_users;


struct user{
    char login[128]; 
    char warunek[128];
    char file[128];
    char passwd[128];
};


struct user arr_user[MAX_NUM_OF_USERS];


int ReadLoginsFromFile(){
	// Wczytanie loginów i haseł z pliku loginList
	FILE *filePointer;
	char ch;
	filePointer = fopen("loginList", "r");
	if (filePointer == NULL) {
		printf("File 'loginList' is not available.\n");
		return 1;
	}
	else {
		int i = 0, j = 0, s = 0;
		int log_or_passwd = 0;
		while ((ch = fgetc(filePointer)) != EOF) {
			if (ch == ' ') {
				strcpy(arr_user[i].warunek, strcat(arr_user[i].warunek, "_warunek"));
				strcpy(arr_user[i].file, strcat(arr_user[i].file, "_file"));
				log_or_passwd = 1;
				s = j;
			} else if (ch == '\n') {
				i++;
				j = 0;
				log_or_passwd = 0;
			} else if (log_or_passwd == 0) {
				arr_user[i].login[j] = ch;
				arr_user[i].warunek[j] = ch;
				arr_user[i].file[j] = ch;
				j++;
			} else if (log_or_passwd == 1) {
				arr_user[i].passwd[j-s] = ch;
				j++;
			}
		}
		num_of_users = i;
	}
	fclose(filePointer);
	printf("=== Dostępne loginy i hasła: ===\n");
	for(int i = 0; i < num_of_users; i++){
		printf("%s %s\n", arr_user[i].login, arr_user[i].passwd);
	}
	printf("=================================\n");
	return 0;
}

void MakeFile(){
    FILE * fp;
    for(int i = 0; i < num_of_users; i++){
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

			// przed zalogowaniem
			int id = -1;
			int zalogowany = 0;
			while(id == -1){
				bzero(buffer, sizeof(buffer));
				recv(newSocket, buffer, 128, 0);
				int komunikat = 0;
				if(strcmp(buffer, ":exit") == 0){
					printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
					break;
				}
				/* 
				   LOGOWANIE
				   Odbiera nazwę użytkownika i wysyła wynik logowania:
				   j - ktoś inny już jest zalogowany pod tym samym loginem, 
				   pl - poprawny login, nl - niepoprawny login,
				   ph - poprawne haslo, nh - niepoprawne hasło;
				*/
				for(int i = 0; i < num_of_users; i++){
					if( strcmp( buffer, arr_user[i].login) == 0){
						if(CzyZalogowany(i)){
							send(newSocket, "j", strlen("j"), 0);
							komunikat = 1;
						} else {
							send(newSocket, "pl", strlen("pl"), 0);
							bzero(buffer, sizeof(buffer));
							recv(newSocket, buffer, 128, 0);
							if (strcmp(buffer, arr_user[i].passwd)){
								send(newSocket, "nh", strlen("nh"), 0);
								komunikat = 1;
							} else {
								Zalogowany1(i);
								id = i;
								zalogowany = 1;
								send(newSocket, "ph", strlen("ph"), 0);
							}
						}
					}
				}
				if(id == -1 && komunikat == 0){
					bzero(buffer, sizeof(buffer));
					send(newSocket, "nl", strlen("nl"), 0);
					bzero(buffer, sizeof(buffer));
				}
			}
			// po zalogowaniu
			while(zalogowany == 1){
				bzero(buffer, sizeof(buffer));
				recv(newSocket, buffer, 128, 0);
				if(strcmp(buffer, ":exit") == 0){
					printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
					Zalogowany0(id);
					zalogowany = 0;
					break;
				}
				// WYSYŁANIE WIADOMOŚCI
				else if(strcmp(buffer, ":send") == 0){
					send(newSocket, "1", strlen("1"), 0);
					int id_odpiorcy = -1;
					while(id_odpiorcy == -1){
						bzero(buffer, sizeof(buffer));
						recv(newSocket, buffer, 128, 0);
						for(int i = 0; i < num_of_users; i++){
                            				if( strcmp( buffer, arr_user[i].login) == 0){
								id_odpiorcy = i;
								if(WielkoscSchowka(id_odpiorcy) < 8000){
 									send(newSocket, "check", strlen("check"), 0);
									bzero(buffer, sizeof(buffer));
									recv(newSocket, buffer, 128, 0); //dane do zapisu
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
				// ODBIERANIE WIADOMOŚCI
				else if(strcmp(buffer, ":recv") == 0){
					int for_i = (WielkoscSchowka(id)-1) /128;
					Odczyt(id, 0);
					send(newSocket, buffer, strlen(buffer), 0);
					bzero(buffer, sizeof(buffer));
					recv(newSocket, buffer, 128, 0); //get "check" or "end"
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

