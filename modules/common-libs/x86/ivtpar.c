/********************************************************************************************
 *  ivtpar.c
 *
 *  created   : ~1988
 *  author    : Georg Bollen, Johannes Gutenberg-Universität Mainz
 *  maintainer: Dietrich Beck, GSI-Darmstadt
 *  version   : 11-December-2020
 * 
 *  history: 
 *  late 1980s: original version by Georg Bollen
 *  00-SEP-1990 last change by by Georg Bollen, 
 *  29-JUL-1998 Dietrich Beck, C and UNIX version
 *  07-AUG-1998 UNIX tested o.k. 
 *  22-OCT-1998 (crippled) MS-DOS Version
 *  03-DEC-1998 calls of GetChar2 changed slightly
 *  17-DEC-1998 comments in code, cleaned up, and one bug removed
 *  04-OCT-2020 reuse this retro code, simplified linux version only
 *  14-DEC-2020 GPL licensed
 *
 *  This software helps in creating simple text based user interfaces for programs. 
 *  See ivtpar.h for help.
 *
 * ------------------------------------------------------------------------------------------
 * 
 * License Agreement for this software:
 * 
 * Copyright (C) 2020  Dietrich Beck
 * GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 * Planckstrasse 1
 * D-64291 Darmstadt
 * Germany
 * 
 * Contact: d.beck@gsi.de
 *           
 * This program is free software: you can redistribute it and/or modify it under the terms of the 
 * GNU General Public License as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *                
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <http: /www.gnu.org/licenses/>.
 * 
 * For all questions and ideas contact: d.beck@gsi.de
 * Last update: 14-December-2020
 *********************************************************************************************/
#include <ivtpar.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <string.h>


#define TOPHEADER     "\033[7m I V T P A R  ------------------------------------------------ GB, DB 1990-2020 \033[0m"
// search escape sequences, https://stackoverflow.com/questions/51024909/how-to-move-cursor-back-in-c-console

// define special keys
#define KEY_UP       274
#define KEY_DOWN     275
#define KEY_RIGHT    276
#define KEY_LEFT     277
#define KEY_PAGEDOWN 278
#define KEY_PAGEUP   279
#define ESC_SEQ      500
#define ESC_1        501
#define ESC_2        502
#define ESC_3        503
#define ESC_4        504
#define ESC_5        505
#define ESC_6        506
#define ESC_7        507
#define ESC_8        508
#define ESC_9        509


// helper function: read character or special key combinations
int readKeyboard()
{
  unsigned char key;
  int c;
  int nChar;

  static struct termios oldt, newt;

  // get current terminal settings
  tcgetattr(STDIN_FILENO, &oldt);

  // set non canonical mode
  newt = oldt;
  //newt.c_lflag &= ~(ICANON);
  newt.c_lflag &= ~(ICANON | ECHO); 
  tcsetattr( STDIN_FILENO, TCSANOW, &newt);

  key   = -1;
   nChar = read(STDIN_FILENO, &key, 1 );

  if (key == 27) {        // 'ESC': escape sequence
    key   = -1;
    nChar = read(STDIN_FILENO, &key, 1 );
    switch (key) {
      case 91 :           // '[' control sequence
        key   = -1;
        nChar = read(STDIN_FILENO, &key, 1 );
        switch (key) {
          case 65 :       // 'arrow up'
            c = KEY_UP; 
            break;
          case 66 :       // 'arrow down'
            c = KEY_DOWN;
            break; 
          case 67 :       // 'arrow right'
            c = KEY_RIGHT;
            break;
          case 68 :       // 'arrow left'
            c = KEY_LEFT;
            break;
          case 53 :       // 'page up'
            // dummy read, probably '~', ASCII 126
            nChar = read(STDIN_FILENO, &key, 1 );
            c = KEY_PAGEUP;
            break;            
          case 54 :       // 'page down'
            // dummy read, probably '~', ASCII 126
            nChar = read(STDIN_FILENO, &key, 1 );
            c = KEY_PAGEDOWN;
            break;
          default :
            // dummy read, probably '~', ASCII 126
            nChar = read(STDIN_FILENO, &key, 1 );
            c = -1;
            break;
        } // switch key (in control sequence)
        break;
      case 49 :       // '1'
        c = ESC_1;
        break;
      case 50 :       // '2'
        c = ESC_2;
        break;
      case 51 :       // '3'
        c = ESC_3;
        break;
      case 52 :       // '4'
        c = ESC_4;
        break;
      case 53 :       // '5'
        c = ESC_5;
        break;
      case 54 :       // '6'
        c = ESC_6;
        break;
      case 55 :       // '7'
        c = ESC_7;
        break;
      case 56 :       // '8'
        c = ESC_8;
        break;
      case 57 :       // '9'
        c = ESC_9;
        break;
      default :
        c = -1;
    } // switch key (in escape sequence)
  } // if key == 27
  else {
    if (nChar) c = key;
    else       c = -1;
  }
  
  // reset to old terminal settings
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
  //printf("c %d\n", c);
  return c;
} // readChar


