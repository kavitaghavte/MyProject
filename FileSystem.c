#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<malloc.h>
#include<unistd.h>

#define MAXINODE 50
#define READ 1
#define WRITE 2
#define MAXFILESIZE 1024
#define REGULAR 1
#define SPECIAL 0
#define START 0
#define CURRENT 1
#define END 2

typedef struct superblock
{
	int TotalInode;

	int FreeInode;
}SUPERBLOCK,*PSUPERBLOCK;

typedef struct inode
{
	char FileName[50];
	int InodeNumber;
	int FileSize;
	int FileActualSize;
	int FileType;
	char *Buffer;
	int LinkCount;
	int RefernceCount;
	int Permission;                 //This permission is for the file which is created
	struct inode * Next;
}INODE,*PINODE,**PPINODE;

typedef struct filetable
{
	int Readoffset;
	int Writeoffset;
	int Count;
	int Mode;                           //this mode is for the file which is open
	PINODE ptrinode;
}FILETABLE,*PFILETABLE;

typedef struct ufdt
{
	PFILETABLE ptrfiletable;
}UFDT;

UFDT UFDTArr[MAXINODE];
SUPERBLOCK SUPERBLOCKobj;
PINODE head=NULL;

void Man(char *Name)
{
	if(Name==NULL)
	{
		return;
	}
	if(strcmp(Name,"create")==0)
	{
		printf("Description : Used to create new regular file\n");
		printf("usage : create File_Name permission\n");
	}
	else if(strcmp(Name,"clear")==0)
	{
		printf("Description : Used to clear the console\n");
		printf("usage : clear\n");
	}
	else if(strcmp(Name,"exit")==0)
	{
		printf("Description : Used to exit from the project\n");
		printf("usage : exit\n");
	}
	else if(strcmp(Name,"help")==0)
	{
		printf("Description : Used to disply the project information\n");
		printf("usage : help\n");
	}
	else if(strcmp(Name,"read")==0)
	{
		printf("Description : Used to read data from regular file\n");
		printf("usage : read File_Name Number_Of_Byte_To_Read\n");
	}
	else if(strcmp(Name,"write")==0)
	{
		printf("Description : Used to Write into regular file\n");
		printf("usage : Write File_Name\n After this enter the data that we want to write\n");
	}
	else if(strcmp(Name,"ls")==0)
	{
		printf("Description : Used to list all imformation of file\n");
		printf("usage : ls\n");
	}
	else if(strcmp(Name,"stat")==0)
	{
		printf("Description : Used to display information of file by taking file name\n");
		printf("usage : stat File_Name\n");
	}
	else if(strcmp(Name,"fstat")==0)
	{
		printf("Description : Used to display information of file by taking file discripter\n");
		printf("usage : fstat File_Discriptor\n");
	}
	else if(strcmp(Name,"truncate")==0)
	{
		printf("Description : Used to remove data form file\n");
		printf("usage : truncate File_Name\n");
	}
	else if(strcmp(Name,"open")==0)
	{
		printf("Description : Used to open existing file\n");
		printf("usage : open File_Name mode\n");
	}
	else if(strcmp(Name,"close")==0)
	{
		printf("Description : Used to close opened file\n");
		printf("usage : close File_Name\n");
	}
	else if(strcmp(Name,"closeall")==0)
	{
		printf("Description : Used to close all opened file\n");
		printf("usage : closeall\n");
	}
	else if(strcmp(Name,"lseek")==0)
	{
		printf("Description : Used to change the offset\n");
		printf("usage : lseek File_Name ChangeInOffset StartPoint\n");
	}
	else if(strcmp(Name,"rm")==0)
	{
		printf("Description : Used to delete file\n");
		printf("usage : rm File_Name\n");
	}
	else
	{
		printf("ERROR : No manunal entry avaliable\n");
	}
}
void DisplayHelp()
{
	printf("ls : To list out all file\n");
	printf("clear : To clear consol\n");
	printf("open : To open the file\n");
	printf("close : To close the file\n");
	printf("closeall : To close all opened files\n");
	printf("read : To read the content from files\n");
	printf("write : To Write the content into file\n");
	printf("exist : To terminate file system\n");
	printf("stat : To Display information of file using name\n");
	printf("fstat : To Display information of all file description");
	printf("truncate : To Remove all data from file\n");
	printf("rm : To Deletethe file\n");
}
int GetFDFromName(char *name)
{
	int i=0;
	while(i<50)
	{
		if(UFDTArr[i].ptrfiletable!=NULL)
		{
			if(strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName),name)==0)
			{
				break;
			}
		}
		i++;
	}
	if(i==50)
	{
		return -1;
	}
	else
	{
		return i;
	}
}
PINODE Get_Inode(char *name)
{
	PINODE temp=head;
	int i=0;
	if(name==NULL)
	{
		return NULL;
	}
	while(temp!=NULL)
	{
		if(strcmp(name,temp->FileName)==0)
		{
			break;
		}
		temp=temp->Next;
	}
	return temp;
}
void CreateDILB()
{
	int i=1;
	PINODE newn=NULL;
	PINODE temp=head;

	while(i<=MAXINODE)
	{
		newn=(PINODE)malloc(sizeof(INODE));
		newn->LinkCount=newn->RefernceCount=0;
		newn->FileType=newn->FileSize=0;
		newn->Buffer=NULL;
		newn->Next=NULL;
		newn->InodeNumber=i;
		if(temp==NULL)                                   //if their is no inode in the list
		{
			head=newn;
			temp=head;
		}
		else
		{
			temp->Next=newn;
			temp=temp->Next;
		}
		i++;
	}
	//printf("DILB created succesfully\n");
}
void deallocateDILB()
{
	int i=0;
        PINODE temp1,temp2;
        if(head!=NULL)
        {
		temp1=head;
                while(temp1!=NULL)
                {
			if(temp1->Buffer!=NULL)
			{
				free(temp1->Buffer);
				temp1->Buffer=NULL;
			}
                        temp2=temp1->Next;
                        free(temp1);
                        temp1=temp2;
                }
                head=NULL;
        }
	while(UFDTArr[i].ptrfiletable!=NULL)
	{
		UFDTArr[i].ptrfiletable->ptrinode=NULL;
		free(UFDTArr[i].ptrfiletable);
		UFDTArr[i].ptrfiletable=NULL;
		i++;
	}

}
void InitialiseSuperBlock()
{
	int i=0;
	while(i<MAXINODE)
	{
		UFDTArr[i].ptrfiletable=NULL;
		i++;
	}
	SUPERBLOCKobj.TotalInode=MAXINODE;
	SUPERBLOCKobj.FreeInode=MAXINODE;
}
int CreateFile(char *name,int permission)
{
	int i=0;
	PINODE temp=head;
	if((name==NULL)||(permission==0)||(permission>3))
	{
		return -1;
	}
	if(SUPERBLOCKobj.FreeInode==0)           //If there is no freeinode
	{
		return -2;
	}
	if(Get_Inode(name)!=NULL)              //if file is already exits
	{
		return -3;
	}

	(SUPERBLOCKobj.FreeInode)--;

	while(temp!=NULL)                     //we find any empty inode in IIT
	{
		if(temp->FileType==0)         //if FileType is 0 then file is not created by the inode otherwisw if FileType is 1 then file is created
		{
			break;
		}
		temp=temp->Next;
	}
	while(i<MAXINODE)                      //we find empty entry in ufdt
	{
		if(UFDTArr[i].ptrfiletable==NULL)
		{
			break;
		}
		i++;
	}
	UFDTArr[i].ptrfiletable=(PFILETABLE)malloc(sizeof(FILETABLE));
	if(UFDTArr[i].ptrfiletable==NULL)
	{
		(SUPERBLOCKobj.FreeInode)++;
		return -4;
	}

	UFDTArr[i].ptrfiletable->Count=1;
	UFDTArr[i].ptrfiletable->Mode=permission;
	UFDTArr[i].ptrfiletable->Readoffset=0;
	UFDTArr[i].ptrfiletable->Writeoffset=0;
	UFDTArr[i].ptrfiletable->ptrinode=temp;

	strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName,name);
	UFDTArr[i].ptrfiletable->ptrinode->FileType=REGULAR;
	UFDTArr[i].ptrfiletable->ptrinode->RefernceCount=1;
	UFDTArr[i].ptrfiletable->ptrinode->LinkCount=1;
	UFDTArr[i].ptrfiletable->ptrinode->FileSize=MAXFILESIZE;
	UFDTArr[i].ptrfiletable->ptrinode->FileActualSize=0;
	UFDTArr[i].ptrfiletable->ptrinode->Permission=permission;
	UFDTArr[i].ptrfiletable->ptrinode->Buffer=(char*)malloc(MAXFILESIZE);
	memset(UFDTArr[i].ptrfiletable->ptrinode->Buffer,0,MAXFILESIZE);

	return i;
}
//This function delete the file
int rm_File(char *name)
{
	int fd=0;
	fd=GetFDFromName(name);
	if(fd==-1)
	{
		return -1;
	}
	(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;
	if(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount==0)
	{
		UFDTArr[fd].ptrfiletable->ptrinode->FileType=0;                         //if file type is 0 then file is deleted
		strcpy(( UFDTArr[fd].ptrfiletable->ptrinode->FileName),"");            //while deleting the file also set the file name to blank
		free( UFDTArr[fd].ptrfiletable->ptrinode->Buffer);
		UFDTArr[fd].ptrfiletable->ptrinode->Buffer=NULL;
		free(UFDTArr[fd].ptrfiletable);
	}
	UFDTArr[fd].ptrfiletable=NULL;
	(SUPERBLOCKobj.FreeInode)++;
	return 0;
}

int ReadFile(int fd,char *arr,int isize)
{
	int read_size=0;
	if(UFDTArr[fd].ptrfiletable==NULL)
	{
		return -1;
	}
	if((UFDTArr[fd].ptrfiletable->Mode!=READ)&&(UFDTArr[fd].ptrfiletable->Mode!=READ+WRITE))
	{
		return -2;
	}
	if((UFDTArr[fd].ptrfiletable->ptrinode->Permission!=READ)&&(UFDTArr[fd].ptrfiletable->ptrinode->Permission!=READ+WRITE))
	{
		return -2;
	}
	if(UFDTArr[fd].ptrfiletable->Readoffset == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
	{
		return -3;
	}
	if(UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)
	{
		return -4;
	}
	read_size=(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)-(UFDTArr[fd].ptrfiletable->Readoffset);
	if(read_size<isize)
	{
		strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->Readoffset),isize);
		(UFDTArr[fd].ptrfiletable->Readoffset) = (UFDTArr[fd].ptrfiletable->Readoffset)+read_size;
	}
	else
	{
		strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->Readoffset),isize);
		(UFDTArr[fd].ptrfiletable->Readoffset) = (UFDTArr[fd].ptrfiletable->Readoffset)+isize;
	}
	return isize;
}

