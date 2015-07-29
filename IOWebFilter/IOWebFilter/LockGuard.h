//
//  LockGuard.h
//  IOWebFilter
//
//  Created by Axis on 7/27/15.
//  Copyright (c) 2015 Axis. All rights reserved.
//

#ifndef IOWebFilter_LockGuard_h
#define IOWebFilter_LockGuard_h

#include <IOKit/IOLocks.h>

class LockGuard
{
private:
    IOLock& lock;
public:
    inline LockGuard(IOLock &lock):lock(lock)
    {
        IOLockLock(&(this->lock));
    }

    inline ~LockGuard()
    {
        IOUnlock(&(this->lock));
    }
};

#endif
