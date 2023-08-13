#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

// 2020A8PS0480H	Caris Abishek I
// 2020A8PS2219H	Rajendra Grandhi
// 2020A8PS2220H	Kashyap Kannan
// 2020A8PS2132H	Harshal Dhagale
// 2020A4PS1979H	Mohammed Sohail Rafeeq
// 2020AAPS1735H	Harish Yuvaraj G P
// 2020A3PS2206H 	Mohammad Ali 
// 2020A3PS0487H	Mohammed Abdul Afreed

struct channel{
	int rw;
	char request[100];
	char response[100];
};

struct channel_comm{
	int rw;
	char name[100];
	int request[100];
	int response[100];
};

char requests[100][100];
pthread_t threadIds[100];
int requestcount=0;
int globalrequestcount = 0;
int connect_id;
struct channel* connect_channel;
bool flag =0;


int search(char* s){
	printf("Searching...\n");
	printf("%s\n", requests[requestcount]);
	for(int i=0;i<requestcount;i++){
		// printf("%d\n", strcmp(requests[i], s));
		if(strcmp(requests[i], s) == 0){
			perror("Invalid request or already joined.\n");
			strcpy(connect_channel->response,"INVALID");
			sleep(1);
			return 0;
		}
	}
	strcpy(requests[requestcount], s);
	return 1;
}

int search_present(char* s){
	printf("Searching...\n");
	
	for(int i=0;i<requestcount;i++){
		
		if(strcmp(requests[i], s) == 0){
			
			return 0;
		}
	}

	return 1;
}

bool isprime(int x){
	int c=0;
    if(x==1){return false;}
	for(int i=1; i<x; i++){
		if(x%i==0) c++;
		if(c==2) return false;
	}
    return true; 
}
 
bool evenodd(int y){
	return y%2;
}

int arithmetic(int a, int b, char c){
	printf("%d, %c, %d\n", a, c, b);
	switch(c){
    	case '+':
		return a+b;
    	case '-':
		return a-b;
    	case '/':
 		if(b!=0) 
    		return a/b;
        break;
    	case '*':
        return a*b;
    	default:
        printf("No.\n");       
	}
    return INT_MIN;
}

void exitcommclient(int* commId, struct channel_comm** communication){
	shmdt(*communication);
	
	shmctl(*commId, IPC_RMID, NULL);
	
	printf("Cleared all association (comm channel) for the client.\n");
}



void initializer(int* commId,  struct channel_comm** communication){
	printf("Reached initializer\n");


	*commId = shmget((key_t)2000 +requestcount, sizeof(struct channel), 0666|IPC_CREAT);
	if(*commId <0){
		perror("Id not created\n");
		exit(1);
	}
	printf("Key of commId: %d\n", *commId);
	*communication = (struct channel_comm*)shmat(*commId, NULL, 0);
	if(*communication == (struct channel_comm *) -1){
		perror("Shared Memory not created.");
		exit(1);
	}
	// printf("%p\n", &communication);

	printf("Initialized.\n");
}