void WriteFile(int fd,char * arr,int isize)
{
 	strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->Writeoffset),arr,isize);
	(UFDTArr[fd].ptrfiletable->Writeoffset)=(UFDTArr[fd].ptrfiletable->Writeoffset)+isize;
	(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)=(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+isize;
}
int OpenFile(char *name,int mode)
{
	int i=0;
	PINODE temp=NULL;
	if(name == NULL || mode<=0)
	{
		return -1;
	}
	temp=Get_Inode(name);
	if(temp==NULL)                    //if file is not exist
	{
		return -2;
	}
	if(temp->Permission < mode)        //if mode is not valid
	{
		return -3;
	}
	while(i<MAXINODE)
	{
		if(UFDTArr[i].ptrfiletable==NULL)
		{
			break;
		}
		i++;
	}
	UFDTArr[i].ptrfiletable=(PFILETABLE)malloc(sizeof(FILETABLE));
	if(UFDTArr[i].ptrfiletable==NULL)
	{
		return -1;
	}
	UFDTArr[i].ptrfiletable->Count=1;
	UFDTArr[i].ptrfiletable->Mode=mode;
	if(mode==READ+WRITE)
	{
		UFDTArr[i].ptrfiletable->Readoffset=0;
		UFDTArr[i].ptrfiletable->Writeoffset=0;
	}
	else if(mode==READ)
	{
		UFDTArr[i].ptrfiletable->Readoffset=0;
	}
	else if(mode==WRITE)
	{
		UFDTArr[i].ptrfiletable->Writeoffset=0;
	}
	UFDTArr[i].ptrfiletable->ptrinode=temp;
	(UFDTArr[i].ptrfiletable->ptrinode->RefernceCount)++;
	return i;
}

