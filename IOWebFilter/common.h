//
//  common.h
//  IOWebFilter
//
//  Created by Axis on 7/27/15.
//  Copyright (c) 2015 Axis. All rights reserved.
//

#ifndef IOWebFilter_common_h
#define IOWebFilter_common_h

#define TAG_NAME "com.axis.IOWebFilterFamily"
#define SF_NAME TAG_NAME
#define IPV4_SFLT_HANDLE 0x0F0F0F0F
//for class
#define IOWebFilterClass com_axis_IOWebFilterFamily
#define IOWebFilterClassStr "com_axis_IOWebFilterFamily"
#define IOWebFilterClientClass IOWebFilterClientClass
//for connect kext
#define IOWebFilterClientClientTypeID 0xff00ff00

//sharedMemory size
#define MAX_SHAREDMEM_SIZE 0x10000  //64K

#define IPV4_ADDR(addr) \
        ((unsigned char *)&addr)[0],     \
        ((unsigned char *)&addr)[1],     \
        ((unsigned char *)&addr)[2],     \
        ((unsigned char *)&addr)[3]

//SharedMemory property
enum
{
    kSharedEventQueueNotifyWhenAddData=0x01,

    //    kSharedEventQueueNotifyMax
};

#endif
