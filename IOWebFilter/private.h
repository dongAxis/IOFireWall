//
//  private.h
//  IOWebFilter
//
//  Created by Axis on 7/27/15.
//  Copyright (c) 2015 Axis. All rights reserved.
//

#ifndef IOWebFilter_private_h
#define IOWebFilter_private_h

#define kSocketTrackerInvalid 0x00000001
#define kSocketTrackerAttach  0x00000002
#define kSocketTrackerConnect 0x00000004
#define kSocketTrackerDataOut 0x00000008
#define kSocketTrackerDataIn  0x00000010
#define kSocketTrackerDetach  0x00000020

#define STATIC_GETANDSET_ATOMIC_INT64(scope, member)                    \
            static inline uint64_t get##member () {                     \
                return scope::member ;                                  \
            }                                                           \
            static inline void set##member (uint64_t m){                \
                uint64_t oldValue = scope::member ;                     \
                OSCompareAndSwap64(oldValue, m, &(scope::member));      \
            }
#define GET_ATOMIC_INT64_PTR(member) get##member()
#define SET_ATOMIC_INT64_PTR(member, new_value) set##member(new_value)

enum
{
    kIOWebFilterGetNetworkFlowData=0,
    kIOWebFilterSetSharedMemoryStatus,

    kIOWebFilterDispatchMethodsMax
};

typedef struct _socket_tracker
{
    long magic;
    struct
    {
        char ip[40];
        uint8_t family;
        uint16_t port;
    }destination, source;
    long data_len;
    pid_t pid;
    char proc_name[50+1];
    char request_meg[1024+1];
    IOLock *lock;
}SocketTracker;

#endif
