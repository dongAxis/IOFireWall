//
//  main.c
//  user_client
//
//  Created by Axis on 7/28/15.
//  Copyright (c) 2015 Axis. All rights reserved.
//

#include <stdio.h>
#include <IOKit/IOKitLib.h>
#include <mach/mach.h>
#include <pthread.h>
#include <CoreFoundation/CoreFoundation.h>

#include "common.h"

typedef struct
{
    mach_port_t data;
    mach_vm_size_t size;
}Data;

IODataQueueMemory *map;
int stop=0;

void *	perform(void *msg, CFIndex size, CFAllocatorRef allocator, void *info)
{
    IODataQueueEntry *entry=NULL;
    while((entry = IODataQueuePeek(map))!=NULL)
    {
        DataArgs * args= (DataArgs*)entry->data;

        uint32_t dataSize=0;
        IODataQueueDequeue(map, NULL, &dataSize);
        printf("head=%d, tail=%d, proc_name=%s\n", map->head, map->tail, args->proc_name);
        printf("size=%s\n", args->request_meg);
        fflush(stdout);
    }
    printf("head=%d, tail=%d\n", map->head, map->tail);

    return NULL;
}

mach_port_t signalMachPort=0;
static mach_port_t SignalHandlerGetPort(void *info)
{
    return signalMachPort;
}

void* handle(void *data __unused)
{
    Data* handle_data = (Data*)data;
    mach_port_t port =  handle_data->data;
    signalMachPort=port;
    printf("queue size %d", map->queueSize);
    fflush(stdout);

    CFRunLoopSourceContext1 context1;
    bzero(&context1, sizeof(CFRunLoopSourceContext1));
    context1.version=1;
    context1.perform = perform;
    context1.getPort=SignalHandlerGetPort;

    CFRunLoopSourceRef source = CFRunLoopSourceCreate(NULL, 0, (CFRunLoopSourceContext*)&context1);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopDefaultMode);

    while(!stop)
    {
        SInt32 status = CFRunLoopRunInMode(kCFRunLoopDefaultMode, 2, false);
        if(status==kCFRunLoopRunFinished || status == kCFRunLoopRunStopped) break;
    }
    printf("we qill quit\n");
//    for(;true;)
//    {
//        mach_msg_header_t msg;
//        bzero(&msg, sizeof(mach_msg_header_t));
//        mach_msg(&msg, MACH_RCV_MSG, 0, sizeof(msg), handle_data->data, 0, 0);
//        printf("%d, ", msg.msgh_local_port);
//        printf("%d\n", msg.msgh_remote_port);
//    }

    return NULL;
}

int main(int argc, const char * argv[]) {

    CFDictionaryRef rect;
    io_iterator_t iter;
    io_service_t service=0;
    kern_return_t kr;
    io_connect_t con;

    rect = IOServiceMatching(IOWebFilterClassStr);
    kr=IOServiceGetMatchingServices(kIOMasterPortDefault, rect, &iter);
    if(kr!=KERN_SUCCESS)
    {
        return kr;
    }

    if ((service=IOIteratorNext(iter))!=0)  //find first
    {
        CFStringRef classname;

        classname = IOObjectCopyClass(service);
        if(CFEqual(classname, CFSTR(IOWebFilterClassStr)))
        {
//           printf("%s", CFStringGetCStringPtr(classname, kCFStringEncodingUTF8));
            kern_return_t ret = IOServiceOpen(service, mach_task_self(), IOWebFilterClientClientTypeID, &con);
            IOObjectRelease(service);

            if(ret!=KERN_SUCCESS)
            {
                IOObjectRelease(iter);
                return KERN_FAILURE;
            }

            mach_port_t port = IODataQueueAllocateNotificationPort();
            printf("%d", port);
            fflush(stdout);
            ret = IOConnectSetNotificationPort(con, 0, port, 0);

            mach_vm_address_t address=0;
            mach_vm_size_t size=0;

//            mach_vm_size_t map_size;

            kern_return_t err =  IOConnectMapMemory(con, 0, mach_task_self(), &address, &size, kIOMapAnywhere);
            if(err==KERN_SUCCESS)
            {
                map = (IODataQueueMemory*)address;

                pthread_t pid;
                void *a;
                Data data;
                data.size=size;
                data.data=port;

                fflush(stdout);
                pthread_create(&pid, NULL, handle, &port);

                printf("push any key to continue....");
                getchar();

                stop=1;
                pthread_join(pid, &a);

                printf("ready for unmap");
//                IOConnectUnmapMemory(con, 0, mach_host_self(), address);
            }
            IOServiceClose(con);
        }

        IOObjectRelease(iter);
    }
    else
    {
        IOObjectRelease(iter);
        return KERN_FAILURE;
    }

    return KERN_SUCCESS;
}
