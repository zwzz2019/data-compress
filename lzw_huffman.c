#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <malloc.h>
#define INTERNAL 1
#define LEAF 0
#define MAX 16	//MAXimum length of codeword, more the MAX more overhead

//LZW define
void adddic(char p[], char dicstring[][100], uint16_t code[], uint16_t prefix[], uint16_t code1, uint16_t len);
int search(char p[], char dicstring[][100],uint16_t len);
void initdic(char dicstring[][100], uint16_t code[], uint16_t prefix[], uint16_t len);
int lzw_compress(char *ch_lzw, uint16_t *out);
//end LZW defined

//Huffman defined
int huffman_encode(int num, char *out_code, uint16_t *out);
char padding;
unsigned char N;

typedef struct symCode
{ char x;
  char code[MAX];
}symCode;

typedef struct node
{
	char x;
	int freq;
	char *code;
	int type;
	struct node *next;
	struct node *left;
	struct node *right;
}node;

node *HEAD,*ROOT;

void printll();
void makeTree();
void genCode(node *p,char* code);
void insert(node *p,node *m);
void addSymbol(char c);
void writeHeader(FILE *f);
void writeBit(int b,FILE *f);
void writeCode(char ch_huffman,FILE *f, char *out_code);
char *getCode(char ch_huffman);

node* newNode(char c)
{
	node *q;
	q=(node *)malloc(sizeof(node));
	q->x=c;
	q->type=LEAF;	//leafnode
	q->freq=1;
	q->next=NULL;
	q->left=NULL;
	q->right=NULL;
	return q;
}


void printll()
{
	node *p;
	p=HEAD;

	while(p!=NULL)
	{
		printf("[%c|%d]=>",p->x,p->freq);
		p=p->next;
	}
}

//end Huffman defined


int main()
{
    //test array
    char in_ch[84]={'T','h','e',' ','1','1','1','2','3','3','h','3','T','h','e',' ','1','1','1','2','3','3','h','3','4','5','e','3','9','9','6','3','5','6','6','3','3','7','8','7','8','T','h','e',' ','1','1','1','2','3','3','h','3','T','h','e',' ','1','1','1','2','3','3','h','3','4','5','e','3','9','9','6','3','5','6','6','3','3','7','8','7','8'};

    //start lzw compressed
    printf("start lzw compressed\n");
    int num;
    uint16_t out[65536];//output data after lzw compress
    num = lzw_compress(&in_ch[0], out);
    //end lzw compress

    printf("\nnumber of output data: %d\noutput data: ",num);
    for(int i = 0;i<num;i++)
    printf("%d ",out[i]);
    printf("\n");
    //'out[]' is a array after lzw compression, 'num' is the size of out[]

    //start huffman encode
    printf("start huffman coding\n");
    char out_code[65536];//output data after huffman coding
    huffman_encode(num, out_code, out);
    //end huffman encode
    printf("output code is: %s\n", out_code);
    //'out_code[]' is a array after huffman coding
    //save output data to 'out_data.txt'
    return 0;
}


//lzw function
//lzw compress
int lzw_compress(char *ch_lzw, uint16_t *out)
{
  int num;
	char dicstring[65536][100], p[100];
	uint16_t code[65536];
  int number1 = 0;
	uint16_t prefix[65536];
	uint16_t len;
	uint16_t code1;
	len = 256;
	initdic(dicstring, code , prefix, len);
	int fd, i, r, fdw;
	char pre[100];
  printf("data to be compressed: ");
	while(*ch_lzw !='\0')
  {
		printf("%c", *ch_lzw);
		strcpy(pre, p);
		p[i] = *ch_lzw;
    ch_lzw++;
		p[++i] = '\0';
		r = search(p, dicstring, len);
		if(r)
		{
			code1 = prefix[r-1];
		}
		else
		{
			adddic(p, dicstring, code, prefix, code1, len);
			len++;
			int j = search(pre, dicstring, len);
      out[number1] = code[j-1];
      number1++;
			int l = strlen(p);
			char c = p[l-1];
			strcpy(p, "\0");
			p[0] = c;
			p[1] = '\0';
			i = 1;
		}
    if((*ch_lzw =='\0')&&(search(p, dicstring, len)))
    {
      int r = search(p, dicstring, len);
      out[number1] = code[r-1];
      number1++;
    }
	}
  return number1;
}
//end lzw compress

