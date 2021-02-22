#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <semaphore.h>

#define PORT_NO 3205

sem_t cont;
int control = 0;
int socket_id;
char username[100];
char group_name[100]; 
void *Receive(void * sockID){ //This function is used to recieve messages from server.
	int socket_id = *((int *) sockID);
	while(1){
		char rec[10000];
		int final = recv(socket_id,rec,10000,0);
		rec[final] = '\0';
		printf("%s\n",rec);	
	
		if(strcmp(rec,"You exit the program.\n")==0){ //socket is now free, control has to become 1 to end the iteration.
			control = 1;
			sem_post(&cont);
			break;
		}
		
		else if(strstr(rec,"-username")!=NULL){

			char temp[100];
			char temp2[100];
			strcpy(temp, rec);
			strcpy(temp2,strtok(temp, " "));
			strcpy(temp2,strtok(NULL, " "));
			strcpy(username, temp2);
	
		}
		else if(strstr(rec,"-gname")!=NULL){

			char temp[100];
			char temp2[100];
			strcpy(temp, rec);
			strcpy(temp2,strtok(temp, "***"));
			strcpy(temp2,strtok(NULL, "***"));
			strcpy(group_name, temp2);
	
		}
	}
}


int main(){
	sem_init(&cont,0,0);//this semaphore is used to control the exits
	socket_id = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT_NO);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(socket_id, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) return 0;

	printf("Connection established ............\n");

	pthread_t thread;
	pthread_create(&thread, NULL, Receive, (void *) &socket_id );

    strcpy(group_name, "no_group");

	while(1){
		char input[100];
		gets(input);
		if((strstr(input, "-send") != NULL) && (strcmp(group_name, "no_group") != 0)){
			char json[100];
			sprintf(json, "{\"from\":\"%s\",\"to\":\"%s\", \"message\":\"%s\"}", username, group_name, strstr(input," "));
			sprintf(input, "-send %s",json);
			send(socket_id,input,100,0);
            
		}else if ((strstr(input, "-send") != NULL) &&(strcmp(group_name, "no_group") == 0))
        {
            printf("You are not in any group. You cannot send any message.\n");
        }
        else if (strcmp(input, "-whoami") == 0)
        {
            printf("You are %s.\n", username);
        }
        else if (strcmp(input, "-exit") == 0) //if the command is exit, semaphore is become "wait".
        {
             
            send(socket_id,input,100,0);
            printf("Have a nice day!\n");
            sem_wait(&cont);
            exit(1);
        }
        else{
			send(socket_id,input,100,0);
		}

		if(control == 1){ //If control turns 1, iteration finishes.
			pthread_join(thread,NULL);
			break;
		}
	}
	return 0;
}
