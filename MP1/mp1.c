#include "list_file.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include "md5.h"  

unsigned char str[104857610],tmpstr[1000];
typedef struct{
    char fname[256];
    unsigned char md[16];
    char trans[33];
    int type;
}Fileid;

typedef struct{
    char org[256];
    char new[256];
}Copied;

int  cmp( const void *a, const void *b)
{
    return( strcmp(*(char **)a,*(char **)b) );
}

int cpcmp( const void *a, const void *b)
{
    Copied *aa=(Copied*)a;
    Copied *bb=(Copied*)b;
    return (strcmp(aa->org,bb->org));
}

int cgcmp( const void *a, const void *b)
{
    Fileid *aa=(Fileid*)a;
    Fileid *bb=(Fileid*)b;
    return (strcmp(aa->fname,bb->fname));
}

int bs( Fileid data[],char str[],int n)
{
    int low = 0, high = n - 1,tmp;

    while (low <= high)
    {
        int mid = (low + high) / 2;
        tmp=strcmp(data[mid].fname,str);
        if (tmp==0) return mid;
        else if(tmp>0) high = mid - 1;
        else if(tmp<0) low = mid + 1;
    }

    return -1;
}

Fileid trs(Fileid aa)
{
    Fileid bb=aa;
    for(int k=0;k<16;k++)
    {
        int i,y=0,x,j;
        unsigned char a[16],b[16];
        b[0]=bb.md[k];
        for(i=0;i<=b[0];i++) 
        {
    
            a[i]=b[i]%16; 
            b[i+1]=b[i]/16; 
            y++; 
            if(b[i+1]==0){break;} 
        }
        if(y==1)
        {
            bb.trans[2*k]='0';
            switch(a[0]) 
            {
                case 10:bb.trans[2*k+1]='a';break; 
                case 11:bb.trans[2*k+1]='b';break; 
                case 12:bb.trans[2*k+1]='c';break; 
                case 13:bb.trans[2*k+1]='d';break; 
                case 14:bb.trans[2*k+1]='e';break; 
                case 15:bb.trans[2*k+1]='f';break; 
                default:bb.trans[2*k+1]=(a[0]+'0');break;
            } 
        }
        else 
        {
            for(j=y-1;j>=0;j--) 
            {
                x=a[j]; 
                switch(x) 
                {
                    case 10:bb.trans[2*k+1-j]='a';break; 
                    case 11:bb.trans[2*k+1-j]='b';break; 
                    case 12:bb.trans[2*k+1-j]='c';break; 
                    case 13:bb.trans[2*k+1-j]='d';break; 
                    case 14:bb.trans[2*k+1-j]='e';break; 
                    case 15:bb.trans[2*k+1-j]='f';break; 
                    default:bb.trans[2*k+1-j]=(a[j]+'0');break;
                } 
            }
        }
    }
    bb.trans[33]='\0';
    return bb;
}
int main(int argc, char *argv[])  
{
    MD5_CTX md5;
    int fd,n=0,co=0,m=0,cmd=0,fpoint=-1;
    char nf[1005][256],mod[1005][256],lr[15]="/.loser_record",tmpname[256],cg[15]="/.loser_config";
    Fileid file[1005],old[1005],confg[1005];
    Copied cp[1005];
    strcpy(tmpname,argv[argc-1]);
    strcat(tmpname,cg);
    FILE *fcg=fopen(tmpname,"r");
    if(fcg!=NULL)
    {
        int i=0,pt;
        char nick[7];
        while(fscanf(fcg,"%s = %s\n",confg[i].fname,nick)==2)
        {
            if(strcmp(nick,"status")==0) confg[i].type=1;
            else if(strcmp(nick,"commit")==0) confg[i].type=2;
            else if(strcmp(nick,"log")==0) confg[i].type=3;
            i++;
            memset(nick,'\0',7);
        }
        qsort((void*)confg,i,sizeof(confg[0]),cgcmp);
        pt=bs(confg,argv[1],i);
        if(pt!=-1) cmd=confg[pt].type;
        fclose(fcg);
    }
    memset(tmpname,'\0',256);
    if(strcmp(argv[1],"status")==0 || cmd==1)
    {
        struct FileNames file_names = list_file(argv[2]);
        qsort((void*)file_names.names,file_names.length,sizeof(file_names.names[0]),cmp);
        strcpy(tmpname,argv[2]);
        strcat(tmpname,lr);
        fd=open(tmpname,O_RDONLY);
        if(fd<0)
        {
            printf("[new_file]\n");
            for(int i=2;i<file_names.length;i++)
                printf("%s\n",file_names.names[i]);
            printf("[modified]\n");
            printf("[copied]\n");

            return 0;
        }
        else
        {
            FILE *f=fopen(tmpname,"r");
            fseek(f,0,SEEK_END);
            int last=ftell(f);
            for(int i=1;i<last;i++)
            {
                fseek(f,-i,SEEK_END);
                char c=getc(f);
                if(c=='\n')
                {
                    fseek(f,-2,SEEK_CUR);
                    if((c=getc(f))==')') break;
                }
            }
            fseek(f,1,SEEK_CUR);
            last=0;
            while(fscanf(f,"%s %s\n",old[last].fname,old[last].trans)==2)
            {
                last++;
            }
            fclose(f);
            for(size_t i = 2; i < file_names.length; i++)
            {
                if(strcmp(file_names.names[i],".loser_record")==0) continue;
                strcpy(file[i].fname,file_names.names[i]);
                memset(tmpname,'\0',256);
                strcpy(tmpname,argv[2]);
                strcat(tmpname,"/");
                strcat(tmpname,file_names.names[i]);
                fd=open(tmpname,O_RDONLY);
                fpoint=lseek(fd,0,SEEK_END);
                lseek(fd,0,SEEK_SET);
                read(fd,str,fpoint);
                MD5Init(&md5);
                MD5Update(&md5,str,fpoint);
                MD5Final(&md5,file[i].md);
                file[i]=trs(file[i]);
                memset(str,'\0',fpoint);
                //printf("%s->%s\n", file[i].fname,file[i].trans);
		  	    close(fd);
                int fnd=bs(old,file[i].fname,last);
                if(fnd!=-1 && strcmp(old[fnd].trans,file[i].trans)!=0) 
                {
                    strcpy(mod[m++],file[i].fname);
                }
                else if(fnd==-1)
                {
                    int flag=0;
                    for(int j=0;j<last;j++)
                    {
                        if(strcmp(old[j].trans,file[i].trans)==0)
                        {
                            strcpy(cp[co].org,old[j].fname);
                            strcpy(cp[co++].new,file[i].fname);
                            flag=1;
                            break;
                        }
                    }
                    if(!flag) strcpy(nf[n++],file[i].fname);
                }
    	    }
            qsort(cp,co,sizeof(cp[0]),cpcmp);
            printf("[new_file]\n");
            for(int i=0;i<n;i++)
                printf("%s\n",nf[i]);
            printf("[modified]\n");
            for(int i=0;i<m;i++)
                printf("%s\n",mod[i]);
            printf("[copied]\n");
            for(int i=0;i<co;i++)
                printf("%s => %s\n",cp[i].org,cp[i].new);
        }
		free_file_names(file_names);
    }
    else if(strcmp(argv[1],"commit")==0 || cmd==2)
    {
        struct FileNames file_names = list_file(argv[2]);
        if(file_names.length==2) return 0;
        qsort((void*)file_names.names,file_names.length,sizeof(file_names.names[0]),cmp);
        strcpy(tmpname,argv[2]);
        strcat(tmpname,lr);
        fd=open(tmpname,O_RDONLY);
        if(fd<0)
        {
            fd=open(tmpname,O_RDWR|O_CREAT,0700);
            write(fd,"# commit 1\n",11);
            write(fd,"[new_file]\n",11);
            for(int i=2;i<file_names.length;i++)
            {
                write(fd,file_names.names[i],strlen(file_names.names[i]));
                write(fd,"\n",1);
            }
            write(fd,"[modified]\n",11);
            write(fd,"[copied]\n",9);
            write(fd,"(MD5)\n",6);
            close(fd);
            FILE *f=fopen(tmpname,"a");
            for(int i=2;i<file_names.length;i++)
            {
                memset(tmpname,'\0',256);
                strcpy(tmpname,argv[2]);
                strcat(tmpname,"/");
                strcat(tmpname,file_names.names[i]);
                fd=open(tmpname,O_RDONLY);
                fpoint=lseek(fd,0,SEEK_END);
                lseek(fd,0,SEEK_SET);
                read(fd,str,fpoint);
                MD5Init(&md5);
                MD5Update(&md5,str,fpoint);
                MD5Final(&md5,file[i].md);
                memset(str,'\0',fpoint);
                close(fd);
                file[i]=trs(file[i]);
                fprintf(f, "%s ",file_names.names[i]);
                fprintf(f, "%s",file[i].trans);
                fprintf(f, "\n");
            }
            fclose(f);
        }
        else
        {
            FILE *f=fopen(tmpname,"r");
            fseek(f,0,SEEK_END);
            int last=ftell(f),cmtnum;
            for(int i=1;i<=last;i++)
            {
                fseek(f,-i,SEEK_END);
                char c=getc(f);
                if(c=='#')
                {
                    fseek(f,8,SEEK_CUR);
                    fscanf(f,"%d",&cmtnum);
                    break;
                }
            }
            for(int i=1;i<last;i++)
            {
                char c=getc(f);
                if(c=='\n')
                {
                    if((c=getc(f))=='(')
                    {
                        fseek(f,5,SEEK_CUR);
                        break;
                    }
                }
            }
            last=0;
            while(fscanf(f,"%s %s\n",old[last].fname,old[last].trans)==2)
            {
                last++;
            }
            fclose(f);
            for(size_t i = 2; i < file_names.length; i++)
            {
                if(strcmp(file_names.names[i],".loser_record")==0) continue;
                strcpy(file[i].fname,file_names.names[i]);
                memset(tmpname,'\0',256);
                strcpy(tmpname,argv[2]);
                strcat(tmpname,"/");
                strcat(tmpname,file_names.names[i]);
                fd=open(tmpname,O_RDONLY);
                fpoint=lseek(fd,0,SEEK_END);
                lseek(fd,0,SEEK_SET);
                read(fd,str,fpoint);
                MD5Init(&md5);
                MD5Update(&md5,str,fpoint);
                MD5Final(&md5,file[i].md);
                file[i]=trs(file[i]);
                //printf("%s->%s\n", file[i].fname,file[i].trans);
                memset(str,'\0',fpoint);
                close(fd);
                int fnd=bs(old,file[i].fname,last);
                if(fnd!=-1 && strcmp(old[fnd].trans,file[i].trans)!=0) 
                {
                    strcpy(mod[m++],file[i].fname);
                }
                else if(fnd==-1)
                {
                    int flag=0;
                    for(int j=0;j<last;j++)
                    {
                        if(strcmp(old[j].trans,file[i].trans)==0)
                        {
                            strcpy(cp[co].org,old[j].fname);
                            strcpy(cp[co++].new,file[i].fname);
                            flag=1;
                            break;
                        }
                    }
                    if(!flag) strcpy(nf[n++],file[i].fname);
                }
            }
            qsort(cp,co,sizeof(cp[0]),cpcmp);
            if(n+co+m==0) return 0;
            memset(tmpname,'\0',256);
            strcpy(tmpname,argv[2]);
            strcat(tmpname,lr);
            f=fopen(tmpname,"a");
            fprintf(f, "\n# commit %d\n",cmtnum+1);
            fprintf(f,"[new_file]\n");
            for(int i=0;i<n;i++)
                fprintf(f,"%s\n",nf[i]);
            fprintf(f,"[modified]\n");
            for(int i=0;i<m;i++)
                fprintf(f,"%s\n",mod[i]);
            fprintf(f,"[copied]\n");
            for(int i=0;i<co;i++)
                fprintf(f,"%s => %s\n",cp[i].org,cp[i].new);
            fprintf(f,"(MD5)\n");
            for(int i=2;i<file_names.length;i++)
            {
                if(strcmp(file_names.names[i],".loser_record")==0) continue;
                fprintf(f, "%s %s\n",file[i].fname,file[i].trans);
            }
        }
        free_file_names(file_names);
    }
    else if(strcmp(argv[1],"log")==0 || cmd==3)
    {
        int l=strlen(argv[2]),cmtnum=0;
        for(int i=0;i<l;i++)
        {
            int a=(argv[2][i]-'0');
            for(int j=i+1;j<l;j++)
            {
                a*=10;
            }
            cmtnum+=a;
        }
        strcpy(tmpname,argv[3]);
        strcat(tmpname,lr);
        FILE *f=fopen(tmpname,"r");
        if(cmtnum>0 && f!=NULL)
        {
            fseek(f,0,SEEK_END);
            int last=ftell(f),cnt=0;
            int end=last;
            for(int i=1;i<=last;i++)
            {
                fseek(f,-i,SEEK_END);
                int tail=0;
                if(ftell(f)==0) tail=1;
                char c=getc(f);
                if(c=='#')
                {
                    cnt++;
                    fseek(f,-1,SEEK_CUR);
                    int head=ftell(f);
                    for(int j=head;j<end;j++)
                    {
                        c=fgetc(f);
                        printf("%c",c);
                    }
                    end=head-1;
                    if(cnt==cmtnum) return 0;
                    if(!tail) printf("\n");
                }
            }
            fclose(f);
        }
        else return 0;
    }
    return 0;
}  
