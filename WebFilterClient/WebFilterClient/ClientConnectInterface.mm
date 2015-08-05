//
//  ClientConnectInterface.cpp
//  WebFilterClient
//
//  Created by Axis on 7/31/15.
//  Copyright (c) 2015 Axis. All rights reserved.
//
#include <boost/thread.hpp>

extern "C" {
#include <IOKit/IOKitLib.h>
#include <IOKit/IODataQueueClient.h>
#include <mach/mach.h>
#include <pthread/pthread.h>
}

#include "configure.h"
#include "common.h"
#include "ClientConnectInterface.h"

//void f()
//{
//    boost::thread t1;
//}

boost::circular_buffer<DataArgs>* ClientConnectInterface::circle_container=NULL;
MtxLock ClientConnectInterface::container_mtx_lock;
IODataQueueMemory* ClientConnectInterface::map_memory=NULL;
mach_vm_size_t ClientConnectInterface::size;
bool ClientConnectInterface::m_stop=0;
mach_port_t ClientConnectInterface::port=0;
pthread_cond_t ClientConnectInterface::full;
pthread_cond_t ClientConnectInterface::empty;

ClientConnectInterface::ClientConnectInterface()
{
    this->io_service=NULL;
    this->io_connect=NULL;
    this->get_thread=NULL;
    this->work_thread=NULL;
    port = MACH_PORT_NULL;
//    this->map_memory=NULL;

//    this->circle_container=new boost::circular_buffer<DataArgs>(100);
    circle_container = get_circle_buffer_instance();
}

ClientConnectInterface::~ClientConnectInterface()
{
    {
//        LockGuard<MtxLock> guard(this->container_mtx_lock);
//        if(this->circle_container)
//        {
//            delete this->circle_container;
//            this->circle_container=NULL;
//        }
    }
}

boost::circular_buffer<DataArgs>* ClientConnectInterface::get_circle_buffer_instance()
{
    LockGuard<MtxLock> guard(ClientConnectInterface::container_mtx_lock);
    if(circle_container==NULL)
    {
        boost::circular_buffer<DataArgs> *buf = new boost::circular_buffer<DataArgs>(100);
        buf->clear();
        pthread_cond_init(&ClientConnectInterface::full, NULL);
        pthread_cond_init(&ClientConnectInterface::empty, NULL);
        return buf;
    }

    return circle_container;
}

void ClientConnectInterface::pthread_get_data_callback(mach_port_t n)
{

    while(1)
    {
        LockGuard<MtxLock> guard(ClientConnectInterface::container_mtx_lock);

        if(ClientConnectInterface::circle_container->empty())
        {
            continue;
//            pthread_mutex_t mtx;
//            pthread_mutex_init(&mtx, NULL);
//
//            pthread_mutex_lock(&mtx);
//            pthread_cond_wait(&ClientConnectInterface::empty, &mtx);
//            pthread_mutex_unlock(&mtx);
//            pthread_mutex_destroy(&mtx);
        }

        DataArgs& args = ClientConnectInterface::circle_container->front();
        ClientConnectInterface::circle_container->pop_front();
        delete &args;   //free memory
//        printf("%s", args.request_meg);
//        pthread_cond_signal(&ClientConnectInterface::full);
    }
}

void* perform(void *msg, CFIndex size, CFAllocatorRef allocator, void *info)
{
    IODataQueueEntry *entry=NULL;
    while((entry = IODataQueuePeek(ClientConnectInterface::map_memory))!=NULL)
    {
        DataArgs * args= (DataArgs*)entry->data;

        uint32_t dataSize=0;
        IODataQueueDequeue(ClientConnectInterface::map_memory, NULL, &dataSize);
        printf("size=%s\n", args->request_meg);
//        fflush(stdout);

        {
            LockGuard<MtxLock> guard(ClientConnectInterface::container_mtx_lock);

//            if(ClientConnectInterface::circle_container->full())
//            {
//                continue;
//                pthread_mutex_t mtx;
//                pthread_mutex_init(&mtx, NULL);
//
//                pthread_mutex_lock(&mtx);
//                pthread_cond_wait(&ClientConnectInterface::full, &mtx);
//                pthread_mutex_unlock(&mtx);
//                pthread_mutex_destroy(&mtx);
//            }
            DataArgs *data = new DataArgs();
            bzero(data, sizeof(DataArgs));
            memcpy(data, args, sizeof(DataArgs));
            ClientConnectInterface::circle_container->push_back(*data);
//            pthread_cond_signal(&ClientConnectInterface::empty);
        }
    }

    return NULL;
}

