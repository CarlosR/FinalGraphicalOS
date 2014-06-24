
#define EOF -1
#define FILE_NAME_SIZE 12
#define MAX_BLOCKS 32
#define LIBRE 1
#define OCUPAT 0
#define MAX_FILES 12
#define NO_TROBAT -1
#define O_RDONLY 1
#define O_WRONLY 2
#define O_RDWR 3
#define O_CREAT 4
#define BLOCK_SIZE 256


//------------------------------------------------------------------------------------------
struct dir_ent
{
char nom[FILE_NAME_SIZE];
short int size; 
};

struct bloc
{
int libre;
int seguent;
};

struct file
{
char nom[FILE_NAME_SIZE];
int primerBloc;
int numRef;
short int size;
};

extern struct file directory[MAX_FILES];
extern struct bloc fs[MAX_BLOCKS];
extern char espaiDisk[MAX_BLOCKS * BLOCK_SIZE];

int initFS(void);
int existeixFitxer(const char *path);
int buscarFitxer();
int buscarBloc();
//-------------------------------------------------------------------------------------------

/*lib.c
Michael Black, 2007
This file contains the various library functions called by the shell and 
user applications
*/

/*setup enables multitasking.  it should be called at the beginning of 
any user program*/
void setup()
{
	seti();
}

/*computes a mod b*/
int mod(int a, int b)
{
        while(a>=b)
                a=a-b;
        return a;
}

/*computes a / b using integer division*/
int div(int a, int b)  
{               
        int q=0;
        while (q*b<=a)
                q++;
        return q-1;
}               

/*reads a sector into the buffer*/
void readsector(int sectornumber, char* buffer)
{
        int sec,head,cyl;

	/*convert to CHS*/
        sec=mod(sectornumber,0x12)+1;
        head=mod(div(sectornumber,0x12),2);
        cyl=div(sectornumber,0x24);

        readsect(buffer,sec,head,cyl);
}

/*writes buffer to a sector*/
void writesector(int sectornumber, char* buffer)
{
        int sec,head,cyl;

	/*convert to CHS*/
        sec=mod(sectornumber,0x12)+1;
        head=mod(div(sectornumber,0x12),2);
        cyl=div(sectornumber,0x24);

        writesect(buffer,sec,head,cyl);
}

/*prints a character*/
void putchar(char c)
{
	printc(c);
}

/*reads a character*/
char getchar()
{
	return (char)readc();
}

/*prints a string terminated with 0*/
void printstring(char* string)
{
	int21(1,string);
}

/*prints an integer*/
void printnumber(int number)
{
	char num[12];
	char pnum[12];
	int d=1;
	int i=0;
	int j;

	/*convert the number to a string digit by digit*/
	while(i<10)
	{
		num[i]=mod(div(number,d),10)+0x30;
		i++;
		d=d*10;
		if (div(number,d)==0)
			break;
	}

	/*reverse it to read most significant to least significant*/
	j=0;
	for (d=i-1; d>=0; d--)
		pnum[j++]=num[d];
	pnum[j]=0;
	printstring(pnum);
}

/*read a line from the keyboard and put it in buffer*/
void readstring(char* buffer)
{
	int21(2,buffer);
}

/*read the file name[] into buffer[]*/
void readfile(char* name, char* buffer)
{
	int21(3,name,buffer);
}

/*write buffer[] to a file called name[]*/
void writefile(char* name, char* buffer, int sectorlength)
{
	int21(4,name,buffer,sectorlength);
}

/*delete the file name[]*/
void deletefile(char* name)
{
	int21(5,name);
}

/*execute the program*/
void executeprogram(char* buffer, int bytelength)
{
	int21(8,buffer,bytelength);
}

/*execute the program, but don't make the caller wait*/
void executeprogrambackground(char* buffer, int bytelength)
{
	int21(6,buffer,bytelength);
}

/*terminate program - this must be called by the program before ending*/
void exit()
{
	int21(7);
}

/*sets the video to 1 - graphics, 0 - text*/
void setvideo(int mode)
{
	if (mode==0)
		setvideotext();
	else
		setvideographics();
}

/*sets the pixel at columnxrow to color*/
void setpixel(int color, int column, int row)
{
	drawpixel(color,row*320+column);
}

/*sets the cursor position to row, column*/
void setcursor(int row, int column)
{
	interrupt(0x10,2*256,0,0,row*256+column);
}

/*clear the screen*/
void clearscreen()
{
        int r,c;
        for (c=0; c<80; c++)
                for (r=0; r<25; r++)
                {
                        putInMemory(0xB800,(r*80+c)*2,0x20);
                        putInMemory(0xB800,(r*80+c)*2+1,7);
                }
}

/*prints a character at a certain cursor position*/
void setchar(char c, char color, int row, int column)
{
	putInMemory(0xB800,(row*80+column)*2,c);
	putInMemory(0xB800,(row*80+column)*2+1,color);
}

/*prints a string at a certain cursor position*/
void setstring(char* c, char color, int row, int column)
{
	int i=0;
	while(c[i]!=0x0)
	{
		putInMemory(0xB800,(row*80+column+i)*2,c[i]);
		putInMemory(0xB800,(row*80+column+i)*2+1,color);
		i++;
	}
}

