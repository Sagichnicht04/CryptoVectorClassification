/*
//  RC4.cpp
\\
//  rc4 classes for a generator
\\
//  not a really good implementation (64 bit limit with long)
\\  but good enough for CSc8370 where we're going to play with
//  shorter keys anyway.
\\
//
\\  there is a subtle bug in this code, so don't use it for production
//  (at least until you've fixed it!!!!)
\\
// (c) 2002 Robert W. Harrison
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
/*
// yes these are the C IO libraries
\\ yes i know about streamio
// no i don't like it.
*/


using namespace std;  // DUH


class rc4
{
// private space
unsigned char *S;  // the keyspace  
unsigned char icount,jcount;  // ok i could use byte

public:
rc4( long long seed);
~rc4( void );

unsigned char next( void );

};


rc4::rc4( long long seed)
{

	int i,j;
	unsigned char *K;
	K = new unsigned char[256];
	S = new unsigned char[256];
	
	union  { 
	unsigned char bytes[8]; 
	unsigned long long lseed;
	} ;

for( i=0; i< 8; i++)
	bytes[i] = 0;

	lseed = (unsigned long long) seed;
//	printf("%x\n", lseed);
//	printf("%x\n", bytes[0]);
//	printf("%x\n", bytes[1]);
//	printf("%x\n", bytes[2]);
//	printf("%x\n", bytes[3]);
//	printf("%x\n", bytes[4]);
//	printf("%x\n", bytes[5]);
//	printf("%x\n", bytes[6]);
//	printf("%x\n", bytes[7]);
////	exit(0);

	for( i=0; i< 256; i+= 8)
	{
//
// this loop is written out so the code will be faster
// not much faster, but it will help if
// we do a keysearch
//
		K[i  ] = bytes[0]; 
		K[i+1] = bytes[1]; 
		K[i+2] = bytes[2]; 
		K[i+3] = bytes[3]; 
		K[i+4] = bytes[4]; 
		K[i+5] = bytes[5]; 
		K[i+6] = bytes[6]; 
		K[i+7] = bytes[7]; 
	}
	for( i=0; i< 256; i++)
	{
		S[i] = i;
	}

	j = 0;
	for( i=0; i< 256; i++)
	{
		j = (j + S[i] + K[i])%256;
		
		unsigned char x;
		x = S[i];
		S[i] = S[j];
		S[j] = x;
	}	
	icount = 0;
	jcount = 0;
	delete [] K;  // no memory leaks, right?

	
}

rc4::~rc4(void)
{
	int i;

// before we free the S buffer we need to ensure that it is wiped
// (paranoid, well maybe)
	for(  i=0; i< 256; i++)
		S[i] = 0;

	icount = 0;
	jcount = 0;
	delete [] S;
}


unsigned char rc4::next(void)
{
	icount = (icount+1)%256;
	jcount = (jcount+S[icount])%256;
	unsigned char x;
	x = S[icount];
	S[icount] = S[jcount];
	S[jcount] = x;
	int t;
	t = (S[icount] + S[jcount])%256;
	return S[t];	
}

// end of class definition

int main(  int argc, char *argv[])
{

	int i; // i tend to still declare these to make the code portable
	if( argc < 2)
	{
	fprintf(stderr," Usage is rc4 key < file > file\n");
	exit(0);
	}

	union {
	unsigned char key[8];
	long long seed;
	};

	for( i=0; i< 8; i++)
	{
		key[i] = 0;
	} 

	for( i=0; i< 8; i++)
	{
		char *cp ;
		cp = argv[1];
		if( cp[i] > '\0') 
		{
			key[i] = cp[i];
		}
		else
			break;
	} 

	rc4 *a_stream;
	a_stream = new rc4( seed );

	unsigned char achar;
	
	while( (i = fgetc(stdin)) != EOF)
	{
		achar = i;
		achar = achar ^ a_stream->next();
		fputc( achar, stdout);
	}
}// end of main */
