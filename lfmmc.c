
///////////
// LFMMC  
///////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <dirent.h>
#include <unistd.h>  
#include <time.h>

#define PATH_MAX 2500
#define STMAX 2040

#define ESC "\033"

//Format text
#define RESET 		0
#define BRIGHT 		1
#define DIM			2
#define UNDERSCORE	3
#define BLINK		4
#define REVERSE		5
#define HIDDEN		6
//Foreground Colours (text)
#define F_BLACK 	30
#define F_RED		31
#define F_GREEN		32
#define F_YELLOW	33
#define F_BLUE		34
#define F_MAGENTA 	35
#define F_CYAN		36
#define F_WHITE		37
//Background Colours
#define B_BLACK 	40
#define B_RED		41
#define B_GREEN		42
#define B_YELLOW	44
#define B_BLUE		44
#define B_MAGENTA 	45
#define B_CYAN		46
#define B_WHITE		47

#define home() 			printf(ESC "[H") //Move cursor to the indicated row, column (origin at 1,1)
#define clrscr()		printf(ESC "[2J") //clear the screen, move to (1,1)
#define gotoyx(y,x)		printf(ESC "[%d;%dH", y, x);
#define    visible_cursor() printf(ESC "[?251");
#define nonvisible_cursor() printf(ESC "[?251");
#define resetcolor() printf(ESC "[0m")
#define set_display_atrib(color) 	printf(ESC "[%dm",color)


int sx1, sx2, sy1, sy2 ;
int rows, cols ; 
int pansel = 1;
int autorefresh = 0;
int show_dir   = 2;
int show_frame = 0;
int show_clock = 0;
int show_title = 0;
int show_path = 0;

char file_filter[10][PATH_MAX];



///////////////// new
char *fextension(char *str)
{ 
    char ptr[strlen(str)+1];
    int i,j=0;
    //char ptrout[strlen(ptr)+1];  
    char ptrout[25];

    if ( strstr( str, "." ) != 0 )
    {
      for(i=strlen(str)-1 ; str[i] !='.' ; i--)
      {
        if ( str[i] != '.' ) 
            ptr[j++]=str[i];
      } 
      ptr[j]='\0';

      j = 0; 
      for( i=strlen(ptr)-1 ;  i >= 0 ; i--)
            ptrout[j++]=ptr[i];
      ptrout[j]='\0';
    }
    else
     ptrout[0]='\0';

    size_t siz = sizeof ptrout ; 
    char *r = malloc( sizeof ptrout );
    return r ? memcpy(r, ptrout, siz ) : NULL;
}



void nsystem( char *mycmd )
{
   printf( "<SYSTEM>\n" );
   printf( " >> CMD:%s\n", mycmd );
   system( mycmd );
   printf( "</SYSTEM>\n");
}




void nrunwith( char *cmdapp, char *filesource )
{
           char cmdi[PATH_MAX];
           strncpy( cmdi , "  " , PATH_MAX );
           strncat( cmdi , cmdapp , PATH_MAX - strlen( cmdi ) -1 );
           strncat( cmdi , " " , PATH_MAX - strlen( cmdi ) -1 );
           strncat( cmdi , " \"" , PATH_MAX - strlen( cmdi ) -1 );
           strncat( cmdi ,  filesource , PATH_MAX - strlen( cmdi ) -1 );
           strncat( cmdi , "\" " , PATH_MAX - strlen( cmdi ) -1 );
           nsystem( cmdi ); 
}







void gfxframe( int y1, int x1, int y2, int x2 )
{
    int foo, fooy , foox ;
    foo = x1;
    for( fooy = y1 ; fooy <= y2 ; fooy++) 
    {
        gotoyx( fooy , foo );  
        printf( "|" );
    }

    foo = x2;
    for( fooy = y1 ; fooy <= y2 ; fooy++) 
    {
         gotoyx( fooy , foo );  
         printf( "|" );
    }
    foo = y1;
    for( foox = x1 ; foox <= x2 ; foox++) 
    {
         gotoyx( foo , foox );  
         printf( "-" );
    }
    foo = y2;
    for( foox = x1 ; foox <= x2 ; foox++) 
    {
         gotoyx( foo , foox );  
         printf( "-" );
    }
}