static mach_port_t SignalHandlerGetPort(void *info)
{
    return ClientConnectInterface::port;
}


void ClientConnectInterface::pthread_handle_data_callback(mach_port_t n)
{
    CFRunLoopSourceContext1 context1;
    bzero(&context1, sizeof(CFRunLoopSourceContext1));

    context1.version=1;
    context1.perform=perform;
    context1.getPort=SignalHandlerGetPort;

    CFRunLoopSourceRef source = CFRunLoopSourceCreate(NULL, 0, (CFRunLoopSourceContext*)&context1);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopDefaultMode);

    while(!ClientConnectInterface::m_stop)
    {
        UInt32 status = CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1, false);
        if(status==kCFRunLoopRunFinished ||
           status==kCFRunLoopRunStopped) break;
    }
}

//void ClientConnectInterface::pthread_get_data_callback(int n)
//{
//    while(1){
//        printf("test it");
//    }
//}

void ClientConnectInterface::thread_get_data_join()
{
    while(1){}
}

void ClientConnectInterface::start_get_data_thread()
{
    if(this->get_thread==NULL)
    {
        this->get_thread = new boost::thread(ClientConnectInterface::pthread_get_data_callback, port);
    }
}

void ClientConnectInterface::start_handle_data_thread()
{
    if(this->work_thread==NULL)
    {
        this->work_thread = new boost::thread(ClientConnectInterface::pthread_handle_data_callback, port);
    }
}

bool ClientConnectInterface::connect_kext(std::string& io_class)
{
    printf("in connect_kext");
    fflush(stdout);
    CFDictionaryRef service_dict;
    kern_return_t error_code;
    io_iterator_t service_iterator;
    bool err_code=true;
    mach_vm_address_t start_addr;
    mach_vm_size_t size;

    do
    {
        printf("%s\n", io_class.c_str());
        service_dict = IOServiceMatching(io_class.c_str());
        error_code = IOServiceGetMatchingServices(kIOMasterPortDefault, service_dict, &service_iterator);
        if(error_code!=KERN_SUCCESS)
        {
            err_code = false;
            break;
        }

        printf("ready to find io_service\n");
        fflush(stdout);
        if((this->io_service=IOIteratorNext(service_iterator))==0)
        {
            printf("failed to find io_service\n");
            fflush(stdout);
            err_code=false;
            break;
        }

        printf("going to start ioservice\n");
        error_code = IOServiceOpen(this->io_service, mach_task_self(), IOWebFilterClientClientTypeID, &this->io_connect);
        IOObjectRelease(this->io_service);
        if(error_code!=KERN_SUCCESS)
        {
            printf("but failed, error code is %x", error_code);
            err_code=false;
            IOObjectRelease(service_iterator);
            break;
        }
        printf("success to open service\n");

        if(port==MACH_PORT_NULL)
        {
            port = IODataQueueAllocateNotificationPort();
        }

        IOConnectSetNotificationPort(this->io_connect, 0, port, 0);

        //map memory
        error_code = IOConnectMapMemory(this->io_connect, 0, mach_task_self(), &start_addr, &size, kIOMapAnywhere);
        if(error_code!=KERN_SUCCESS)
        {
            printf("fail to map memory");
            ClientConnectInterface::map_memory=NULL;
            err_code=false;
            break;
        }

        printf("the map successfully");
        ClientConnectInterface::map_memory = (IODataQueueMemory*)start_addr;
        ClientConnectInterface::size = size;

    }while(false);

//    SAFE_RELEASE(service_dict);
    if(service_iterator) IOObjectRelease(service_iterator);

    return err_code;
}

void ClientConnectInterface::disconnect_kext()
{
    if(ClientConnectInterface::map_memory!=NULL)
    {
        IOConnectUnmapMemory(this->io_connect, 0, mach_task_self(), (mach_vm_address_t)ClientConnectInterface::map_memory);
    }

    IOServiceClose(this->io_connect);
}