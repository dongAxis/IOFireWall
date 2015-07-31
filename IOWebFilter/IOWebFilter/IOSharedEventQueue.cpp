//
//  IOSharedEventQueue.cpp
//  IOWebFilter
//
//  Created by Axis on 7/27/15.
//  Copyright (c) 2015 Axis. All rights reserved.
//
#include <IOKit/IODataQueueShared.h>
#include <libkern/OSAtomic.h>

#include "IOSharedEventQueue.h"
#include "debugInfo.h"

#define super IOSharedDataQueue

OSDefineMetaClassAndStructors( IOSharedEventQueue, super );

IOSharedEventQueue* IOSharedEventQueue::withCapacity(UInt32 size)
{
    IOSharedEventQueue *data_source = new IOSharedEventQueue;

    if(data_source)
    {
        if(!data_source->initWithCapacity(size))
        {
            data_source->release();
            data_source=NULL;
        }
    }

    data_source->_descriptor=NULL;
    data_source->_lock = IOLockAlloc();
    data_source->_status=kSharedEventQueueNotifyWhenAddData;

    return data_source;
}

void IOSharedEventQueue::free()
{
    if(this->_lock)
    {
        IOLockFree(this->_lock);
        this->_lock=NULL;
    }

    if(this->_descriptor)
    {
        this->_descriptor->release();
        this->_descriptor=NULL;
    }
}

void IOSharedEventQueue::setNotificationPort(mach_port_t port)
{
    LOG(LOG_ERROR, "port is %d", port);
    super::setNotificationPort(port);
}

IOMemoryDescriptor* IOSharedEventQueue::getMemoryDescriptor()
{
    if(this->_descriptor==NULL)
    {
        this->_descriptor = super::getMemoryDescriptor();
    }
    return this->_descriptor;
}

bool IOSharedEventQueue::EnqueueTracker(DataArgs * data)
{
    uint32_t singleTrackerLen = sizeof(DataArgs);
    const UInt32 head = dataQueue->head;
    const UInt32 tail = dataQueue->tail;

    LOG(LOG_DEBUG, "head=%d", dataQueue->head);
    LOG(LOG_DEBUG, "tail=%d", dataQueue->tail);

    const UInt32 entrySize = singleTrackerLen+DATA_QUEUE_ENTRY_HEADER_SIZE;
    IODataQueueEntry *entry;

    if(singleTrackerLen>UINT32_MAX-DATA_QUEUE_ENTRY_HEADER_SIZE)
    {
        return false;
    }

    LOG(LOG_DEBUG, "this->getQueueSize()=%d", this->getQueueSize());
    if(this->getQueueSize()<tail)
    {
        return false;
    }

    if(tail>=head)
    {
        if(entrySize<=UINT32_MAX-DATA_QUEUE_ENTRY_HEADER_SIZE &&
        tail+entrySize<=this->getQueueSize())
        {
            entry = (IODataQueueEntry*)((uint8_t*)dataQueue->queue+dataQueue->tail);
            entry->size=singleTrackerLen;
            memcpy(entry->data, data, singleTrackerLen);
            OSAddAtomic(entrySize, (SInt32*)&(dataQueue->tail));
        }
        else if(head>singleTrackerLen)
        {
            dataQueue->queue->size = singleTrackerLen;

            if ( ( getQueueSize() - tail ) >= DATA_QUEUE_ENTRY_HEADER_SIZE )
            {
                ((IODataQueueEntry *)((UInt8 *)dataQueue->queue + tail))->size = entrySize;
            }

            memcpy(&dataQueue->queue->data, data, singleTrackerLen);
            OSCompareAndSwap(dataQueue->tail, entrySize, &dataQueue->tail);
        }
        else
        {
            return false;
        }
    }
    else
    {
        if ( (head - tail) > entrySize )
        {
            entry = (IODataQueueEntry *)((UInt8 *)dataQueue->queue + tail);

            entry->size = singleTrackerLen;
            memcpy(&entry->data, data, singleTrackerLen);
            OSAddAtomic(entrySize, (SInt32 *)&dataQueue->tail);
        }
        else
        {
            return false;    // queue is full
        }
    }

    //if ( (this->_status&kSharedEventQueueNotifyWhenAddData) || ( head == tail ) || ( dataQueue->head == tail ))
    {
        sendDataAvailableNotification();
    }

    return true;
}

OptionBits IOSharedEventQueue::getStatus()
{
    return this->_status;
}

void IOSharedEventQueue::setStatus(OptionBits st)
{
    OptionBits oldValue = this->_status;
    OSCompareAndSwap(oldValue, st, &(this->_status));
}

//void IOSharedEventQueue::start()
//{
//    if(this->_lock)
//    {
//        IOLockLock(this->_lock);
//    }
//
//    if(this->_magic & kSharedEventQueueStart) goto end;
//
//end:
//    if(this->_lock)
//    {
//        IOLockUnlock(this->_lock);
//    }
//}
//
//void IOSharedEventQueue::stop()
//{
//    if(this->_lock)
//    {
//        IOLockLock(this->_lock);
//    }
//
//    this->_magic &= ~kSharedEventQueueStart;
//
//    if(this->_lock)
//    {
//        IOLockUnlock(this->_lock);
//    }
//}
//
//void IOSharedEventQueue::enable()
//{
//    if(this->_lock) IOLockLock(this->_lock);
//
//    this->_magic &= ~kSharedEventQueueDisable;
//
//    if(this->_lock) IOLockUnlock(this->_lock);
//}
//
//void IOSharedEventQueue::disable()
//{
//    if(this->_lock) IOLockLock(this->_lock);
//
//    this->_magic |= kSharedEventQueueDisable;
//
//    if(this->_lock) IOLockUnlock(this->_lock);
//}