void initdic(char dicstring[][100], uint16_t code[], uint16_t prefix[], uint16_t len){
	int i;
	for(i = 0; i < len; i++)
	{
		dicstring[i][0] = i;
		dicstring[i][1] = '\0';
		code[i] = i;
		prefix[i] = i;
	}
}

int search(char p[], char dicstring[][100],uint16_t len){
	int i;
	for(i = 0; i < len; i++)
	{
		if(!strcmp(p, dicstring[i]))
		{
      //printf("\nsearch- %d -%s--%s\n", i, dicstring[i], p);
			return i + 1;
		}
	}
	return 0;
}

void adddic(char p[], char dicstring[][100], uint16_t code[], uint16_t prefix[], uint16_t code1, uint16_t len){
	strcpy(dicstring[len], p);
	code[len] = len;
	prefix[len] = code1;
}
//end lzw function

//huffman function

//huffman encode
int huffman_encode(int num, char *out_code, uint16_t *out)
{
  //transform lzw output data to string
  int i,L=0;
  char str[65536];
  for (i=0;i<num;i++)
  {
    sprintf(str+L,"%d",out[i]);
    strcat(str,",");
    L = strlen(str);
  };
  //printf("\n%s\n",str);

  //huffman compress
  FILE *fp2;
  int j=0,k=0;
  char ch_huffman,*ch2;
  int t;
  HEAD=NULL;
  ROOT=NULL;
  ch2 = str;
  while(*ch2!='\0')
  {
    addSymbol(*ch2);
    ch2++;
  }
  makeTree();
  genCode(ROOT,"\0");	//preorder traversal
  fp2=fopen("out_data.txt","wt");
  if(fp2==NULL)
  {
    printf("\n[!]Output file cannot be opened.\n");
    return -2;
  }
  writeHeader(fp2);
  ch2 = str;
  while(*ch2!='\0')
  {
    printf("%c replaced with ",*ch2);
    ch_huffman = *ch2;
    writeCode(ch_huffman,fp2, out_code);	//write corresponding code into fp2
    ch2++;
  }
  fclose(fp2);
  //printf("%s\n", out_code);
  return 0;
}
void writeHeader(FILE *f)
{
	//Table mapping 'codewords' to actual symbols
	symCode record;
	node *p;
	int temp=0,i=0;
	p=HEAD;
	while(p!=NULL)	//determine number of unique symbols and padding of bits
	{
		temp+=(strlen(p->code)) * (p->freq);		//temp stores padding
		if(strlen(p->code) > MAX) printf("\n[!] Codewords are longer than usual.");	//TODO: Solve this case
		temp%=8;
		i++;
		p=p->next;
	}

	if(i==256)
		N=0;	//if 256 diff bit combinations exist, then alias 256 as 0
	else
		N=i;

	fwrite(&N,sizeof(unsigned char),1,f);	//read these many structures while reading
	//printf("\nN=%u",i);

	p=HEAD;
	while(p!=NULL)	//start from HEAD, write each char & its code
	{
		record.x=p->x;
		strcpy(record.code,p->code);
		fwrite(&record,sizeof(symCode),1,f);
	//	printf("\n%c|%s",record.x,record.code);
		p=p->next;
	}
	//discard 'padding' bits before data, while reading
	padding=8-(char)temp;	//int to char & padding = 8-bitsExtra
	fwrite(&padding,sizeof(char),1,f);
	//printf("\nPadding=%d",padding);
	//do actual padding
	for(i=0;i<padding;i++)
	writeBit(0,f);

}//fun

void writeCode(char ch,FILE *f, char *out_code)
{
	char *code;
	code=getCode(ch);
	printf("%s\n",code);
  strcat(out_code,code);
	while(*code!='\0')
	{
		if(*code=='1')
			writeBit(1,f); //write bit 1 into file f
		else
			writeBit(0,f);
	code++;
	}
	return;
}