/*turns an integer into a string*/
void getnumberstring(char* pnum, int number)
{
	char num[12];
	int d=1;
	int i=0;
	int j;

	/*convert the number to a string digit by digit*/
	while(i<10)
	{
		num[i]=mod(div(number,d),10)+0x30;
		i++;
		d=d*10;
		if (div(number,d)==0)
			break;
	}

	/*reverse it to read most significant to least significant*/
	j=0;
	for (d=i-1; d>=0; d--)
		pnum[j++]=num[d];
	pnum[j]=0;
}

//-----------------------------------------------------------------------------------------

/*
int readdir(struct dir_ent *buffer, int offset)
{
	int resultat;
	__asm__ __volatile__(
	"pushl %%ebx\n"	
	"pushl %%ecx\n"
	"movl %2, %%ecx\n"	
	"movl %1, %%ebx\n"	
	"movl $141, %%eax\n"	
	"int $0x80\n"	
	"movl %%eax, %0\n"	
	"popl %%ecx\n"
	"popl %%ebx\n"
	      : "=g" (resultat)
	      : "g" (buffer), "g" (offset)
	: "ax", "cx", "bx", "memory"
	);

	if (resultat>=0)
		return resultat;
	else {
		errno=resultat;

		return -1;	
	}
}
*/

//----------------------------------------------------------------------------------------

struct file directory[MAX_FILES];
//__attribute__((__section__(".data.directory")));

struct bloc fs[MAX_BLOCKS];
char espaiDisk[MAX_BLOCKS * BLOCK_SIZE];

int initFS()
{
	int i;

	for(i=0;i<MAX_BLOCKS;i++)
	{
		fs[i].libre = LIBRE;
		fs[i].seguent = EOF;
	}

	for(i=0;i<MAX_FILES;i++)
	{
		directory[i].nom[0] = '\0';
		directory[i].primerBloc = EOF;
		directory[i].numRef = 0;
		directory[i].size = 0;
	}
	return 0;
	}


void readFile(char* fileName,char* buffer){
    int fileFound;
    int nameCt = 0;
	int index, k,h;
	int sectors[27];
	int j = 0;
	int i;
	int buffAddress = 0;


    readSector(buffer, 2);

	fileFound = strComp(buffer,fileName);

	if (fileFound!=0){

		index = fileFound*32+6;
		for (j=0;j<26;j++){
			sectors[j] = buffer[index+j];

		}

		sectors[26] = 0;
		k = 0;
		while(sectors[k]!=0x0){
			readSector(buffer+buffAddress,sectors[k]);
			buffAddress += 256;
			k++;
		}

	}
	else
	{
		printString("File no encontrado!");
		return;
	}	

}


void writeFile(char* name,char* buffer, int numberOfSectors) {
	char map[256];
	char directory[256];
	int directoryLine,j,k, index, diff;
	int nameLen = 0;
	int sectorNum;
	char subBuff[256];
	int iterator = 0;
	int foundFree = 0;
	int nameInts[7];
	int i,h;
	int kVal;

	readSector(map,1);
	readSector(directory,2);

	for (directoryLine = 0; directoryLine < 16; directoryLine++){
		if (directory[32*directoryLine] == 0x00){
			foundFree = 1;
			break;
		}
	}
	
	if (foundFree == 0){
		printString("No encontro espacio libre para el archivo");
		return;
	}

	while(name[nameLen] != '\0' && name[nameLen] != 0x0){
		nameLen++;
	}

	for (j=0;j<nameLen;j++){
		directory[32*directoryLine+j] = name[j];
	}

	if (nameLen < 6){
	diff = 6-nameLen;
		for(j=0;j<diff;j++){
			index = j+nameLen;
			directory[32*directoryLine+index] = 0x0;
		}
	}

	for (k = 0; k < numberOfSectors; k++){

		sectorNum = 0;
		while(map[sectorNum] != 0x0){
			sectorNum++;
		}
		
		if (sectorNum==26)
		{
			printString("No hay suficiente espacio en el directorio\n");
			return;
		}

		map[sectorNum] = 0xFF;

		directory[32*directoryLine+6+k] = sectorNum;
	
		for(j=0;j<512;j++){
			kVal = k+1;
			subBuff[j] = buffer[j*kVal];
		}
		
		writeSector(subBuff,sectorNum);
	}

	writeSector(map,1);
	writeSector(directory,2);
}


//---------------------------------------------------------------------

int existeixFitxer(char *path)
{
	int i;
	char trobat = 0;

	for(i=0;i<MAX_FILES && !trobat;i++)
	{
		if(equalStrings(directory[i].nom, (char *)path)){
			return i;
		} 
	}
	return NO_TROBAT;
}

int buscarBloc()
{
	int i;
	for(i=0;i<MAX_BLOCKS;i++)
	{
		if(fs[i].libre== LIBRE)
		{
			fs[i].libre = LIBRE;
			return i;
		}
	}
	return EOF;
}

int buscarFitxer()
{
	int i;
	for(i=0; i<MAX_FILES; i++)
	{
		if(directory[i].nom[0] == '\0'){
			return i;
		} 
	}
	//return -ENOSPC;
	return -1;
}
