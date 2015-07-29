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

#include "common.h"

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
           ret = IOConnectSetNotificationPort(con, 0, port, NULL);

//            mach_msg_header_t msg;
//
//            bzero(&msg, sizeof(mach_msg_header_t));
//            msg.msgh_id=0;
//            msg.msgh_bits=0;
//            msg.msgh_remote_port=MACH_PORT_NULL;
//            msg.msgh_local_port=port;
//            msg.msgh_size = sizeof(mach_msg_header_t);
//            mach_msg(&msg, MACH_RCV_MSG, 0, sizeof(msg), port, 0, 0);

           printf("receive msg, ret=%d", ret);

            mach_vm_address_t address;
            mach_vm_size_t size;

            IODataQueueMemory *map;
            mach_vm_size_t map_size;

            kern_return_t err =  IOConnectMapMemory(con, 0, mach_task_self(), &address, &size, kIOMapAnywhere);
            if(err==KERN_SUCCESS)
            {
                map = (IODataQueueMemory*)address;
                map_size = size;

//                mach_msg_header_t msg;
//    
//                bzero(&msg, sizeof(mach_msg_header_t));
//                msg.msgh_id=0;
//                msg.msgh_bits=0;
//                msg.msgh_remote_port=MACH_PORT_NULL;
//                msg.msgh_local_port=port;
//                msg.msgh_size = sizeof(mach_msg_header_t);
//                mach_msg(&msg, MACH_RCV_MSG, 0, sizeof(msg), port, 0, 0);

                IOConnectUnmapMemory(con, 0, mach_task_self(), address);
            }
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
