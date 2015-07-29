//
//  IOWebFilterClient.h
//  IOWebFilter
//
//  Created by Axis on 7/27/15.
//  Copyright (c) 2015 Axis. All rights reserved.
//

#ifndef __IOWebFilter__IOWebFilterClient__
#define __IOWebFilter__IOWebFilterClient__

#include <IOKit/IOUserClient.h>
#include <libkern/OSAtomic.h>

#include "common.h"
#include "IOWebFilter.h"
#include "private.h"
#include "IOSharedEventQueue.h"

class IOWebFilterClientClass : public IOUserClient
{
    OSDeclareDefaultStructors(IOWebFilterClientClass);
protected:
    task_t                  _task;
    IOWebFilterClass*       _owner;
    IOOptionBits            _status;
    IOLock*                 _lock;

    static  uint64_t test;
public:
    //@override
    virtual bool start(IOService * provider);
    //@override
    virtual void free();
    //@override
    virtual IOReturn clientClose( void );
    //@override
    virtual IOReturn clientDied( void );
    //@override
    virtual IOService * getService( void );
    //@override
    IOReturn externalMethod(uint32_t                    selector,
                            IOExternalMethodArguments * arguments,
                            IOExternalMethodDispatch *  dispatch,
                            OSObject *                  target,
                            void *                      reference);
    virtual IOReturn clientMemoryForType(UInt32 type,
                                         IOOptionBits * options,
                                         IOMemoryDescriptor ** memory );
    virtual IOReturn registerNotificationPort(
                                              mach_port_t port, UInt32 type, io_user_reference_t refCon);
public:
    static IOWebFilterClientClass* withTask(task_t owningTask, void* securityID);

#define DISPATCH_PARAMETER \
                (OSObject * target, void * reference,IOExternalMethodArguments * arguments)
    static IOReturn GetNetworkFlowData DISPATCH_PARAMETER;
    static IOReturn SetSharedMemStatus DISPATCH_PARAMETER;
};

#endif /* defined(__IOWebFilter__IOWebFilterClient__) */
