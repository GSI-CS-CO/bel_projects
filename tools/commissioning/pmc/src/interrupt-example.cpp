/*
 * interrupt-example.cpp
 *
 *  Created on: Sep 9, 2014
 *      Author: bmarolt
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "etherbone.h"

using namespace std;
using namespace etherbone;

class intrHandler : public Handler{

    virtual status_t read (address_t address, width_t width, data_t* data){

        cout<<"intrHandler::read called! Printing parameters: \n";
        cout<<"address: "<<hex<<address<<"\n";
        cout<<"width: "<<width<<"\n";

        return EB_OK;
    };

    virtual status_t write(address_t address, width_t width, data_t  data){
        cout<<"intrHandler::write called! Printing parameters: \n";
        cout<<"address: "<<hex<<address<<"\n";
        cout<<"width: "<<width<<"\n";
        cout<<"data: "<<data<<"\n";
        return EB_OK;
    };
};


int main(int argc, char** argv){

    Socket              socket;
    Device              pexaria;
    status_t            status;
    struct sdb_device   info;
    eb_address_t        address;
    intrHandler         handler;
    const char          *master;
    const char          *slave;

    if(argc != 4){
        cout<<argv[0]<<": Expecting 3 parameter: <master> <slave> <sdb_address>\n";
        return EB_FAIL;
    }

    master = argv[1];
    slave = argv[2];

    //Address of sdb component triggering the interrupts
    address = strtoull(argv[3], NULL, 0);

    //Open socket with default parameter values: port = 0, width=0xff
    status = socket.open();
    //Listen for interrupts
    status = socket.passive(slave);
    if(status != EB_OK){
        cerr<<argv[0]<<": Socket::open() failed to open Etherbone socket: "<<eb_status(status);
        return EB_FAIL;
    }

    //Open device with default values of width=0xff, attempts = 5
    status = pexaria.open(socket, master);
    if(status != EB_OK){
        cerr<<argv[0]<<": Device::open() failed to open Etherbone device: "<<eb_status(status);
        return EB_FAIL;
    }

    status = pexaria.sdb_find_by_address(address, &info);
    if(status != EB_OK){
        cout<<argv[0]<<": No sdb component on address: "<<address<<".\n";
        return EB_ADDRESS;
    }

    status = socket.attach(&info, &handler);
    if(status != EB_OK){
        cout<<argv[0]<<": Failed to attach the interrupt handler to sokcet.\n";
        return EB_FAIL;
    }

    cout<<argv[0]<<": Waiting for interrupt!\n";
    while(true){
        socket.run();
    }

    return EB_OK;
}