static struct termios oldt;
void restore_terminal_settings(void)
{
    tcsetattr(0, TCSANOW, &oldt);  /* Apply saved settings */
}

void enable_waiting_for_enter(void)
{
    tcsetattr(0, TCSANOW, &oldt);  /* Apply saved settings */
}

void disable_waiting_for_enter(void)
{
    struct termios newt;

    /* Make terminal read 1 char at a time */
    tcgetattr(0, &oldt);  /* Save terminal settings */
    newt = oldt;  /* Init new settings */
    newt.c_lflag &= ~(ICANON | ECHO);  /* Change settings */
    tcsetattr(0, TCSANOW, &newt);  /* Apply settings */
    atexit(restore_terminal_settings); /* Make sure settings will be restored when program ends  */
}






char userstr[PATH_MAX];
/////////////////////////////////////
void strninput( char *mytitle, char *foostr )
{
      strncpy( userstr , "" , PATH_MAX );
      disable_waiting_for_enter();
      char strmsg[PATH_MAX];
      char charo[PATH_MAX];
      int foousergam = 0; int ch ;  int chr;

      strncpy( strmsg, ""  ,  PATH_MAX );
      strncpy( strmsg, foostr , PATH_MAX );

      int j; 
      char ptr[PATH_MAX];
      char str[PATH_MAX];

      while( foousergam == 0 ) 
      {

         gotoyx( sy2, 0 );
         for ( chr = 0 ;  chr <= cols-1 ; chr++) printf( " ");
         gotoyx( sy2, 0 );
         printf( ": %s", strmsg );

         ch = getchar();
         if ( ch == 10 )            foousergam = 1;

	 else if ( ch == 27 ) 
	      strncpy( strmsg, ""  ,  PATH_MAX );

	 else if ( ch == 2 ) 
	      strncpy( strmsg, ""  ,  PATH_MAX );

	 else if ( ch == 4 ) 
	 {      
            snprintf( charo, PATH_MAX , "%s%d",  strmsg, (int)time(NULL));
	    strncpy( strmsg,  charo ,  PATH_MAX );
         }

	 else if ( ( ch == 8 )  || ( ch == 127 ) )  
         {
            if ( strlen( strmsg ) >= 2 ) 
            {
              j = 0; strncpy(  ptr , "" ,  PATH_MAX );
              for ( chr = 0 ;  chr <= strlen( strmsg )-2 ; chr++) 
              {
                 ptr[j++] = strmsg[chr];
              }
	      strncpy( strmsg, ptr  ,  PATH_MAX );
            }
            else
	      strncpy( strmsg, ""  ,  PATH_MAX );
         }

	 else if (
			(( ch >= 'a' ) && ( ch <= 'z' ) ) 
		        || (( ch >= 'A' ) && ( ch <= 'Z' ) ) 
		        || (( ch >= '1' ) && ( ch <= '9' ) ) 
		        || (( ch == '0' ) ) 
		        || (( ch == '~' ) ) 
		        || (( ch == '!' ) ) 
		        || (( ch == '&' ) ) 
		        || (( ch == '=' ) ) 
		        || (( ch == ':' ) ) 
		        || (( ch == ';' ) ) 
		        || (( ch == '<' ) ) 
		        || (( ch == '>' ) ) 
		        || (( ch == ' ' ) ) 
		        || (( ch == '|' ) ) 
		        || (( ch == '#' ) ) 
		        || (( ch == '?' ) ) 
		        || (( ch == '+' ) ) 
		        || (( ch == '/' ) ) 
		        || (( ch == '\\' ) ) 
		        || (( ch == '.' ) ) 
		        || (( ch == '$' ) ) 
		        || (( ch == '%' ) ) 
		        || (( ch == '-' ) ) 
		        || (( ch == ',' ) ) 
		        || (( ch == '{' ) ) 
		        || (( ch == '}' ) ) 
		        || (( ch == '(' ) ) 
		        || (( ch == ')' ) ) 
		        || (( ch == ']' ) ) 
		        || (( ch == '[' ) ) 
		        || (( ch == '*' ) ) 
		        || (( ch == '"' ) ) 
		        || (( ch == '@' ) ) 
		        || (( ch == '-' ) ) 
		        || (( ch == '_' ) ) 
		        || (( ch == '^' ) ) 
		        || (( ch == '\'' ) ) 
	             ) 
		  {
                        snprintf( charo, PATH_MAX , "%s%c",  strmsg, ch );
		        strncpy( strmsg,  charo ,  PATH_MAX );
		  }
     }
     strncpy( userstr, strmsg , PATH_MAX );
}