// helper function: clear screen
void clrscr()
{
  int i;

  for (i=0; i<48; i++) printf("\n");
} // clrscr



/* helper function, leftover from VAX late 1980s */
void vtpo(int line, int column)
{
  /* tried with C escape sequences, look into this maybe later
     printf("\033[0m\n"); //Esc[0m           
     printf("\033[%d;%df\n", line, column); 
  */ 
  // MoveCursor(line, column)
} /* vtpo */


/* helper function: error handling */
void error_handling(short* error, int* ichara, short del, short no_txtfile, 
                    short no_parfile, short par_mismatch, short chara, 
                    char txtnam[], char parnam[])
{
 vtpo(1,25);   
 *error=1;
 if(del)        printf(" >>>>> no characters to delete <<<<<");
 if(chara)      printf(" >>>>> too many characters <<<<<");
 if(no_txtfile) printf(" >>>>> unable to open textfile %s <<<<<",txtnam);
 if(no_parfile) printf(" >>>>> unable to open parameterfile %s <<<<<",parnam);
 if(par_mismatch) {
  printf(" >>>>> parameter mismatch between text and parameter file <<<<<");
  vtpo(2,1);
  printf("                                       \n");
  printf(" Delete parameter file and start again \n");
  printf("                                       \n");
 } /* if par_mismatch */
 
 if(no_txtfile || no_parfile || par_mismatch) {
  *ichara = readKeyboard();
 } /* if no_txtfile */
} /* error handling */


