//
//  LockGuard.h
//  WebFilterClient
//
//  Created by Axis on 8/5/15.
//  Copyright (c) 2015 Axis. All rights reserved.
//

#ifndef WebFilterClient_LockGuard_h
#define WebFilterClient_LockGuard_h

#include <pthread.h>

template<typename T>
class LockGuard
{
public:
    LockGuard(T& l):lock(l)
    {
        if(this->lock.lock())
        {
            printf("lock failed");
        }
    }

//    LockGuard(T& l, bool isRead):lock(l)
//    {
//        if(isRead)
//        {
//            this->lock.
//        }
//    }

    ~LockGuard()
    {
        this->lock.unlock();
    }

private:
    LockGuard(LockGuard& l);
    LockGuard& operator=(const LockGuard& r);
private:
    T& lock;
};

class LockTemplate
{
public:
    virtual bool lock()=0;
    virtual void unlock()=0;
};

class EmptyLock : public LockTemplate
{
public:
    virtual bool lock(){return 1;}
    virtual void unlock(){}
};

class MtxLock : public LockTemplate
{
public:
    MtxLock()
    {
        pthread_mutex_init(&this->mtx_lock, NULL);
    }

    virtual bool lock()
    {
        return pthread_mutex_lock(&this->mtx_lock);
    }

    virtual void unlock()
    {
        pthread_mutex_unlock(&this->mtx_lock);
    }

    ~MtxLock()
    {
        pthread_mutex_destroy(&this->mtx_lock);
    }
private:
    MtxLock(MtxLock& r);
    MtxLock& operator=(const MtxLock& r);
private:
    pthread_mutex_t mtx_lock;
};

//class RWLock : public LockTemplate
//{
//public:
//    RWLock()
//    {
//        pthread_rwlock_init(&this->rw_lock, NULL);
//    }
//
//    int rd_lock()
//    {
//        return pthread_rwlock_rdlock(&this->rw_lock);
//    }
//
//
//
//    ~RWLock()
//    {
//        pthread_rwlock_destroy(&this->rw_lock);
//    }
//
//private:
//    pthread_rwlock_t rw_lock;
//};

#endif
