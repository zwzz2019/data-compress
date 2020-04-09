#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#define MAX 16	//MAXimum length of codeword, more the MAX more overhead

//huffman defined
char padding;
unsigned char N;

typedef struct symCode
{ char x;
  char code[MAX];
}symCode;

symCode *symlist;
int n;
char *decodeBuffer(char buffer);
char *int2string(int n);
int match(char a[],char b[],int limit);
int fileError(FILE *fp);
int huffman_decode(char *decode);
//end huffman defined

//lzw defined
void adddic(char p[], char dicstring[][100], uint16_t code[], uint16_t len);
void initdic(char dicstring[][100], uint16_t code[], uint16_t len);
int builddic(uint16_t *ch, int num, char *decompress);
uint16_t out_ch[65536];
int lzw_decompress(char *decode, char *decompress);
//end lzw defined

int main()
{
  //start huffman decode
  char decode[65536]={};
  int num;
  num = huffman_decode(decode);
  //end huffman decode
  //'decode' is a array after huffman decode

  //start lzw decompress
  char decompress[]={};
  lzw_decompress(decode,decompress);
  //end lzw decompress
  //printf("%s\n", decompress);
  //decompress is a array after lzw decompression
  FILE *fw;
  char *s = &decompress[0];
  fw = fopen("origin_data.txt","wb");
  for(;*s!='\0';s++)
  fwrite(s,sizeof(unsigned char),1,fw);
	return 0;
}

//huffman function
int huffman_decode(char *decode)
{
  FILE *fp,*outfile;
	char buffer;
  char *decoded;
	int i;
	fp=fopen("out_data.txt","rb");
	fread(&buffer,sizeof(unsigned char),1,fp);
	N=buffer;		//No. of structures(mapping table records) to read
	if(N==0)n=256;
	else n=N;
	//printf("%u different characters.",n);

	//allocate memory for mapping table
	symlist=(symCode *)malloc(sizeof(symCode)*n);
	fread(symlist,sizeof(symCode),n,fp);

	//Read mapping table
	// printf("\n");
	// for(i=0;i<n;i++)
  // printf("[%c|%s] ",symlist[i].x,symlist[i].code);

	fread(&buffer,sizeof(char),1,fp);
	padding=buffer;		//No. of bits to discard

	while(fread(&buffer,sizeof(char),1,fp)!=0)	//Read 1 byte at a time
	{
		decoded=decodeBuffer(buffer);	//decoded is pointer to array of characters read from buffer byte
		i=0;
		while(decoded[i++]!='\0');	//i-1 characters read into decoded array
		strcat(decode, decoded);
	}
	fclose(fp);
  return i-1;
}

char *decodeBuffer(char b)
{
	int i=0,j=0,t;
	static int k;
	static int buffer;	// buffer larger enough to hold two b's
	char *decoded=(char *)malloc(MAX*sizeof(char));
	t=(int)b;
	//printf("\nt=%sk=%d",int2string(t),k);
	t=t & 0x00FF;		//mask high byte
	//printf("\nt=%sk=%d",int2string(t),k);
	t=t<<8-k;		//shift bits keeping zeroes for old buffer
	//printf("\nt=%sk=%d",int2string(t),k);
	buffer=buffer | t;	//joined b to buffer
	k=k+8;			//first useless bit index +8 , new byte added

	if(padding!=0)	// on first call
	{
		buffer=buffer<<padding;
		k=8-padding;	//k points to first useless bit index
	 	padding=0;}

		//printf("\nbuffer=%s, k=%d",int2string(buffer),k);
		//loop to find matching codewords

	while(i<n)
	{
		if(!match(symlist[i].code, int2string(buffer),k))
		{
			decoded[j++]=symlist[i].x;	//match found inserted decoded
			t=strlen(symlist[i].code);	//matched bits
			buffer=buffer<<t;		//throw out matched bits
			k=k-t;				//k will be less
			i=0;				//match from initial record
			//printf("\nBuffer=%s,removed=%c,k=%d",int2string(buffer),decoded[j-1],k);
			if(k==0) break;
			continue;
		}
	i++;
	}

	decoded[j]='\0';
	return decoded;

}

int match(char a[],char b[],int limit)
{
	b[strlen(a)]='\0';
	b[limit]='\0';
	return strcmp(a,b);
}

char *int2string(int n)
{
	int i,k,and,j;
	char *temp=(char *)malloc(16*sizeof(char));
	j=0;

for(i=15;i>=0;i--)
{
	and=1<<i;
	k=n & and;
	if(k==0) temp[j++]='0'; else temp[j++]='1';
}
	temp[j]='\0';
	return temp;
}

int fileError(FILE *fp)
{
	printf("[!]File read Error.\n[ ]File is not compressed using huffman.\n");
	fclose(fp);
	return -3;
}

//end huffman function

//lzw function
int lzw_decompress(char *decode, char *decompress)
{
  char p[99] = {}, *q=p;
  int num = 0;
  //transfrom string to int array
  for(char *de = decode; *de!='\0'; de++)
  {
    if(*de == ',')
    {
      out_ch[num] = atoi(p);
      memset(p,'\0',sizeof(p));
      q=p;
      num++;
      continue;
    }
    *q = *de;
    q++;
  }
  //printf("\n");
  //for(int i=0; i<num;i++)
  //printf("%d ", out_ch[i]);
  //printf("\n");
  builddic(out_ch,num,decompress);
  return 0;
}


int builddic(uint16_t *ch, int num, char *decompress){
  char dicstring[65536][100];
	uint16_t code[65536];
	uint16_t len, in;
	uint16_t code1;
	len = 256;
	initdic(dicstring, code , len);
	int fd, i, fdw;
	uint16_t old, new;
  old = *ch;
	//printf("%s", dicstring[old]);
  strcat(decompress,dicstring[old]);
	char olds[100], c, s[100], r[2];
  ch++;
	while(num--){
    new = *ch;
    ch++;
		//printf("%d", new);
		if(new >= len)
		{
			strcpy(s, dicstring[old]);
			strcat(s, r);
			//printf("%s", s);
		}
		else
		{
			strcpy(s, dicstring[new]);
		}
		//printf("|%s|", s);
    strcat(decompress,s);
		//write(fdw, s, sizeof(s));
		c = s[0];
		r[0] = c;
		r[1] = '\0';
		strcpy(olds, dicstring[old]);
		strcat(olds, r);
		adddic(olds, dicstring, code, len);
		len++;
		old = new;
	}
	close(fd);
  return 0;
}

void initdic(char dicstring[][100], uint16_t code[], uint16_t len){
	int i;
	for(i = 0; i < len; i++)
	{
		dicstring[i][0] = i;
		dicstring[i][1] = '\0';
		code[i] = i;
		//prefix[i] = i;
	}
}
void adddic(char p[], char dicstring[][100], uint16_t code[], uint16_t len){
	strcpy(dicstring[len], p);
	code[len] = len;
	//printf("\n%s-%d\n", dicstring[len], code[len]);
	//prefix[len] = code1;
}
//end lzw function
