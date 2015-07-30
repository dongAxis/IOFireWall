//
//  IOSharedEventQueue.h
//  IOWebFilter
//
//  Created by Axis on 7/27/15.
//  Copyright (c) 2015 Axis. All rights reserved.
//

#ifndef __IOWebFilter__IOSharedEventQueue__
#define __IOWebFilter__IOSharedEventQueue__

#include <IOKit/IOSharedDataQueue.h>
#include <IOKit/IOLocks.h>
#include <IOKit/IOMemoryDescriptor.h>

#include "private.h"
#include "common.h"

class IOSharedEventQueue : public IOSharedDataQueue
{
    OSDeclareDefaultStructors(IOSharedEventQueue);
public:
    //@override
    static IOSharedEventQueue *withCapacity(UInt32 size);
    //@override
    virtual IOMemoryDescriptor *getMemoryDescriptor();
    //@override
    virtual void setNotificationPort(mach_port_t port);
    //@override
    virtual void free();

    bool EnqueueTracker(DataArgs * data);
    OptionBits getStatus();
    void setStatus(OptionBits st);

private:
    IOLock *_lock;
    IOMemoryDescriptor *_descriptor;
    OptionBits _status;
};

#endif /* defined(__IOWebFilter__IOSharedEventQueue__) */