/////////////////////////
/////////////////////////
int  nexp_user_sel[5] ; 
int  nexp_user_scrolly[5] ;
char nexp_user_fileselection[PATH_MAX]; 
int  tc_det_dir_type = 0;
/////////////////////////
void printdir( int pyy, int fopxx, char *mydir , int panviewpr )
{
   int pxx = fopxx+1;
   if ( pxx == 0 ) pxx = 2;
   DIR *dirp; int posy = 0;  int posx, chr ; 
   int fooselection = 0;

   posy = 1; posx = cols/2;
   posy = pyy-1; 

   char cwd[PATH_MAX];
   struct dirent *dp;
   dirp = opendir( mydir  );
   int entrycounter = 0;
   fooselection = 0;
   while  ((dp = readdir( dirp )) != NULL ) 
   if ( posy <= sy2-1 )
   {
        entrycounter++;
        if ( entrycounter <= nexp_user_scrolly[panviewpr] )
              continue;

        if ( strcmp(  file_filter[panviewpr] , "" ) != 0 ) 
        {
           if ( strstr( dp->d_name, file_filter[panviewpr] ) == 0 ) 
              continue;
        }

        if (  dp->d_name[0] !=  '.' ) 
        if (  strcmp( dp->d_name, "." ) != 0 )
        if (  strcmp( dp->d_name, ".." ) != 0 )
        {
            posy++;  fooselection++;
            if ( dp->d_type == DT_DIR ) 
            {
                 gotoyx( posy, pxx );
                 printf( "/" );
                 posx++;
            }
            else if ( dp->d_type == 0 )
            {
                 gotoyx( posy, pxx );
                 printf( "/" );
                 posx++;
            }

            if ( nexp_user_sel[ panviewpr ] == fooselection ) 
            {
                  if ( panviewpr == pansel )
                  {
                    gotoyx( posy, pxx-1 );
                    strncpy( nexp_user_fileselection, dp->d_name , PATH_MAX );
                    printf( ">" );
                  }
            }
            else 
            {
                  gotoyx( posy, fopxx );
                  printf( " " );
            }

            gotoyx( posy, pxx );
            if ( dp->d_type == DT_DIR ) 
               printf( " [%s]\n",  dp->d_name );
            else 
               printf( " %s\n",  dp->d_name );

            /*gotoyx( posy, pxx );
            for ( chr = 0 ;  chr <= strlen(dp->d_name) ; chr++) 
            {
              if  ( dp->d_name[chr] == '\n' )
              {    //posx = cols/2;
              }
              else if  ( dp->d_name[chr] == '\0' )
              {    //posx = cols/2;
              }
              else
              {  
                 printf( "%c", dp->d_name[chr] );
                 posx++;
              }
            }*/
        }
   }
   closedir( dirp );
}





