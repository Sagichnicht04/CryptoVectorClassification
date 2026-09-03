/*
Name:- Padaliya Shivam
Roll No:- MT2024107
Program:-7. Write a simple program to print the created thread ids.
*/

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void print()
{
    printf("Printing from thread :%lu\n", pthread_self());
}

void main()
{
    pthread_t threadID;
    int pstatus;

   for(int i=0;i<3;i++){
	threadID=i+1;
	pstatus = pthread_create(&threadID, NULL, (void *)print, NULL);
   	 if (pstatus != 0)
   	 {
        	perror("Error while creating thread");
        	_exit(1);
    	 }
	
   }
    pthread_exit(NULL);
}

/*
shivam@shivam-Vostro-3590:~/Desktop/Handsonlist2/ss-assignments-2$ ./a.out
Printing from thread :140067959994048
Printing from thread :140067951601344
Printing from thread :140067943208640
*/