void writeBit(int b,FILE *f)
{//My Logic: Maintain static buffer, if it is full, write into file
	static char byte;
	static int cnt;
	char temp;
	//printf("\nSetting %dth bit = %d of %d ",cnt,b,byte);
	if(b==1)
	{	temp=1;
		temp=temp<<(7-cnt);		//right shift bits
		byte=byte | temp;
	}
	cnt++;

	if(cnt==8)	//buffer full
	{
		//	printf("[%s]",bitsInChar(byte));
		fwrite(&byte,sizeof(char),1,f);
		cnt=0; byte=0;	//reset buffer
		return;// buffer written to file
	}
	return;
}

char *getCode(char ch)
{
	node *p=HEAD;
	while(p!=NULL)
	{
	    if(p->x==ch)
		  return p->code;
	    p=p->next;
	}
	return NULL; //not found
}

void insert(node *p,node *m)
{ // insert p in list as per its freq., start from m to right,
	// we cant place node smaller than m since we dont have ptr to node left to m
	if(m->next==NULL)
	{  m->next=p; return;}
		while(m->next->freq < p->freq)
		{  m=m->next;
		  if(m->next==NULL)
		    { m->next=p; return; }
		}
	  p->next=m->next;
	  m->next=p;
}

void addSymbol(char c)
{// Insert symbols into linked list if its new, otherwise freq++
	node *p,*q,*m;
	int t;

	if(HEAD==NULL)
	{	HEAD=newNode(c);
		return;
	}
		p=HEAD; q=NULL;
	if(p->x==c) //item found in HEAD
	{
		p->freq+=1;
		if(p->next==NULL)
			return;
		if(p->freq > p->next->freq)
		{
			HEAD=p->next;
			p->next=NULL;
			insert(p,HEAD);
		}
		return;
	}

	while(p->next!=NULL && p->x!=c)
	{
		q=p; p=p->next;
	}

	if(p->x==c)
	{
		p->freq+=1;
	        if(p->next==NULL)
			return;
		if(p->freq > p->next->freq)
		{
			m=p->next;
			q->next=p->next;
			p->next=NULL;
			insert(p,HEAD);
		}
	}
	else  //p->next==NULL , all list traversed c is not found, insert it at beginning
	{
		q=newNode(c);
		q->next=HEAD;  //first because freq is minimum
		HEAD=q;
	}
}

void makeTree()
{
	node  *p,*q;
	p=HEAD;
		while(p!=NULL)
		{
			q=newNode('@');
			q->type=INTERNAL;	//internal node
			q->left=p;		//join left subtree/node
			q->freq=p->freq;
			if(p->next!=NULL)
			{
				p=p->next;
				q->right=p;	//join right subtree /node
				q->freq+=p->freq;
			}
			p=p->next;	//consider next node frm list
			if(p==NULL)	//list ends
				break;
			//insert new subtree rooted at q into list starting from p
			//if q smaller than p
			if(q->freq <= p->freq)
			{//place it before p
				q->next=p;
				p=q;
			}
			else
				insert(q,p);	//find appropriate position
		}//while
		ROOT=q; //q created at last iteration is ROOT of h-tree
}

void genCode(node *p,char* code)
{
		char *lcode,*rcode;
		static node *s;
		static int flag;
		if(p!=NULL)
		{
		//sort linked list as it was
			if(p->type==LEAF)   //leaf node
			{
				if(flag==0) //first leaf node
				{flag=1; HEAD=p;}
				else	//other leaf nodes
				{ s->next=p;}		//sorting LL
				p->next=NULL;
				s=p;
			}

			//assign code
			p->code=code;	//assign code to current node
			//	printf("[%c|%d|%s|%d]",p->x,p->freq,p->code,p->type);
			lcode=(char *)malloc(strlen(code)+2);
			rcode=(char *)malloc(strlen(code)+2);
			sprintf(lcode,"%s0",code);
			sprintf(rcode,"%s1",code);
			//recursive DFS
			genCode(p->left,lcode);		//left child has 0 appended to current node's code
			genCode(p->right,rcode);
		}
}
//end huffman function
