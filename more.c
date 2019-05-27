
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

int PAGELEN =20;
#define LINELEN 512
extern char environ;

void do_more(FILE *,char*);
int  get_input(FILE*, int*);
void Page_Size_Updation();
float count=0.0;	//counting the number of lines of files
int pagesize=0;		//calculating page size in this variable

int main(int argc , char *argv[])
{
   int i=0;
   if (argc == 1)
   {
      do_more(stdin,argv[i]);
   }
   FILE * fp;
   FILE *fp1;
   while (++i < argc)
   {
      fp1 = fopen(argv[i],"r");
      char chr;
      if (fp1 == NULL)
      {
         perror("Can't open file");
         exit (1);
      } 
      else
      {
          chr=getc(fp1);
         while(chr !=EOF)
         {
            if(chr=='\n')
            count=count+1;
            chr=getc(fp1);
         }
      }
   }
   i=0;
   fclose(fp1);
   while(++i < argc)
   {
      fp = fopen(argv[i] , "r");
      if (fp == NULL)
      {
         perror("Can't open file");
         exit (1);
      } 
      do_more(fp,argv[i]);
      fclose(fp);
   }  
   return 0;
}

 
void do_more(FILE *fp, char* fileName)		//first argument is file pointer and the second argument is file name that is to be opened
{
   int a=0;
   struct termios old_tio, new_tio;
   int res=0;
   res=tcgetattr(STDIN_FILENO, &old_tio);		//getting terminal attributes
   memcpy(&new_tio, &old_tio , sizeof(new_tio));		//copyting memory to be retored later after process completed
/* disable canonical mode (buffered i/o) and local echo */
   new_tio.c_lflag &=(~ICANON );/*& ~ECHOE  );*/
   tcsetattr(STDIN_FILENO,TCSANOW,&new_tio);	//setting the new terminal attributes
   Page_Size_Updation();   		//update the page size
   int num_of_lines = 0;		//this will store the number of lines of the file
   int rv;
   char buffer[LINELEN];		//this will be used to read data from the file
   FILE* fp_tty = fopen("/dev//tty", "r");
   char ch;
   char searchk[100],pre1[100],pre2[100];		//searched word + previous word then searched + previous word than the first previous
   while (fgets(buffer, LINELEN, fp))			//read from file
   {
      fputs(buffer, stdout);					//print on terminal
      num_of_lines++;
      a++;
      if (num_of_lines >= PAGELEN)				//if page size is completed
      {
         rv = get_input(fp_tty,&a);				//read input from the user
         if (rv == 0)
         {//user pressed q
            printf(" \033[2K \033[1G");			
            break;								//if q is pressed stop reading text from file
         }
         else if (rv == 1)						//if space is pressed then decrement the lines for one complete page 
         {//user pressed space bar
            printf(" \033[2K \033[1G");   
            num_of_lines -= PAGELEN;
         }
         else if (rv == 2)
         {//user pressed return/enter
            printf("\033[1A \033[2K \033[1G");		//if enter is pressed then decrement one line to read one more line from the file in next iteration
           num_of_lines -= 1; //show one more line
         }
         else if(rv==3)
         {//if user pressed /
            printf(" \033[2K \033[1G");
            printf("/");
            fgets(searchk,100,stdin);     
            while(fgets(buffer,LINELEN,fp))
            {
               if(strcmp(buffer,searchk)==0)		//if string is found
               {
                  printf("..../skipping\n");
                  printf("%s",pre2 );
                  printf("%s",pre1 );
                  printf("%s",searchk );
                  break;
               }
               strcpy(pre2,pre1);
               strcpy(pre1,buffer);
               num_of_lines=3;
               a++;
            }
         }
         else if (rv == 4)
         {//if user pressed v
            printf("\033[1A \033[2K \033[1G");
            int pid=vfork();
            if(pid==0)
            {
               execlp("vim","vim",fileName,NULL);		//opening file in vim using child process
               exit(0);
             }
            else
            {
               wait(0);
            }
         }
         else if (rv == 5)
         { //invalid character
            printf("\033[1A \033[2K \033[1G");
            break; 
         }
      }
  }
  res = tcsetattr(STDIN_FILENO,TCSANOW,&old_tio);
}
int get_input(FILE* cmdstream,int *a)
{
   int c; 
   float avg=(*a/count)*100;
   Page_Size_Updation();
   printf("\033[7m --more--(%.0f%%) \033[m",avg);
     c = getc(cmdstream);
      if(c == 'q')
         return 0;
      if ( c == ' ' )   
        return 1;
      if ( c == '\n' )  
        return 2; 
      if(c == '/')
         return 3;
      if (c == 'v' || c == 'V')
      {
         printf("\033[2K \033[1G"); 
         return 4;
      }
      
     return 5;
   return 0;
}
void Page_Size_Updation()
{
   struct winsize ws;
   ioctl(2,TIOCGWINSZ,&ws);
   PAGELEN=ws.ws_row-1;
}