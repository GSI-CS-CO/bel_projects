/*
 * multiple-read.cpp
 *
 *  Created on: Sep 8, 2014
 *      Author: bmarolt
 */

#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "etherbone.h"

/**
 * DETERMINED EMPIRICALLY!!
 * 7060 B is the largest data block that can be read with 1 call to socket.run
 */
#define MAX_BYTES_PER_RUN 7060

using namespace std;
using namespace etherbone;

//Read callback uses this to report the cycle with index cycleNumber has completed
static int cycleNumber = 0;

//Structure holding the statistical data - read latency and size of data being read (in bytes)
typedef struct transferData{
    time_t seconds;
    long   nSeconds;
    int    dataTransfered;
}transferData;

/**
 * Extending the Device class so the protected member 'device' can be accessed in
 * order to read the bus width.
 */
class Pexaria: public Device
{
public:
    eb_width_t  busWidth();
};

/**
 * Used to read the device bus width. If it does not match the size of the single
 * read operation (requested by the user as a parameter to the program), a warning
 * is issued.
 */
eb_width_t Pexaria::busWidth(){
    return eb_device_width(device)&0x0f;
}

/**
 * Called when the cycle is completed. Used to report the index of the cycle.
 */
static void cycleCallback(eb_user_data_t user, eb_device_t dev, eb_operation_t op, eb_status_t status) {
    cycleNumber++;
    cout<<"Cycle number "<<cycleNumber<<" finished\n";
}

/**
 * Calculates the read latency and stores it in the transferData structure
 */
static void getTime(transferData *statistics, struct timespec startTime, struct timespec stopTime){
    statistics->seconds = stopTime.tv_sec - startTime.tv_sec;
    statistics->nSeconds = stopTime.tv_nsec - startTime.tv_nsec;

    if(statistics->nSeconds < 0){
        statistics->seconds--;
        statistics->nSeconds = 1000000000 - statistics->nSeconds;
    }

}

int main(int argc, char** argv){

    Socket              socket;
    Pexaria             pexaria;
    Cycle               cycle;
    status_t            status;
    address_t           address;
    data_t              blockSize;
    data_t              *data;
    data_t              size;
    data_t              nRuns;
    format_t            format;
    format_t            endian;
    format_t            busWidth;
    struct sdb_device   sdbComponent;
    const char          *deviceAddress;
    const char          *program;
    int                 callbackData;
    struct timespec     startTime;
    struct timespec     stopTime;
    transferData        *statistics;

    program = argv[0];

    if(argc != 5){
        cout<<program<<": Exepcting 4 arguments: <host> <address> <blockSize> <size>\n";
        return EB_FAIL;
    }

    deviceAddress = argv[1];
    address = strtoull(argv[2], NULL, 0);
    blockSize = strtoull(argv[3], NULL, 0);
    size = strtoull(argv[4], NULL, 0);

    if(size != 1 && size != 2 && size != 4){
        cout<<program<<": size must be 1, 2 or 4\n";
        return EB_FAIL;
    }

    data = new data_t [blockSize];

    //Open socket with default parameter values: port = 0, width=0xff
    status = socket.open();
    if(status != EB_OK){
        cerr<<program<<": Socket::open() failed to open Etherbone socket: "<<eb_status(status);
        return EB_FAIL;
    }

    //Open device with default values of width=0xff, attempts = 5
    status = pexaria.open(socket, deviceAddress);
    if(status != EB_OK){
        cerr<<program<<": Device::open() failed to open Etherbone device: "<<eb_status(status);
        return EB_FAIL;
    }

    busWidth = pexaria.busWidth();
    if((busWidth & size) != size){
        cout<<program<<": Selected data size: "<<size<<" does not match the device bus width: "
                <<(int)busWidth<<". Some data might not be read!\n";
    }

    if ((status = pexaria.sdb_find_by_address(address, &sdbComponent)) != EB_OK) {
      cout<<program<<": failed to find SDB component for address: "<<address<<"\n";
      return EB_ADDRESS;
    }

    if ((sdbComponent.bus_specific & SDB_WISHBONE_LITTLE_ENDIAN) != 0){
      endian = EB_LITTLE_ENDIAN;
    }
    else{
      endian = EB_BIG_ENDIAN;
    }

    format = endian | size;

    nRuns = blockSize/MAX_BYTES_PER_RUN;
    if(blockSize % MAX_BYTES_PER_RUN !=0){
        nRuns++;
    }

    statistics = new transferData [nRuns];

    for(data_t i = 0, k = 0; i < nRuns; i++){
        cycle.open(pexaria, &callbackData, &cycleCallback);

        for(data_t j = 0; (j < MAX_BYTES_PER_RUN/size) && (k != blockSize/size); j++){
            cycle.read(address+size*k, format, &data[k]);
            k++;
        }

        cycle.close();

        clock_gettime(CLOCK_MONOTONIC, &startTime);
        socket.run();
        clock_gettime(CLOCK_MONOTONIC, &stopTime);

        getTime(statistics+i, startTime, stopTime);

        //data transfered during the last call to socket.run
        statistics[i].dataTransfered = k*size - i*MAX_BYTES_PER_RUN;
    }

    status = pexaria.close();
    status = socket.close();

    for(data_t i = 0; i < blockSize/size; i++){
        cout<<"data["<<i<<"] = "<<data[i]<<"\n";
    }

    for(data_t i = 0; i < nRuns; i++){
        cout<<"seconds: "<< statistics[i].seconds <<", nseconds: "<< statistics[i].nSeconds <<
                ", data transfered: "<<statistics[i].dataTransfered<<"\n";
    }

    return EB_OK;
}

