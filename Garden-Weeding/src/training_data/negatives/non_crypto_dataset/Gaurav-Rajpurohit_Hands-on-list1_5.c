
/*
============================================================================
Name : 5.c
Author : Gaurav Rajpurohit
Description : Write a program to create five new files with infinite loop. Execute the program in the background
and check the file descriptor table at /proc/pid/fd.
Date: 15th Aug, 2025.
============================================================================
*/

#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>


int main(){
	int fd1 = open("a.txt", O_CREAT | O_WRONLY, 0644);
	printf("fd1 = %d\n",fd1);
	int fd2 = open("b.txt", O_CREAT | O_WRONLY, 0644);
	printf("fd2 = %d\n",fd2);
	int fd3 = open("c.txt", O_CREAT | O_WRONLY, 0644);
	printf("fd3 = %d\n",fd3);
	int fd4 = open("d.txt", O_CREAT | O_WRONLY, 0644);
	printf("fd4 = %d\n",fd4);
	int fd5 = open("e.txt", O_CREAT | O_WRONLY, 0644);
	printf("fd5 = %d\n",fd5);

	while(1){
		sleep(1);
	};
	return 0;
}

/*
gaurav176@gaurav176-HP-Pavilion-Laptop-14-dv0xxx:~/Desktop/hands-on-list/que5$ ls -l
total 20
-rw-rw-r-- 1 gaurav176 gaurav176   934 Sep  2 18:16 5.c
-rwxrwxr-x 1 gaurav176 gaurav176 16040 Sep  2 18:20 a.out
-rw-r--r-- 1 gaurav176 gaurav176     0 Sep  2 18:20 a.txt
-rw-r--r-- 1 gaurav176 gaurav176     0 Sep  2 18:20 b.txt
-rw-r--r-- 1 gaurav176 gaurav176     0 Sep  2 18:20 c.txt
-rw-r--r-- 1 gaurav176 gaurav176     0 Sep  2 18:20 d.txt
-rw-r--r-- 1 gaurav176 gaurav176     0 Sep  2 18:20 e.txt

*/