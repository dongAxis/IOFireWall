//
//  ClientConnectInterface.h
//  WebFilterClient
//
//  Created by Axis on 7/31/15.
//  Copyright (c) 2015 Axis. All rights reserved.
//

#ifndef __WebFilterClient__ClientConnectInterface__
#define __WebFilterClient__ClientConnectInterface__

extern "C" {
#include <IOKit/IOKitLib.h>
#include <IOKit/IODataQueueClient.h>
#include <mach/mach.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFBase.h>
#include "common.h"
}

#include <boost/thread.hpp>
#include <boost/circular_buffer.hpp>

#include "LockGuard.h"

class ClientConnectInterface
{
public:
    static boost::circular_buffer<DataArgs> *circle_container;
    static pthread_cond_t full;
    static pthread_cond_t empty;
    static MtxLock container_mtx_lock;
    static IODataQueueMemory* map_memory;
    static mach_vm_size_t size;
    static mach_port_t port;
    static bool m_stop;
private:
    io_service_t io_service;
    io_connect_t io_connect;
    boost::thread *get_thread;
    boost::thread *work_thread;
    //todo: add lock there
private:
    static void pthread_get_data_callback(mach_port_t n);
    static void pthread_handle_data_callback(mach_port_t n);
    static boost::circular_buffer<DataArgs>* get_circle_buffer_instance();
public:
    ClientConnectInterface();
    ~ClientConnectInterface();
    void start_get_data_thread();
    void start_handle_data_thread();
    bool connect_kext(std::string& io_class);
    void disconnect_kext();
    void thread_get_data_join();
};

#endif /* defined(__WebFilterClient__ClientConnectInterface__) */
