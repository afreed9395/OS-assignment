#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
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

struct channel
{
	int rw;
	char request[100];
	char response[100];
};

struct channel_comm
{
	int rw;
	char name[100];
	int request[100];
	int response[100];
};

int connect_id;
struct channel *connect_channel;

int commId;
struct channel_comm *comm_channel;

char name[100];

void connect()
{
	connect_id = shmget((key_t)1110, sizeof(struct channel), 0666);
	if (connect_id < 0)
	{
		perror("Id not recieved\n");
		exit(1);
	}

	printf("Key of connect_channel, connect_id: %d\n", connect_id);

	connect_channel = shmat(connect_id, NULL, 0);
	if (connect_channel == (void *)-1)
	{
		perror("Shared Memory not created.");
		exit(1);
	}

	
	printf("Enter client name: \n");

	scanf("%s", name);
	strcpy(connect_channel->request, name);
	printf("%s client requesting...\n", name);

	while (strlen((char *)connect_channel->response) == 0) sleep(1); 
	printf("Client has connected to the server\n");
	printf("reading response\n");

	if (strcmp((char *)connect_channel->response, "INVALID") == 0)
	{
		printf("Invalid client name. Not joined\n");
		strcpy(connect_channel->response, "");
		exit(1);
	}
	else
	{
		printf("Joined!\n");
	}
	printf("%s\n", (char *)connect_channel->response);
	sleep(1);
	strcpy(connect_channel->request, "send"); sleep(1);

	while (strcmp((char *)connect_channel->response, "I have acknowledged you") == 0) sleep(1);
	printf("%d\n", commId = *(int *)connect_channel->response);
	sleep(1);
}

void connect_comm_new()
{
	if(shmget((key_t)1110, sizeof(struct channel), 0666)<0){
			printf("Server closed, exiting...\n");
			return;
		}

	comm_channel = (struct channel_comm *)shmat(commId, NULL, 0);
	if (comm_channel == (struct channel_comm *)-1)
	{
		perror("Shared Memory not created.");
		exit(1);
	}
	printf("Client connected to the server using comm channel\n");
	// printf("comm_channel: %p\n", &comm_channel);
	comm_channel->rw = 0;
}

/*int menu(){
	int op=0;
	printf("Inside menu.\n");
	printf("Enter a number: \n");

	if(scanf("%d",&op)!=1){
		printf("ERROR: wrong input\n");

	}
	getchar();

	return op;
}*/
int menu()
{
	printf("Enter a number between 1 to 3, or -1.\n1: Arithmetic operation.\n2: Even or Odd.\n3: Prime Number Test.\n-1: Exit.\n\n");
	int op=0;
	printf("Enter a number: \n");
	scanf("%d",&op);
	getchar();
	// if(op >57 && op<48) return 0;
	return op;
	
}

void Arithmetic()
{
	int a, b;
	char operation;
	printf("Enter the equation in this format: \"n1 operation n2\"\n");
	printf("Valid operations: +, -, *, /\n");
	if (scanf("%d %c %d", &a, &operation, &b) != 3)
	{
		printf("ERROR: wrong input\n");
		return;
	}
	getchar();
	printf("%c\n", operation);
	if (operation == '+' || operation == '-' || operation == '*' || operation == '/')
	{
		printf("Here is your equation: %d %c %d \n", a, operation, b);
		comm_channel->rw = 1;
		strcpy(comm_channel->name, name);
		comm_channel->request[0] = a;
		comm_channel->request[1] = b;
		comm_channel->request[2] = (int)operation;

		printf("Sent the data.\n");
		// strcpy(comm_channel->request,"Hey!");
		// printf("%p\n", comm_channel->response);
		// while(strcmp(comm_channel->response,"") == 0) sleep(1);
		while (comm_channel->rw == 1)
			sleep(1);
		if (comm_channel->response[0] != INT_MIN)
			printf("Server response: %d\n", comm_channel->response[0]);
		else
			printf("Error.\n");
		
	}
	else
	{
		printf("Wrong operation character. Terminating\n");
		return;
	}

	return;
}

void EvenOrOdd()
{
	int a;
	printf("Enter the number you want to check: \n");
	if (scanf("%d", &a) != 1)
	{
		printf("ERROR: wrong input\n");
		return;
	}
	getchar();
	comm_channel->rw = 2;
	strcpy(comm_channel->name, name);
	comm_channel->request[0] = a;

	printf("Sent the data.\n");
	while (comm_channel->rw == 2)
		sleep(1);
	if (comm_channel->response[0] == 1)
	{
		printf("The number: %d is Odd\n", a);
	}
	else
	{
		printf("The number: %d is Even\n", a);
	}
	return;
}

void IsPrime()
{
	int a = INT_MIN;
	printf("Enter the number you want to check: \n");
	if (scanf("%d", &a) != 1)
	{
		printf("ERROR: wrong input\n");
		return;
	}
	getchar();
	// fflush(stdin);
	// printf("%d\n", (int) isdigit(a));
	// printf("%d\n",a);
	if (a == INT_MIN)
	{
		printf("This is not an integer. Returning...\n");
		return;
	}
	comm_channel->rw = 3;
	strcpy(comm_channel->name, name);
	comm_channel->request[0] = a;

	printf("Sent the data.\n");
	while (comm_channel->rw == 3)
		sleep(1);
	if (comm_channel->response[0] == 1)
	{
		printf("The number: %d is PRIME\n", a);
	}
	else
	{
		printf("The number: %d is NOT PRIME\n", a);
	}

	return;
}

void send()
{
	comm_channel->rw = -404;
	strcpy(comm_channel->name, name);
	printf("Sent the data.\n");
	while (comm_channel->rw == -404) sleep(1);
	printf("Server response: %d, wrong option selected\n", comm_channel->response[0]);
}

void clearall()
{
	printf("Clearing all association...\n");
	shmdt(connect_channel);
	shmdt(comm_channel);
}

int main()
{
	// int i;
	// void *connect_channel;
	// char buff[100];
	// int shmid;

	printf("Process ID: %d\n", (int)getpid());
	connect();
	connect_comm_new();
	int t;
	int flag = 0;

	while (flag != -1)
	{
		fflush(stdin);
		t = menu();
		if (t == -1)
		{
			printf("Client exiting loop.\n");
			strcpy(comm_channel->name, "exit");
			strcpy(connect_channel->request, name);
			clearall();
			break;
		}
		/*if(t >57 && t<48){
			printf("Wrong character.\n");
			continue;*/
		if(shmget((key_t)1110, sizeof(struct channel), 0666)<0){
			printf("Server closed, exiting...\n");
			break;
		}
		switch (t)
		{
		case 1:
			Arithmetic();
			break;
		case 2:
			EvenOrOdd();
			break;
		case 3:
			IsPrime();
			break;
		default:
			send();
			// printf("Wrong option. Select again.\n");
		}
		sleep(1);
	}
	sleep(1);
	printf("Client exited\n");

	return 0;
}