/* ivtpar menu */
int ivtpar(char txtnam[], char parnam[], int* l0, int lchange[IVTMAXPAR])
{
 int ivtpar_long;
 
 int    vtp[IVTMAXPAR][3];      /* position of menu parameters  */
        /* 1st index  : no. of param                            */
        /* 2nd index=0: no. of column, where input param begins */ 
	/* 2nd index=1: no. of column, where input param ends   */
	/* 2nd index=2: no. of line where input parameter sits  */
 char   vt[82][24];            	/* matrix for output	        */
 int    vt81[24];              	/* length of line in *.txt 	*/
 char   ch;			/* help variables		*/
 int    nch, nch1; 
 char   help;                   /* another help variable        */
 char   vtline[1000];           /* help line for file reading   */
 char   buff[100];
 //char   headerline[80];		/* displayed header line 	*/
 short  error;                  /* general error flag           */
 short  no_txtfile;             /* flag for missing txtfile     */
 short  no_parfile=1;           /* flag for missing parfile     */
 short  par_mismatch=1;         /* flag for parameter mismatch  */
 short  up,down,left,right;     /* flags for cursor keys        */
 short  dir;                    /* flag for 'a' cursor key      */
 short  del;                    /* flag for backspace key       */
 short  chara;                  /* flag for a character         */
 short  return_;                /* flag for return key          */
 short  lblank;                 /* flag for leading blank       */
 short  bruch;                  /* special flag, used only once */
 short  new;                    /* flag for plotting headerline */
 short  fin;                    /* flag for leaving routine     */
 int    k1,ichara,ica,i2,i1,nold,il, /* help variables          */
        line,i,ich,ic=0,ip=0,in,iloop;
 int    iofl=1;                 /* offset for line              */
 int    iofc=1;                 /* offset for column            */
 int    ipar;                   /* present parameter            */
 int    npar;                   /* total no. of menu parameters */
 int    maxpar;                 /* no. of input parameters      */
 FILE   *file;
 time_t time_date;              

 /*
  init return values
  */
 *l0 = 0;
 for (i=0;i<IVTMAXPAR;i++) lchange[i] = 0;

 vtpo(1,1);                             /* goto pos x=1,y=1  	*/
 printf("%s\n", TOPHEADER);             
 vtpo(24,1);
 printf(" next parameter = arrow keys | exit = <ret>                                 DB \n");
 time_date = time(0);
 strftime(buff,50,"%d-%b-%y %H:%M",localtime(&time_date));
 vtpo(24,55);
 printf("%s",buff);


 /* 
  read textfile
  */
 if ((file=fopen(txtnam,"r"))){

  no_txtfile=0;
  line=0;
  npar=0;
  maxpar=0;

  for(il=0;il<24;il++) {
   /*
     read one line, copy it to the vt vector and remember length of line
     using the vt81 vector
   */
   if (fgets(vtline,100,file)) nch = strlen(vtline) - 1;
   else break;
   if (nch > 80) { /* check for illegal length of line */
     printf("\n ivtpar: Illegal (>80) line length in txtfile \n");
     ch = readKeyboard();
     clrscr();
     return 0;
   }
     
   for (iloop=0;iloop<nch;iloop++) vt[iloop][il] = vtline[iloop];
   vt81[il]=nch;

   line++;
   ic=-1;/*check*/
   ip=0;/*check*/
   /*
     now reformat the line in case it containes junk or ivtpar specific
     command characters ($ or &). In case of '$' or '&' ivtpar assumes
     that a input parameter should be placed at this position. '$' means
     an input parameter, '&' means a menue (=command) parameter 
   */
   for(in=0;in<nch;in++) {
    ic++;
    vt[ic][il] = vt[in][il];        /* re-copy one character        */
    ch = vt[ic][il];                /* copy to help character       */

    if(ch <= 32) vt[ic][il]= ' ';   /* if junk, just assume a blank */
    
    if((ch == 36) || (ch == 38)) {  /* '$' or '&'                   */
      ic--;                         /* inhibit output of '$' or '&' */
      vt81[il]--;                   /* decrement length of line     */
      npar++;                       /* increment no. of parameters  */
      vtp[npar-1][0]=ic+1;          /* store position of parameter  */
      vtp[npar-1][1]=ic+1;
      vtp[npar-1][2]=il+1;
    } /* if ch */
    if (ch == 36) maxpar = npar;    /* update no. of input params    */

   } /* for in*/
  } /* for il */
  fclose(file);
 }
 else {printf("\n ivtpar: Error reading textfile\n");return 0;}

 /* 
   read the parameter file
 */ 

 if(maxpar > 0) { 

  par_mismatch = 0;
  no_parfile   = 1;

  if ((file = fopen (parnam,"r"))) {
   no_parfile = 0;
   for(ipar=0;ipar<maxpar;ipar++){
    /*
      init change flag and  read parameter line
    */
    lchange[ipar] = 0;                

    par_mismatch=1;
    if (fgets(vtline,100,file)) nch = strlen(vtline) - 1;
    else break;
    par_mismatch=0;

    /* 
      if there is a non zero parameter, get parameter position
      remove trailing and leading blanks, and copy the stuff to 
      the output matrix, then check for length of line and copy
      and update length of line
    */

    if(nch > 0) {
     ic=vtp[ipar][0];             
     il=vtp[ipar][2];

     nch1 = nch;
     for(ich=nch1-1;ich>=0;ich--){       /* 'remove' trailing blanks */
      ch=vtline[ich];
      if(ch >= 32) break;
      nch = nch - 1;
     } /* for ich */

     lblank=1;
     for(ich=0;ich<nch;ich++){      
      ch=vtline[ich];
      if(lblank && (ch <= 32)) lblank=0; /* 'remove' leading blanks  */
      else {                             /* copy to output matrix    */
       ic++;
       vtp[ipar][1]++;
       vt[ic-1][il-1]=ch;
      } /* else lblank */
     } /* for ich*/
    }/* if nch */

    if (ic >= 80) {/* check maximum length of output line            */
      printf("\n ivtpar: the length of a line (txtfile + parameter) \n");
      printf(  " exceeds the maximum (=80) line\n");
      ch = readKeyboard();
      clrscr();
      return 0;
    }

    nold=vt81[il-1];                /* update length of line         */
    if (ic > nold ) vt81[il-1]= ic;

   } /* for ipar */

  fclose(file);

  } /* if file */
  else {
   vtpo(28,1);
   printf("parameter file not found, I''ll create a new one\n");
   file = fopen(parnam,"w");
   no_parfile=0;
   fclose(file);
  } /* if file */
 }  /* if maxpar */

 /* 
  Output text+parameter
  */

 vtpo(2,1);
 for(il=0;il<line;il++){ 
  for(ic=0;ic<vt81[il];ic++) buff[ic] = vt[ic][il];
  if (vt81[il] > 0) buff[vt81[il]] = '\0';
  else sprintf(buff,"%s","");
  printf("%s\n",buff);
 } /* for il */                           

 ipar = 1;
 new  = 1;
 fin  = 0;
 return_ = 0;

 /*
   up to now, the text- and parameter file have been read and 
   displayed to the screen. Now ivtpar starts communicating
   with the user
 */
 
 while(!fin){
  if (!return_) {
   if(error) new=0;
   if (new) {  /* plot new headerline if just started or after error */
    vtpo(1,1);
    printf("%s\n",TOPHEADER);
    new = 0;
   } /* if new */
   if(error) new=1;
   if(error) error=0;

   ic=vtp[ipar-1][1];     /* get present position     */
   il=vtp[ipar-1][2];

   vtpo(il+iofl,ic+iofc); /* move to this position    */

   
  } /* if !return_ */

  /* simply dump whole stuff to screen */
  clrscr();
  printf("%s\n", TOPHEADER);
  for(il=0;il<line;il++){ 
    for(ic=0;ic<vt81[il];ic++) buff[ic] = vt[ic][il];
    if (il == vtp[ipar-1][2]-iofl) buff[vtp[ipar-1][1]-iofc+1] = '#';
    if (vt81[il] > 0) buff[vt81[il]] = '\0';
    else sprintf(buff,"%s","");
    printf("%s\n",buff);
  } /* for il */                           
  
  /* print bottom line */      
  printf("\033[7m move with arrow keys | exit with <RET> or <ESC> + <digit>");
  time_date = time(0);
  strftime(buff,50,"      %d-%b-%y %H:%M ",localtime(&time_date));
  printf("%s",buff);
  printf("\033[0m\n");

  ichara = readKeyboard();
   
  k1 = ichara;

  /* check for input of a normal character at an input parameter */
  chara = ((k1 < 127) && (k1 >= 32) && (ipar <= maxpar));

  /* check for backspace */
  del   = ((k1 == 127)||(k1==263)||(k1==8)); /* 263 for LINUX, 8 for WIN32 */

  /* check for cursor keys */
  up    = k1 == KEY_UP;
  down  = k1 == KEY_DOWN;
  right = k1 == KEY_RIGHT;
  left  = k1 == KEY_LEFT;   

  /* check for return key */
  return_  = k1 == 10;

  /* delword no longer implemented */

  /* set flag for exit */
  fin   = return_ && ((ipar > maxpar) || (npar == maxpar));

  // exit is also triggered by sequence <ESC> + <1..9> 
  if ((ESC_1 <= k1) && (k1 <= ESC_9)) {
    fin  = 1;
    ipar = maxpar + k1 - ESC_SEQ;
  } // if ESC_1

  // exit is also triggered by sequence <PAGE UP/DOWN> 
  if (k1 == KEY_PAGEUP) {
    fin  = 1;
    ipar = maxpar + 1;     // convention: <PAGE UP> -> 1st menu entry 
  } // if KEY_PAGEUP

  // exit is also triggered by sequence <PAGE UP/DOWN> 
  if (k1 == KEY_PAGEDOWN) {
    fin  = 1;
    ipar = maxpar + 2;     // convention: <PAGE DOWN> -> 2nd menu entry 
  } // if KEY_PAGEDOWN

  /* set flag for cursor movement (after cursor key pressed) */
  dir   = up || down || right || left;

  /* check for strange characters */
  if((ichara < 1) || (ichara > 277) || dir) {
   if(!dir) vtpo(il+iofl,ic+iofc);
  } /* end if */

  if(chara) {
   ica = vtp[ipar-1][0];                 /* get present position */
   ic  = vtp[ipar-1][1];
   il  = vtp[ipar-1][2];

   if((vt[ic][il-1] > 32) || (ic >= 79)) /* check for dimensions */
    error_handling(&error, &ichara, del, no_txtfile, no_parfile, par_mismatch, 
                   chara, txtnam, parnam);
   vtpo(il+iofl, ic+iofc);               /* move to position     */
   ic=ic+1;                              /* increment no. of char*/
   vtp[ipar-1][1] = ic;                  /* update present pos.  */
   vt[ic-1][il-1] = k1;                  /* copy char to matrix  */
   lchange[ipar-1] = 1;                  /* indicate change      */
   *l0 = 1;
   help = k1;                            /* print character      */
   printf("%c",help);
  } /* if char */
  
  if(del ) {
   ica = vtp[ipar-1][0];                 /* get present position */
   ic  = vtp[ipar-1][1];
   il  = vtp[ipar-1][2];
   
   if(ica >= ic) error_handling(&error, &ichara, del, no_txtfile, 
                   no_parfile, par_mismatch, chara, txtnam, parnam);
   else{
    vt[ic-1][il-1] = ' ';                /* copy blank to matrix */
    vtpo(il+iofl, ic+iofc -1);           /* move to new position */
    printf("%c",vt[ic-1][il-1]);         /* print the blank      */
    ic--;                                /* decrement no. of char*/
    vtp[ipar-1][1]=ic;                   /* update present pos.  */
    lchange[ipar-1] = 1;                 /* indicate change      */
    *l0 = 1;
   } /* else ica */
  } /* if del */

  if(dir ){
   ica = vtp[ipar-1][0];                 /* get present position */
   ic  = vtp[ipar-1][1];
   il  = vtp[ipar-1][2];

   if(right) ipar=ipar+1;                /* decide where to move */
   if(left) ipar=ipar-1;                 /* for left right keys  */
   if(ipar > npar) ipar=1;
   if(ipar < 1) ipar=npar;

   if(down) {                            /* determine first param*/
     for (i=0;i<npar;i++) {	         /* with a line number   */
       ip=vtp[i][2];                     /* higher then the line */
       ipar=i+1;                         /* of present param     */
       if(ip > il) break;
    } /* for i*/
    if (!(ip > il)) ipar = 1; /* if present param is in the last */
   } /* if down */            /* line then goto the first param  */

   if(up) {
     bruch = 0;               /* needed to escape the 2nd loop   */
     for (i=0;i<npar;i++){    
      ipar--;                 /* go one parameter back           */
      if(ipar < 1) ipar=npar; /* if < first param, goto last par */
      ip=vtp[ipar-1][2];      /* get line of 'one parameter back'*/      
      if(ip != il) {          /* if this line != present line... */
	for(ipar=1;ipar<=npar;ipar++){ /*get no. of 1st par in   */
        if(vtp[ipar-1][2] == ip) {bruch = 1; break;} /*this line */
       } /* for ipar */
      } /* if ip */
      if(bruch) break;        /* quit, if new line determined    */
     } /* for i */
   }   /* if up */
  } /* if dir */

  if(return_){               
    /* 
     if present parameter is already a command (and no input 
     parameter) the set flag to exit the routine, else set new
     position to the first command parameter and move there
    */
   if(ipar > maxpar) fin = 1;
   else {
    ipar = maxpar + 1;
    ic=vtp[ipar-1][1];
    il=vtp[ipar-1][2];
    vtpo(il+iofl, ic+iofc);
   } /* if ipar */
  } /* if return_*/
 } /* while not fin*/

 /* determine command of exit */
 ivtpar_long = ipar - maxpar;       
 if(ivtpar_long <= 0) ivtpar_long=-ipar;
 
 /* 
  write the parameters back to file. First, get start and end
  position of the parameters in the matrix, then copy the 
  parameters to a help buffer, then add a linebreak, and finally
  add a '\0' to indicate the end of the string. Secondly, write
  the help buffer to the file.
 */ 
 if ((file = fopen(parnam,"w+"))) {
  for (ipar=0;ipar<maxpar;ipar++){
    i1=vtp[ipar][0]+1; 
    i2=vtp[ipar][1];   
    il=vtp[ipar][2];
    for(iloop=i1;iloop<=i2;iloop++) buff[iloop-i1] = vt[iloop-1][il-1];
    buff[i2-i1+1] = '\n';
    buff[i2-i1+2] = '\0';
    fwrite(buff,sizeof(char)*strlen(buff),1,file);
  } /* for ipar */
  fclose(file);
 } /* if file */


 clrscr();                     /* clean up the screen           */
 printf("\n");                 /* go to beginning of a new line */
 return ivtpar_long;           /* return command and exit       */
} /* ivtpar */

