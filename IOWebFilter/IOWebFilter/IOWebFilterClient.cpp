//
//  IOWebFilterClient.cpp
//  IOWebFilter
//
//  Created by Axis on 7/27/15.
//  Copyright (c) 2015 Axis. All rights reserved.
//

#include "IOWebFilterClient.h"
#include "debugInfo.h"

#define super IOUserClient

OSDefineMetaClassAndStructors(IOWebFilterClientClass, super);

/*
 IOExternalMethodAction function;
 uint32_t		   checkScalarInputCount;
 uint32_t		   checkStructureInputSize;
 uint32_t		   checkScalarOutputCount;
 uint32_t		   checkStructureOutputSize;
 */
const IOExternalMethodDispatch dispatch_methods[kIOWebFilterDispatchMethodsMax] =
{
    [kIOWebFilterGetNetworkFlowData] = {
        .function = IOWebFilterClientClass::GetNetworkFlowData,
        .checkScalarInputCount=0,
        .checkStructureInputSize=0,
        .checkScalarOutputCount=1,
        .checkStructureOutputSize=0
    },

    [kIOWebFilterSetSharedMemoryStatus] = {
        .function = IOWebFilterClientClass::SetSharedMemStatus,
        .checkScalarInputCount=1,
        .checkStructureInputSize=0,
        .checkScalarOutputCount=0,
        .checkStructureOutputSize=0
    }
};

#pragma mark - dispatch func
IOReturn IOWebFilterClientClass::GetNetworkFlowData DISPATCH_PARAMETER
{
    arguments->scalarOutput[0]=IOWebFilterClass::GET_ATOMIC_INT64_PTR(CurrentNetworkData);
    return kIOReturnSuccess;
}

IOReturn IOWebFilterClientClass::SetSharedMemStatus DISPATCH_PARAMETER
{
    IOReturn err=kIOReturnUnsupported;
//    if(arguments->scalarInputCount>0)
//    {
//        if(this->_queue)
//        {
//            //this->_queue->SetSharedMemStatus(arguments->scalarInput[0]);
//        }
//    }
    return err;
}

#pragma mark - override func
IOReturn IOWebFilterClientClass::externalMethod( uint32_t                    selector,
                                                 IOExternalMethodArguments * arguments,
                                                 IOExternalMethodDispatch *  dispatch,
                                                 OSObject *                  target,
                                                 void *                      reference)
{
    if (selector < (uint32_t) kIOWebFilterDispatchMethodsMax)
    {
        dispatch = (IOExternalMethodDispatch *)&dispatch_methods[selector];

        if (!target)
            target = this;
    }
    
    return super::externalMethod(selector, arguments, dispatch, target, reference);
}

IOWebFilterClientClass* IOWebFilterClientClass::withTask(task_t owningTask, void * securityID)
{
    IOWebFilterClientClass *me=NULL;

    //check admin privilege
    IOReturn error_code = clientHasPrivilege(securityID, kIOClientPrivilegeAdministrator);
    if(error_code!=kIOReturnSuccess)
    {
        LOG(LOG_ERROR, "need admin privilege to connect ");
        return NULL;
    }

    me = new IOWebFilterClientClass;
    if(me)
    {
        if(!me->init())
        {
            me->release();
            return 0;
        }
        me->_task=owningTask;
        me->_status=0;
        me->_lock=IOLockAlloc();
    }
    return me;
}

bool IOWebFilterClientClass::start(IOService * provider)
{
    this->_owner = OSDynamicCast(IOWebFilterClass, provider);   //get owner
    assert(this->owner);

    //allocate new memory for further use
//    if(this->_queue==NULL)
//        this->_queue = IOSharedEventQueue::withCapacity(MAX_SHAREDMEM_SIZE);

    if(!super::start(this->_owner))
    {
        return false;
    }

    if(!this->_owner->IOService::open(this))
    {
        return false;
    }
    return true;
}

IOReturn IOWebFilterClientClass::registerNotificationPort(mach_port_t port, UInt32 type, io_user_reference_t refCon)
{
    LOG(LOG_ERROR, "in, port is %p", port);

    return this->_owner->setSharedQueueNotifyPort(port);
}

IOReturn IOWebFilterClientClass::clientMemoryForType(UInt32 type,
                                                     IOOptionBits * options,
                                                     IOMemoryDescriptor ** memory )
{
    if(this->_lock) IOLockLock(this->_lock);
    LOG(LOG_DEBUG, "in, we will gonging to share memory with userland");

    if(this->_status==1) IOLockUnlock(this->_lock);

    this->_status=1;

    IOReturn err=kIOReturnNoMemory;

    //if(this->_queue)
    {
        LOG(LOG_DEBUG, "in if");
        IOMemoryDescriptor *memToShared = this->_owner->getSharedQueueDescriptor();
        if(memToShared)
        {
            memToShared->retain();
            err=kIOReturnSuccess;
        }
        *options=0;
        *memory=memToShared;
    }
    LOG(LOG_DEBUG, "%d", err);
    if(this->_lock) IOLockUnlock(this->_lock);

    return err;
}

void IOWebFilterClientClass::free()
{
    if(this->_lock) IOLockLock(this->_lock);

    if(this->_owner)
    {
        this->_owner=NULL;
    }

    this->_status=0;

    if(this->_lock) IOLockUnlock(this->_lock);

    if(this->_lock) IOLockFree(this->_lock);
    this->_lock=NULL;

    super::free();
}

IOReturn IOWebFilterClientClass::clientClose(void)
{
    if(this->_owner)
    {
        this->_owner->IOService::close(this);
        detach(this->_owner);
    }
    return kIOReturnSuccess;
}

IOReturn IOWebFilterClientClass::clientDied( void )
{
    return this->clientClose();
}

//for IOConnectGetService()
IOService* IOWebFilterClientClass::getService( void )
{
    return this->_owner;
}