#include <iostream>
#include <scu_shared_mem.h>

using namespace std;

int main( int argc, char** ppArgv )
{
   cout << "sizeof( SCU_SHARED_DATA_T ): " << sizeof(SCU_SHARED_DATA_T) << endl;
   cout << "offsetof( SCU_SHARED_DATA_T, daq_buf ): "
        << offsetof( SCU_SHARED_DATA_T, daq_buf ) << endl;
   cout << "sizeof( struct daq_buffer ): "
        << sizeof( struct daq_buffer ) << endl;
   return 0;
}
