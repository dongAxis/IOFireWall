//
//  main.cpp
//  WebFilterClient
//
//  Created by Axis on 7/31/15.
//  Copyright (c) 2015 Axis. All rights reserved.
//

#include<iostream>
#include <boost/regex.hpp>
#include <boost/array.hpp>
#include "ClientConnectInterface.h"
#include "configure.h"
extern "C"{
#include <CoreFoundation/CoreFoundation.h>
}

int main()
{

//    std::string path("/Users/Axis/Documents/IOFireWall/WebFilterClient/WebFilterClient/config.xml");
//
//    ReadConfig<MtxLock> config(path);
//
//    int enable = config.getValue<int>("conf.enable_connect_kext");

    ClientConnectInterface interface;
    std::string str(IOWebFilterClassStr);

//    fflush(stdout);
    if(interface.connect_kext(str))
    {
        interface.start_get_data_thread();
        interface.start_handle_data_thread();
        interface.thread_get_data_join();           //dead recycle
        interface.disconnect_kext();
    }
    printf("after");
}

