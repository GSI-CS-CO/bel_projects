#ifndef _DBG_H_
#define _DBG_H_

// Provides some control on how talkative your dbg statements get

//**************************//
   #define DEBUGLEVEL   0   //
//**************************//

//print macro for debuglevel 1-3
#ifdef DEBUGLEVEL
   #if DEBUGLEVEL>=1
      #define DBPRINT1 mprintf
      #define DBPRINT  mprintf
   #else
      #define DBPRINT1(...)
      #define DBPRINT(...)
   #endif

   #if DEBUGLEVEL>=2
      #define DBPRINT2 mprintf
   #else
      #define DBPRINT2(...)
   #endif

   #if DEBUGLEVEL>=3
      #define DBPRINT3 mprintf
   #else
      #define DBPRINT3(...)
   #endif
#endif

#endif
