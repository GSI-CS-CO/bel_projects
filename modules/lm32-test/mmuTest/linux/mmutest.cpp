

#include <iostream>
#include <string>
#include <scu_env.hpp>
#include <scu_mmu_fe.hpp>

using namespace std;
using namespace Scu;

int main( int argc, char** ppArgv )
{
   cout << "MMU-Test" << endl;

   string scuUrl;
   if( isRunningOnScu() )
   {
       scuUrl = "dev/wbm0";
   }
   else
   {
      if( argc < 2 )
      {
         cerr << "ERROR: Missing SCU-url!" << endl;
         return 1;
      }
      scuUrl = ppArgv[1];
      if( scuUrl.find( "tcp/" ) == string::npos )
            scuUrl = "tcp/" + scuUrl;
   }

   cout << scuUrl << endl;

   mmuEb::EtherboneConnection ebc( scuUrl );

   Mmu mmu( &ebc );

   mmu::mmuIsPresent();

   return 0;
}

//================================== EOF ======================================