void* worker(void* ok){
	int commId;
	int comm_request_count =0;
	int key = requestcount;
	struct channel_comm * comm_channel;
	printf("The variables are initialized\n");
	initializer(&commId, &comm_channel);
	printf("Thread created.\n");
	printf("Thread number: %d\n", key);
	while(strcmp(connect_channel->request, "send")!=0) sleep(1);
	printf("sending id...\n");
	printf("sending commId: %d\n", commId);
	*(int*)connect_channel->response = commId;
	sleep(1);
	printf("Client register request Success\n");
	strcpy(connect_channel->request, "");
	flag = false;
	comm_channel->rw = 0;
	strcpy(comm_channel->name,"");
	printf("Total no. of comm request counts =%d \n", comm_request_count);
	while(1){
		printf("Inside comm request, checking...\n");
		while(strlen(comm_channel->name)==0) sleep(1);
		printf("New request recieved\n");
		comm_request_count++;
		globalrequestcount++;
		//printf("%s\n", comm_channel->name);
		if(strcmp(comm_channel->name, "exit")==0){
			printf("Exiting requests.\n"); 
			printf("Cleaning up comm channel of %s \n", requests[key]);
			exitcommclient(&commId, &comm_channel);
			break;
		}
		if(search_present(comm_channel->name)==0) printf("VALID REQUEST\n");
		else printf("INVALID\n");
		printf("Responding to the request\n");
		int res;
		switch(comm_channel->rw){
		case -404:
			comm_channel->response[0] = -404;
			// comm_channel->rw = 0;
			printf("Wrong option selected from the client\n");
			break;
		case 1:
			printf("%d, %c, %d\n", comm_channel->request[0],(char) comm_channel->request[2], comm_channel->request[1]);
			res = arithmetic(comm_channel->request[0], comm_channel->request[1],(char) comm_channel->request[2]);
			comm_channel->response[0] = res;

			break;
		case 2:
			printf("%d\n", comm_channel->request[0]);
			res = (int)evenodd(comm_channel->request[0]);
			comm_channel->response[0] = res;
			break;
		case 3:
			printf("%d\n", comm_channel->request[0]);
			res = (bool)isprime(comm_channel->request[0]);
			comm_channel->response[0] = res;
			break;

		
		default:
			printf("Not supported, returning.\n");
		}
		
		
		
		strcpy(comm_channel->name,"");
		// printf("%p \n", comm_channel->response);
		// char* message = "Hello, here is your reply.";
		// strcpy(comm_channel->response, message);
		// comm_channel->response[0] = 200;
		printf("Reply sent. (%d)\n", comm_channel->response[0]);
		comm_channel->rw = 0;
		printf("Request count = %d\n", comm_request_count);
		// while(strcmp(comm_channel->response, message) == 0) sleep(1);
		
		
		// strcpy(comm_channel->request,"");
		// strcpy(comm_channel->response, "");

	}


}

void newclientconnect(){
	printf("Waiting for connect requests...\n");
	strcpy(connect_channel->request,"");
	strcpy(connect_channel->response, "");
	// printf("%d\n", connect_id);
	while(strlen(connect_channel->request) == 0){sleep(1);} 
	
	if(search((char*)connect_channel->request) ==1){
		printf("Recieved client request\n");
		strcpy(connect_channel->response, "I have acknowledged you");
		 
		printf("client request count: %d\n", requestcount);
		flag = true;

		int check = pthread_create(&threadIds[requestcount], NULL, worker, NULL);
		strcpy(connect_channel->request,"");
		while(flag)sleep(1);
		// pthread_join(threadIds[requestcount], NULL);
		if(check){ 
			printf("Failed creating thread for client.\n");
			printf("Client register request unsuccessful\n");
			exit(1);
		}
		
		requestcount++;
		
		pthread_join(threadIds[requestcount], NULL);
		printf("Going to parent thread.\n");
		strcpy(connect_channel->request,"");
		

	}
	// strcpy(connect_channel_rw, "0");

}


void printall(){
	printf("All past and present clients\n");
	for(int i =0;i<requestcount;i++){
		printf("%s ", requests[i]);
	}
	printf("\n");

}

void exitall(){
	shmdt(connect_channel);
	
	shmctl(connect_id, IPC_RMID, NULL);
	
}

void sighandler(int sig_num)
{
    
    signal(SIGTSTP, sighandler);
    printf(" manually exiting.\n");
    exitall();
    exit(0);
}

int main(){
	printf("Process ID: %d\n", (int)getpid());
	connect_id = shmget((key_t)1110, sizeof(struct channel), 0666|IPC_CREAT);
	if(connect_id <0){
		perror("Id not created\n");
		exit(1);
	}


	connect_channel= shmat(connect_id, NULL, 0);
	if(connect_channel == (void *) -1){
		perror("Shared Memory not created.\n");
		exit(1);
	}
        printf("Created and initiated connect channel\n");
	printf("connect id: %d\n", connect_id);
	printf("Connect channel address : %p\n" ,(void *)&connect_channel);
	
	signal(SIGTSTP, sighandler);
	while(1)
		{	
			newclientconnect();
			printall();
			printf("Global client request counts : %d \n", globalrequestcount);
			printf("Repeating\n");
	}
	for(int i =0;i<requestcount;i++){
		pthread_join(threadIds[i], NULL);
	}
	exitall();

	return 0;
}