///////////////////////////////////////////
///////////////////////////////////////////
///////////////////////////////////////////
void readfilesp( char *filesource, int linestart , int lineend )
{
  FILE *source; 
  int ch ;  int linecount = 1;
  source = fopen( filesource , "r");
  if ( source == NULL ) { printf( "File not found.\n" ); } else  
  {
   clrscr();
   //for ( ch = 0 ;  ch <= cols-1 ; ch++) printf( "%c", '-');
     printf( "     FILE: %s\n", filesource );
   //for ( ch = 0 ;  ch <= cols-1 ; ch++) printf( "%c", '-');
   //  printf( "%d: ", linecount );
   printf( "     ");
   while( ( ch = fgetc(source) ) != EOF )
   {
       if ( linecount <= lineend +3 ) 
       {
         if ( ch == '\n' ) 
         {
            linecount++; 
            printf( "\n");
            printf( "     ");
            printf( "%d: ", linecount );
         }
         else
            printf( "%c", ch );
       }
   }
   fclose(source);
   }

   printf( "\33[2K" ); 
   printf( "\r" );
   for ( ch = 0 ;  ch <= cols-1 ; ch++) printf( "%c", '-');
}



   




////////////////////////////////////////
int main( int argc, char *argv[])
{
  char string[STMAX];
  char strpath[STMAX];
  char cwd[STMAX];
  FILE *fptt;
  strncpy( file_filter[1], "", PATH_MAX );
  strncpy( file_filter[2], "", PATH_MAX );


  ////////////////////////////////////////////////////////
  if ( argc == 2)
  if ( strcmp( argv[1] , "" ) !=  0 )
  {
          chdir( argv[ 1 ] );
          //strncpy( pathpan[ 1 ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
          //strncpy( pathpan[ 2 ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
  }

  struct winsize w; // need ioctl and unistd 
  ioctl( STDOUT_FILENO, TIOCGWINSZ, &w );
  clrscr();
  home();
  sy2   = w.ws_row  - 10; 
  sx2   = w.ws_col  - 10;
  sy1   = 2; 
  sx1   = 2;
  rows = sy2; cols = sx2; 

  long t;
  struct tm *ltime;


  char userstrsel[PATH_MAX];

  sx1 = 5; pansel = 1;
  nexp_user_sel[pansel] = 1;
  int ch , gameover ; 
  gameover = 0;
  while( gameover == 0 )
  {
      disable_waiting_for_enter();
      clrscr();
      home();


     //set_display_atrib(B_BLUE); set_display_atrib(F_YELLOW);
     //gotoyx(7,10);
     //puts(	"┌─────────┐\n" );
     if ( show_dir == 0 )
     {    
      gotoyx(sy1+2,sx1+5);
      //set_display_atrib(B_BLUE); set_display_atrib(F_WHITE);
      printf( "PATH:\n" );
    
      gotoyx(sy1+3, sx1+5);
      //set_display_atrib(B_BLUE); set_display_atrib(F_WHITE);
      printf( "[%s]\n", getcwd( string, STMAX ));

      gotoyx( sy2-5, sx1+5);
      //set_display_atrib(B_BLUE); set_display_atrib(F_WHITE);
      printf( "-LFM-\n"  );
      gotoyx( sy2-4, sx1+5);
      printf( "=Fast Light File Manager=\n" );
    
      //printf("Screen\n" );
      //printf("Env HOME:  %s\n", getenv( "HOME" ));
      //printf("Env PATH:  %s\n", getcwd( string, STMAX ) );
      //printf("Env TERM ROW:  %d\n", w.ws_row );
      //printf("Env TERM COL:  %d\n", w.ws_col );
    
      gotoyx( sy1 + (sy2-sy1)/2, sx1 + (sx2-sx1)/2);
      //set_display_atrib(B_BLUE); set_display_atrib(F_WHITE);
      printf( "Clock\n" );

      gotoyx( sy1 + (sy2-sy1)/2 +1 , sx1 + (sx2-sx1)/2);
      time(&t);
      ltime=localtime(&t);
      printf( "[TIME :%02d:%02d:%02d]\n", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);
    
      gotoyx( sy2-2, sx2-12);
      printf( "|KEY?|" );
    
      gotoyx( sy1+1, sx1+1 );
      printf( "%d,%d,%d,%d", sy1, sx1, sy2, sx2 );

      gfxframe( sy1, sx1, sy2, sx2 );
     }    

     else if ( show_dir == 2 )
     {     printdir( sy1+1 , sx1+1 , "." , 1 );
     }

     else if ( show_dir == 1 )
     {
        gotoyx( sy1 + (sy2-sy1)/2 +1 , sx1 + (sx2-sx1)/2);
        time(&t);
        ltime=localtime(&t);
        printf( "[TIME :%02d:%02d:%02d]\n", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);
        gotoyx( sy1+1, sx1+1 );
        printf( "%d,%d,%d,%d", sy1, sx1, sy2, sx2 );
     }    


     if ( show_frame == 1 ) gfxframe( sy1, sx1, sy2, sx2 );
     if ( show_clock == 1 ) 
     {
       gotoyx( sy2-1, sx2-20 );
       time(&t);
       ltime=localtime(&t);
       printf( "[TIME :%02d:%02d:%02d]\n", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);
     }

     if ( show_path == 1 ) 
     {
       gotoyx( sy2, sx1+5-2 );
       printf( "[PATH :%s]" , getcwd( cwd, PATH_MAX ) );
     }


     rows = sy2-1; 

     if ( show_title == 1 )    
     {
        gotoyx( sy1, sx1 + ( sx2 - sx1 )/2 ); 
        printf( " =LFMMC= " );  
        //gotoyx( sy2-1, sx2-20 );
        //time(&t);
        //ltime=localtime(&t);
        //printf( "[TIME :%02d:%02d:%02d]\n", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);
     }

     gotoyx( sy1, sx1 ); 

     if ( autorefresh != 0 ) 
     {
       gotoyx( sy2-2, sx1+5 );
       printf( "[AUTOREFRESH]");
       gotoyx( sy1, sx1 ); 
       printf( " " );
       ch = 0; 
       if      ( autorefresh == 1 ) usleep( 20      * 10000 );
       else if ( autorefresh == 2 ) usleep(   10 * 20 * 10000 );     // 2sec
       else if ( autorefresh == 6 ) usleep( 3 * 10 * 20 * 10000 );   // 6sec
     }
     else
         ch = getchar();

      if ( ch == 'Q' ) 
          gameover = 1; 



      /// check val here ch
      else if ( ch == 7)        // gctrl g 
      {
          printf( "ctrl+g: quick key.\n");
          ch = getchar();
          if ( ch == 'w' )
          {
            chdir( getenv( "HOME" ));
            chdir( "workspace" );
            nexp_user_sel[pansel]=1; nexp_user_scrolly[pansel] = 0; 
            strncpy( file_filter[pansel]  , "" , PATH_MAX );
          }
          ch = 0 ; 
      }



      //// dir mode
      if ( show_dir == 2 )
      {

           if ( ch == 'k')      nexp_user_sel[pansel]--;
           else if ( ch == 'j')      nexp_user_sel[pansel]++;
           else if ( ch == 'g')      { nexp_user_sel[pansel]=1; nexp_user_scrolly[pansel] = 0; }
           else if ( ch == 'G')      { nexp_user_sel[pansel]=1; nexp_user_scrolly[pansel] = 0; }
           else if ( ch == 'u')      nexp_user_scrolly[pansel]-=4;
           else if ( ch == 'd')      nexp_user_scrolly[pansel]+=4;
           else if ( ch == 'n')      nexp_user_scrolly[pansel]+=4;

      // quick pan view embedded reader
      else if ( ch == 'p') 
      {
            readfilesp( nexp_user_fileselection , 0 , rows-4 );
            getchar();
      }

      // pager
      else if (  ch == 'r' )  
        nrunwith(  " less     ",  nexp_user_fileselection    ); 

      else if (  ch == 'e' ) {  enable_waiting_for_enter();  nsystem(  " lkmmc    " );   } //explorer
      else if (  ch == 't' )  nrunwith(  " lkview   ",  nexp_user_fileselection    ); 
      else if ( ch == 'v' )   
      {  enable_waiting_for_enter();  nrunwith(  " vim  ",  nexp_user_fileselection    );   }



      else if ( ch == '?' )   
      {  
         printf( "==========\n" );
         printf( "LFMMC\n" );
         printf( "==========\n" );
         printf( "<Press Key>\n" );
         disable_waiting_for_enter();   
         getchar();
      }

      else if ( ch == 5 )   //ctrl+e
      {
         printf( "lkmmc\n" );
         enable_waiting_for_enter();   
         nsystem(  " lkmmc  " );
     }

     else if ( ch == 5234 )  
     {
         printf( "this is  a test " );
         getchar();
         printf( "\33[2K" ); printf( "\r" );
         printf( "second == == = = = = = = = = = = = = = = = = = = == = = " );
         getchar();
         printf( "\33[2K" ); printf( "\r" );
         printf( "  third " );
         getchar();
     }


      else if ( ch == 'z' ) 
      {
         if ( show_title == 1 ) show_title = 0; else show_title = 1;
      }


      else if ( ch == '~')      
      {
            chdir( getenv( "HOME" ));
            nexp_user_sel[pansel]=1; nexp_user_scrolly[pansel] = 0; 
            strncpy( file_filter[pansel]  , "" , PATH_MAX );
      }

      else if ( ch == 'h')      
      {
            //chdir( pathpan[ pansel ] );
            chdir( ".." );
            nexp_user_sel[pansel]=1;  
            nexp_user_scrolly[pansel] = 0; 
            //strncpy( pathpan[ pansel ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
            //strncpy( file_filter[pansel]  , "" , PATH_MAX );
       }
       else if ( ch == 'l')      
       {
            // save 
            //chdir( pathpan[ pansel ] );
            //strncpy( pathclipboard , getcwd( string, PATH_MAX ), PATH_MAX );
            //selclipboard =      nexp_user_sel[pansel];
            //scrollyclipboard =  nexp_user_scrolly[pansel];
            // go 
            //chdir( pathpan[ pansel ] );
            chdir( nexp_user_fileselection );
            nexp_user_sel[pansel]=1; 
            nexp_user_scrolly[pansel] = 0; 
            //strncpy( pathpan[ pansel ] , getcwd( cwd, PATH_MAX ), PATH_MAX );
            //strncpy( file_filter[pansel]  , "" , PATH_MAX );
       }
      }
      
      else if ( show_dir == 0 )
      {
         if      ( ch == 'L' )   sx2++;
         else if ( ch == 'H' )   sx2--;
         else if ( ch == 'J' )   sy2++;
         else if ( ch == 'K' )   sy2--;
         else if ( ch == 'j' )   sy1++;
         else if ( ch == 'k' )   sy1--;
         else if ( ch == 'h' )   sx1--;
         else if ( ch == 'l' )   sx1++;
      }

      // Nothing before here
      /// here shall be if
      if ( ( ch == 's' ) || ( ch == 'w' ) )
      {
         if      ( show_dir == 0 ) show_dir = 1;
         else if ( show_dir == 1 ) show_dir = 2; 
         else if ( show_dir == 2 ) show_dir = 0; 
      }

      else if ( ch == 'i' )   show_dir = 2;

      else if ( ch == 'x' ) 
      {
         if ( show_path == 1 ) show_path = 0; else show_path = 1;
      }

      else if ( ch == 'c' ) 
      {
         if ( show_clock == 1 ) show_clock = 0; else show_clock = 1;
      }

      else if ( ch == '|' ) 
      {
         if ( show_frame == 1 ) show_frame = 0; else show_frame = 1;
      }

      else if ( ch == 'm' ) 
      {
            clrscr();
            home();
            printf( "  1: less \n" );
            printf( "  2: lfview \n" );
            printf( "  3: lkview \n" );
            printf( "  4: vim \n" );
            ch = getchar();
            if      ( ch == '1' )  
               nrunwith(  " less  ",  nexp_user_fileselection    );  
            else if ( ch == '2' )
               nrunwith(  " lfview  ",  nexp_user_fileselection    );  
            else if ( ch == '3' ) 
               nrunwith(  " lkview  ",  nexp_user_fileselection    );  
            else if ( ch == '4' )  
               nrunwith(  " vim  ",  nexp_user_fileselection    );  
            else if ( ch == 'v' )  
               nrunwith(  " vim  ",  nexp_user_fileselection    );  
            ch = 0;
      }


      else if ( ch == '$' ) 
      {
            strninput( "", "" );
            printf("got: \"%s\"\n", userstr );
            nsystem( userstr );
      }
      else if ( ch == '!' ) 
      {
            strninput( "", "" );
            printf("got: \"%s\"\n", userstr );
            nrunwith( userstr ,  nexp_user_fileselection    ); 
      }


      else if ( ch == 10 ) 
      {
            if ( strcmp( fextension( nexp_user_fileselection ) ,       "png" ) == 0 )
               nrunwith( " export DISPLAY=:0 ; feh  " , nexp_user_fileselection );
            else if ( strcmp( fextension( nexp_user_fileselection ) , "jpg" ) == 0 )
               nrunwith( " export DISPLAY=:0 ; feh  " , nexp_user_fileselection );

            else if ( strcmp( fextension( nexp_user_fileselection ) , "pdf" ) == 0 )
               nrunwith( "   export DISPLAY=:0 ; mupdf " , nexp_user_fileselection );

            else if ( strcmp( fextension( nexp_user_fileselection ) , "wmv" ) == 0 )
               nrunwith( " export DISPLAY=:0 ; mplayer  " , nexp_user_fileselection );
            else if ( strcmp( fextension( nexp_user_fileselection ) , "avi" ) == 0 )
               nrunwith( "   export DISPLAY=:0 ; mplayer " , nexp_user_fileselection );
            else if ( strcmp( fextension( nexp_user_fileselection ) , "mp4" ) == 0 )
               nrunwith( " export DISPLAY=:0 ; mplayer  " , nexp_user_fileselection );
            else if ( strcmp( fextension( nexp_user_fileselection ) , "ogg" ) == 0 )
               nrunwith( "   export DISPLAY=:0 ; mplayer " , nexp_user_fileselection );
      }

      else if ( ch == 'f' ) 
      {
            gotoyx( sy2 , 0 );  
            strninput( "", "" );
            printf("got: \"%s\"\n", userstr );
            strncpy( file_filter[1] ,  userstr, PATH_MAX );
            nexp_user_sel[pansel]=1; nexp_user_scrolly[pansel] = 0;
      }

      else if ( ch == 'y' )
      {
         //stringline( argv[ 1 ] , user_line_sel );
         //nrunwith(  " less     ",  nexp_user_fileselection    ); 
         // copy a line to clipboard
         strncpy( strpath , getcwd( cwd, PATH_MAX ), PATH_MAX );
         chdir( getenv( "HOME" ) );
         fptt = fopen( ".clipboard", "wb+" );
         fputs( nexp_user_fileselection , fptt );
         fputs( "\n" , fptt );
         fclose( fptt );
         chdir( strpath );
         printf( "Copying to clipboard the selection.\n" );
       }


      else if ( ch == ':' ) 
      {
            gotoyx( sy2 , 0 );  
            strninput( "", "" );
            printf("got: \"%s\"\n", userstr );
      }

      else if ( ch == 'a' ) 
      {
         if ( autorefresh == 1 ) autorefresh = 0; else 
         { 
            clrscr();
            home();
            printf( "  Autorefresh y/n?\n" );
            printf( "  2: 2 sec\n" );
            printf( "  6: 6 sec\n" );
            ch = getchar();
            if ( ch == '1' )       autorefresh = 1;
            else if ( ch == '2' )  autorefresh = 2;
            else if ( ch == '6' )  autorefresh = 6;
         }
         ch = 0;
      }

  }
  resetcolor();
  return 0;
}




