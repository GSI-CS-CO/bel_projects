/*     late  1980s oiginal version by Georg Bollen
c      00-SEP-1990 last change by by Georg Bollen, 
c      29-JUL-1998 Dietrich Beck, C and UNIX version
c      07-AUG-1998 UNIX tested o.k. 
c      22-OCT-1998 (crippled) MS-DOS Version
c      03-DEC-1998 calls of GetChar2 changed slightly
c      17-DEC-1998 comments in code, cleaned up, and one bug removed
c      04-OCT-2020 reuse this retro code, linux version only
c
c
c 	IVTPAR ist eine INTEGER FUNCTION  fuer die vereinfachte Eingabe von
c	Parametern in ein Programm, wobei die Kommentierung der
c	Parameter sehr erleichtert wird.
c
c	Aufruf:
c
c	  i = ivtpar (txtnam, parnam,&l0,lchange)
c
c	  txtnam:	Name eines Textfiles
c	  parnam:	Name eines zugehoerigen Parameterfiles
c         l0    :       1, falls irgendein parameter geaendert, sonst 0
c         lchange[i]:   1, wenn Parameter i geaendert wurde      
c
c	  z.B. 
c
c	  i = ivtpar ("menue.txt", "menue.par", &l0, lchange)
c
c	Zur Kommentierung erzeugt man einen 
c	Textfile <txtnam> mit einer maximalen Laenge von 23 Zeilen. 
c	An die Stellen des Textbildes an denen die Parameter eingegeben
c	werden sollen schreibt man ein  $.
c	Nach dem letzten Parameter hat man die Moeglichkeit
c	mit dem & Zeichen verschiedene Ausgaenge ins Leben zu rufen
c
c	Beispiel fuer Textfile:
c
c
c		parameter 1: $       parameter 2: $
c		parameter 3: $       KOMMENTAR.... etc
c
c		[&EXIT]   [&GOTO MENUE2]   [&GOTO MENUE3]
c
c
c
c	IVTPAR liest, nachdem es den Textfile auf den Bildschirm
c	geschrieben hat ( $ und & werden unterdrueckt)
c	von einem Paramterfile <parfil> die
c	Parameter und setzt sie in dem Text anstelle der $ ein.
c	Nun kann man mit den Cursortasten zu den verschiedenen Parametern
c 	springen und diese aendern.
c	Mit <ret> springt man zu den Ausgangsmarken oder man verlaesst
c	IVTPAR falls es keine Marken gibt oder wenn man bereits auf 
c	einer sitzt.
c	Beim Verlassen schreibt IVTPAR die Parameter in den Parameterfile 
c	und erhaelt folgenden Wert zugewiesen: 
c
c	  IVTPAR = +1,+2,+3..+N. 	wenn man IVTPAR ueber die N-te 
c					Ausgangsmarke verlaesst
c
c	  IVTPAR = -1,-2,-3..-M		wenn keine Ausgangsmarken 
c					existieren und man IVTPAR 
c					vom M-ten Parameter aus verlaesst
c	Maximalwerte:
c
c	  Zahl der Parameter + Zahl der Marken <= 100
c
c
c	Mit den logischen Variablen kann man herausfinden ob und welcher 
c	parameter geaendert wurde
c
c	  l0	= .true.		wenn irgendein parameter geaendert
c					wurde
c	  lchange[i-1] = .true.		wenn der i-te Parameter geaendert
c					wurde
c
c	Hierbei ist eine Aenderung definiert als jeder andere Tastendruck
c	als die Cursertasten und <crt>
c
c--------------------------------------------------------------------------*/
#ifndef _IVTPAR
#define _IVTPAR

#define IVTMAXPAR 100


/*#include "cfortran.h"*/


int ivtpar(
           char txtnam[],          /* name of textfile */
	   char parnam[],          /* name of parfile  */
	   int* l0,                /* true if changed  */
	   int lchange[IVTMAXPAR]  /* change of par    */
          );

/* make ivtpar callable by fortran routines */
/*FCALLSCFUN4(INT,ivtpar,CFIVTPAR,cfivtpar,STRING,STRING,PINT,INTV)*/

#endif