void CloseFileByFd(int fd)
{
	UFDTArr[fd].ptrfiletable->Readoffset=0;
	UFDTArr[fd].ptrfiletable->Writeoffset=0;
	(UFDTArr[fd].ptrfiletable->ptrinode->RefernceCount)--;
}
int CloseFileByName(char *name)
{
	int i=0;
	i=GetFDFromName(name);
	if(i==-1)
	{
		return -1;
	}
	UFDTArr[i].ptrfiletable->Readoffset=0;
	UFDTArr[i].ptrfiletable->Writeoffset=0;
	(UFDTArr[i].ptrfiletable->ptrinode->RefernceCount)--;
	return 0;
}
void CloseAllFile()
{
	int i=0;
	while(i<MAXINODE)
	{
		if((UFDTArr[i].ptrfiletable)!=NULL)
		{
			UFDTArr[i].ptrfiletable->Readoffset=0;
	        	UFDTArr[i].ptrfiletable->Writeoffset=0;
         		(UFDTArr[i].ptrfiletable->ptrinode->RefernceCount)--;
			break;
		}
		i++;
	}
}
int LseekFile(int fd,int isize,int from)
{
	if((fd<0)||(from>2))
	{
		return -1;
	}
	if(UFDTArr[fd].ptrfiletable==NULL)
	{
		return -1;
	}
	if(((UFDTArr[fd].ptrfiletable->Mode)==READ) || ((UFDTArr[fd].ptrfiletable->Mode)==READ+WRITE))
	{
		if(from==CURRENT)
		{
			if(((UFDTArr[fd].ptrfiletable->Readoffset)+isize) > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
			{
				return -1;
			}
			if(((UFDTArr[fd].ptrfiletable->Readoffset)+isize)<0)
			{
				return -1;
			}
			(UFDTArr[fd].ptrfiletable->Readoffset)=(UFDTArr[fd].ptrfiletable->Readoffset)+isize;
		}
		else if(from==START)
		{
			if(isize > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
			{
				return -1;
			}
			if(isize<0)
			{
				return -1;
			}
			(UFDTArr[fd].ptrfiletable->Readoffset)=isize;
		}
		else if(from==END)
		{
			if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+isize > MAXFILESIZE)
			{
				return -1;
			}
			if(((UFDTArr[fd].ptrfiletable->Readoffset)+isize)<0)
			{
				return -1;
			}
			(UFDTArr[fd].ptrfiletable->Readoffset)=(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+isize;
		}
	}
	else if((UFDTArr[fd].ptrfiletable->Mode)==WRITE))
	{
		if(from==CURRENT)
		{
			if(((UFDTArr[fd].ptrfiletable->Writeoffset)+isize) > MAXFILESIZE)
			{
				return -1;
			}
			if(((UFDTArr[fd].ptrfiletable->Writeoffset)+isize) < 0)
			{
				return -1;
			}
			if(((UFDTArr[fd].ptrfiletable->Writeoffset)+isize) > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
			{
				(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)=(UFDTArr[fd].ptrfiletable->Writeoffset)+isize;
			}
			(UFDTArr[fd].ptrfiletable->Writeoffset)=(UFDTArr[fd].ptrfiletable->Writeoffset)+isize;
		}
		else if(from==START)
		{
			if(isize>MAXFILESIZE)
			{
				return -1;
			}
			if(isize<0)
			{
				return -1;
			}
			if(isize > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
			{
				(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)=isize;
			}
			(UFDTArr[fd].ptrfiletable->Writeoffset)=isize;
		}
		else if(from==END)
		{
			if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+isize > MAXFILESIZE)
			{
				return -1;
			}
			if(((UFDTArr[fd].ptrfiletable->Writeoffset)+isize) < 0)
			{
				return -1;
			}
			(UFDTArr[fd].ptrfiletable->Writeoffset)=(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+isize ;
		}
	}
}
//This function list the all file in the project
void ls_File()
{
	int i=0;
	PINODE temp=head;
	if(SUPERBLOCKobj.FreeInode==MAXINODE)
	{
		printf("Error : There are no files\n");
		return;
	}
	printf("\nFile Name\tINODE Number\tFile Size\tLink Count\n");
	printf("-----------------------------------------------------\n");
	while(temp!=NULL)
	{
		if(temp->FileType!=0)
		{
			printf("%s\t\t%d\t\t%d\t\t%d\n",temp->FileName,temp->InodeNumber,temp->FileActualSize,temp->LinkCount);
		}
		temp=temp->Next;
	}
	printf("------------------------------------------------------\n");
}
//This function print the file information by taking the file descripter
int Fstat_File(int fd)
{
	PINODE temp=head;
	int i=0;
	if(fd<0)
	{
		return -1;
	}
	if(UFDTArr[fd].ptrfiletable == NULL)
	{
		return -2;
	}
	temp=UFDTArr[fd].ptrfiletable->ptrinode;

	printf("\n-----------Statistical Information about File\n");
	printf("File Name : %s\n",temp->FileName);
	printf("Inode Number : %d\n",temp->InodeNumber);
	printf("File Size : %d\n",temp->FileSize);
	printf("Actual File Size : %d\n",temp->FileActualSize);
	printf("Link count : %d\n",temp->LinkCount);
	printf("Reference Count : %d\n",temp->RefernceCount);

	if(temp->Permission == 1)
	{
		printf("File Permission : Read only\n");
	}
	else if(temp->Permission == 2)
	{
		printf("File Permission : Write\n");
	}
	else if(temp->Permission == 3)
	{
		printf("File Permission : Read & Write\n");
	}
	printf("---------------------------------------------------\n\n");
	return 0;
}
//This function print the statistical information of file by taking File name
int Stat_File(char *name)
{
	PINODE temp=head;
	int i=0;
	if(name==NULL)
	{
		return -1;
	}
	while(temp!=NULL)
	{
		if(strcmp(name,temp->FileName)==0)
		{
			break;
		}
		temp=temp->Next;
	}
	if(temp==NULL)
	{
		return -2;
	}
	printf("\n-----------Statistical Information about File\n");
	printf("File Name : %s\n",temp->FileName);
	printf("Inode Number : %d\n",temp->InodeNumber);
	printf("File Size : %d\n",temp->FileSize);
	printf("Actual File Size : %d\n",temp->FileActualSize);
	printf("Link count : %d\n",temp->LinkCount);
	printf("Reference Count : %d\n",temp->RefernceCount);

	if(temp->Permission == 1)
	{
		printf("File Permission : Read only\n");
	}
	else if(temp->Permission == 2)
	{
		printf("File Permission : Write\n");
	}
	else if(temp->Permission == 3)
	{
		printf("File Permission : Read & Write\n");
	}
	printf("---------------------------------------------------\n\n");
	return 0;
}
//This function remove the file data
int Truncate_File(char *name)
{
	int fd=GetFDFromName(name);
	if(fd==-1)
	{
		return -1;
	}
	memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer,0,1024);              //memset function is used to clean the given memory
	UFDTArr[fd].ptrfiletable->Readoffset=0;
	UFDTArr[fd].ptrfiletable->Writeoffset=0;
	UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize=0;
}

int main()
{
	char *ptr=NULL;
	int ret=0,fd=0,count=0,i=0,len=0;
	char command[4][80],str[80],arr[1024],ch;
	InitialiseSuperBlock();
	CreateDILB();

	while(1)
	{
		//fflush(stdin);
		strcpy(str,"");                            //make str empty evry time
		printf("\nDynamic Custmise File System : >");
		fgets(str,80,stdin);

		count=sscanf(str,"%s%s%s%s",command[0],command[1],command[2],command[3]);
	
		if(count==1)
		{
			if(strcmp(command[0],"ls")==0)
			{
				ls_File();
			}
			else if(strcmp(command[0],"closeall")==0)
			{
				CloseAllFile();
				printf("All Files are close succesfully\n");
				continue;
			}
			else if(strcmp(command[0],"clear")==0)
			{
				system("cls");
				continue;
			}
			else if(strcmp(command[0],"help")==0)
			{
				DisplayHelp();
				continue;
			}
			else if(strcmp(command[0],"exit")==0)
			{
				deallocateDILB();
				printf("Terminating the Dynamic custmize file System\n");
				break;
			}
			else 
			{
				printf("\nERROR : Command not Found !!!\n");
				continue;
			}
		}
		else if(count==2)
		{
		    if(strcmp(command[0],"stat")==0)
			{
				ret=Stat_File(command[1]);
				if(ret==-1)
				{
					printf("ERROR : Incorret parameters\n");
				}
				if(ret==-2)
				{
					printf("ERROR : There is no such file\n");
				}
				continue;
			}
			else if(strcmp(command[0],"fstat")==0)
			{
				ret=Fstat_File(atoi(command[1]));
				if(ret==-1)
				{
					printf("ERROR : Incorret parameters\n");
				}
				if(ret==-2)
				{
					printf("ERROR : There is no such file\n");
				}
				continue;
			}
			else if(strcmp(command[0],"close")==0)
			{
				ret=CloseFileByName(command[1]);
				if(ret==-1)
				{
					printf("ERROR : There is no such file\n");
				}
			}
			else if(strcmp(command[0],"rm")==0)
			{
				ret=rm_File(command[1]);
				if(ret==-1)
				{
					printf("ERROR : There is no such file\n");
				}
			}
			else if(strcmp(command[0],"man")==0)
			{
				Man(command[1]);
			}
			else if(strcmp(command[0],"write")==0)
			{
				fd=GetFDFromName(command[1]);
				if(fd==-1)
				{
					printf("ERROR : Incorrect Parameter\n");
					continue;
				}
				if(((UFDTArr[fd].ptrfiletable->Mode)!=WRITE)&&((UFDTArr[fd].ptrfiletable->Mode)!=READ+WRITE))
				{
					printf("Error : Permission denied\n");
					continue;
				}
				if(((UFDTArr[fd].ptrfiletable->ptrinode->Permission)!=WRITE)&&((UFDTArr[fd].ptrfiletable->ptrinode->Permission)!=READ+WRITE))
				{
					printf("Error : Permission denied\n");
					continue;
				}
				if((UFDTArr[fd].ptrfiletable->Writeoffset)==MAXFILESIZE)
				{
					 printf("Error : There is not sufficient memory to Write to Write\n");
					 continue;
				}
				if((UFDTArr[fd].ptrfiletable->ptrinode->FileType)!=REGULAR)
				{
					printf("Error : It is not regular file\n");
					continue;
				}
				
				i=0;
				printf("Enter the Data : \n");
				while((ch=fgetc(stdin))!='\n')
				{
					if(i<1024)
					{
						arr[i++]=ch;
					}
				}
				len=strlen(arr);
				if(len==0)
				{
					printf("Enter : Incorrect Parameter\n");
					continue;
				}
				WriteFile(fd,arr,len);
				memset(arr,0,MAXFILESIZE);
			}
			else if(strcmp(command[0],"truncate")==0)
			{
				ret=Truncate_File(command[1]);
				if(ret==-1)
				{
					printf("ERROR : Incorrect parameter\n");
				}
			}
			else
			{
				printf("\nERROR : command not found !!!\n");
				continue;
			}
		}
		else if(count==3)
		{
		    if(strcmp(command[0],"create") == 0)
			{
				ret=CreateFile(command[1],atoi(command[2]));
				if(ret>=0)
				{
					printf("File is Succesfully created with the descripter : %d\n",ret);
				}
				if(ret==-1)
				{
					printf("ERROR : Incorrect parameters\n");
				}
				if(ret==-2)
				{
					printf("ERROR : There is no inode\n");
				}
				if(ret==-3)
				{
					printf("ERROR : File already Exists\n");
				}
				if(ret==-4)
				{
					printf("ERROR : Memory allocation failuer\n");
				}
				continue;
			}
			else if(strcmp(command[0],"open")==0)
			{
				ret=OpenFile(command[1],atoi(command[2]));
				if(ret>=0)
				{
					printf("File is Succesfully opened with the descripter : %d\n",ret);
				}
				if(ret==-1)
				{
					printf("ERROR : Incorrect parameters\n");
				}
				if(ret==-2)
				{
					printf("ERROR : File not present\n");
				}
				if(ret==-3)
				{
					printf("ERROR : Permission denied\n");
				}
				continue;
			}
			else if(strcmp(command[0],"read")==0)
			{
				fd=GetFDFromName(command[1]);
				if(fd==-1)
				{
					printf("ERROR : Incorrect parameters\n");
					continue;
				}
				ptr=(char *)malloc(sizeof(atoi(command[2]))+1);
				if(ptr==NULL)
				{
					printf("ERROR : Memory allocation failuer\n");
					continue;
				}
				ret=ReadFile(fd,ptr,atoi(command[2]));
				if(ret==-1)
				{
					printf("ERROR : File not existing\n");
				}
				if(ret==-2)
				{
					printf("ERROR : Permission denied\n");
				}
				if(ret==-3)
				{
					printf("ERROR : Reached at the end of file\n");
				}
				if(ret==-4)
				{
					printf("ERROR : It is not regular file\n");
				}
				if(ret==0)
				{
					printf("ERROR : File empty\n");
				}
				if(ret>0)
				{
					//printf("%s",ptr);
					write(2,ptr,ret);               //write is system call which take 2 as file discripter which indicate system.out
				}
				continue;
			}
			else
			{
				printf("\nERROR : Command not Found !!!\n");
				continue;
			}
		}
		else if(count==4)
		{
			if(strcmp(command[0],"lseek")==0)
			{
				fd=GetFDFromName(command[1]);
				if(fd==-1)
				{
					printf("ERROR : INcorrect Parameter\n");
					continue;
				}
				ret=LseekFile(fd,atoi(command[2]),atoi(command[3]));
				if(ret==-1)
				{
					printf("ERROR : Unable to perform lseek\n");
				}
			}
			else
			{
				printf("\nERROR : Command not found !!!\n");
				continue;
			}
		}
		else
		{
			printf("\nERROR : Command not found !!!!\n");
			continue;
		}
	}
	return 0;
}
