/* add your code here */
#include <netinet/in.h>
#include <IOKit/IOLocks.h>
#include <sys/proc.h>
#include <sys/kpi_mbuf.h>
#include <libkern/OSAtomic.h>

#include "IOWebFilter.h"
#include "IOWebFilterClient.h"
#include "debugInfo.h"
#include "LockGuard.h"
#include "private.h"

#define super IOService

OSDefineMetaClassAndStructors(IOWebFilterClass, super)
volatile uint64_t IOWebFilterClass::CurrentNetworkData=0;
IOSharedEventQueue* IOWebFilterClass::_queue=NULL;

#pragma mark - static data
struct sflt_filter IOWebFilterClass::ipv4_filter =
{
    .sf_handle = IPV4_SFLT_HANDLE,
    .sf_flags=SFLT_GLOBAL,
    .sf_name = (char*)SF_NAME,

    .sf_unregistered = IOWebFilterClass::tl_unregistered_func,
    .sf_attach = IOWebFilterClass::tl_attach_func,
    .sf_detach = IOWebFilterClass::tl_detach_func,

    .sf_notify = NULL,
    .sf_getpeername = NULL,
    .sf_getsockname = NULL,
    .sf_data_in = IOWebFilterClass::tl_data_in_func,
    .sf_data_out = IOWebFilterClass::tl_data_out_func,
    .sf_connect_in = NULL,
    .sf_connect_out = IOWebFilterClass::tl_connect_out_func,
    .sf_bind = NULL,
    .sf_setoption = NULL,
    .sf_getoption = NULL,
    .sf_listen = NULL,
    .sf_ioctl = NULL
};

#pragma mark - kext callback
bool IOWebFilterClass::init (OSDictionary* dict)
{
    IOLog("enter");
    bool res = super::init(dict);
    if(res==false)
    {
        return false;
    }
    return true;
}

void IOWebFilterClass::free (void)
{
    IOLog("enter");

    super::free();
}

IOService* IOWebFilterClass::probe (IOService* provider, SInt32* score)
{
    IOService *res = super::probe(provider, score);
    IOLog("IOKitTest::probe\n");
    return res;
}

bool IOWebFilterClass::start (IOService *provider)
{
    bool res = super::start(provider);
    IOLog("IOKitTest::start\n");

    if(!_queue)
    {
        _queue = IOSharedEventQueue::withCapacity(MAX_SHAREDMEM_SIZE);
    }

    sflt_register(&ipv4_filter, PF_INET, SOCK_STREAM, IPPROTO_TCP);

    registerService();

    return res;
}

void IOWebFilterClass::stop (IOService *provider)
{
    IOLog("IOKitTest::stop\n");

    if(_queue)
    {
        _queue->release();
    }

    sflt_unregister(IPV4_SFLT_HANDLE);

    super::stop(provider);
}

IOReturn IOWebFilterClass::newUserClient(task_t owningTask, void * securityID,UInt32 type,
                                         OSDictionary * properties,IOUserClient ** handler )
{
    if(type!=IOWebFilterClientClientTypeID)
    {
        return kIOReturnBadArgument;
    }

    IOWebFilterClientClass *client = IOWebFilterClientClass::withTask(owningTask, securityID);
    IOReturn errorCode = kIOReturnSuccess;

    if(!client || !client->attach(this) || !client->start(this))
    {
        if(client)
        {
            client->detach(this);
            client->release();
            client=NULL;
        }
        errorCode = kIOReturnNoMemory;
        return errorCode;
    }
    *handler=client;

    super::newUserClient(owningTask, securityID, type, properties, handler);

    return errorCode;
}

#pragma mark - callback
void IOWebFilterClass::tl_unregistered_func(sflt_handle handle)
{

}

errno_t IOWebFilterClass::tl_attach_func(void	**cookie, socket_t so)
{
    LOG(LOG_DEBUG, "enter");

    SocketTracker *tracker = new SocketTracker();
    *cookie=(void*)tracker;

    if(tracker==NULL) return 0;

    bzero(tracker, sizeof(SocketTracker));
    tracker->lock = IOLockAlloc();
    if(tracker->lock==NULL)
    {
        tracker->magic=kSocketTrackerInvalid;
        return 0;
    }
    tracker->magic=kSocketTrackerAttach;
    tracker->pid = proc_selfpid();      //get the process id

    return 0;
}

/*
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
 IOLock *lock;
 pid_t pid;
 char request_meg[1024+1];
 }SocketTracker;
 */
void IOWebFilterClass::tl_detach_func(void *cookie, socket_t so)
{
//    LOG(LOG_DEBUG, "enter");
//    SocketTracker *tracker = (SocketTracker*)cookie;
//    if(tracker==NULL) return;
//    if(tracker->magic==kSocketTrackerInvalid)
//    {
//        delete tracker;
//        return ;
//    }
//
//    delete tracker;
//
//    if(tracker->lock)
//    {
//        IOLockFree(tracker->lock);
//        tracker->lock=NULL;
//    }
}

errno_t	IOWebFilterClass::tl_connect_out_func(void *cookie, socket_t so,
                                              const struct sockaddr *to)
{
    SocketTracker *tracker = (SocketTracker*)cookie;

    if(tracker==NULL || tracker->magic!=kSocketTrackerAttach) return 0;

    if(tracker->lock==NULL)
    {
        tracker->magic=kSocketTrackerInvalid;
        return 0;
    }

    struct sockaddr_in *sock_addr = (struct sockaddr_in*)to;
    //just handle the ipv4 address
    if(sock_addr==NULL)
    {
        if(sock_addr->sin_family==AF_INET6)
            tracker->magic=kSocketTrackerInvalid;
        return 0;
    }

    IOLockLock(tracker->lock);

    tracker->magic=kSocketTrackerConnect;   //update status

    //record source information
    struct sockaddr sockname;
    bzero(&sockname, sizeof(sockname));
    sock_getsockname(so, &sockname, sizeof(sockname));
    if(sockname.sa_family==AF_INET6)
    {
        tracker->magic=kSocketTrackerInvalid;
        return 0;
    }
    tracker->source.family = ((struct sockaddr_in*)&sockname)->sin_family;
    tracker->source.port = ((struct sockaddr_in*)&sockname)->sin_port;
    bzero(tracker->source.ip, sizeof(tracker->source.ip));
    snprintf(tracker->source.ip, sizeof(tracker->source.ip), "%u.%u.%u.%u", IPV4_ADDR(sock_addr->sin_addr.s_addr));

    if(strncmp(tracker->source.ip, "127.0.0.1", sizeof("127.0.0.1"))==0 ||
       (tracker->source.port!=80 && tracker->source.port!=8080))
    {
        tracker->magic=kSocketTrackerInvalid;
        if(tracker->lock) IOLockUnlock(tracker->lock);
        return 0;
    }

    //record destination information
    tracker->destination.family=sock_addr->sin_family;
    tracker->destination.port = sock_addr->sin_port;
    bzero(tracker->destination.ip, sizeof(tracker->destination.ip));
    snprintf(tracker->destination.ip, sizeof(tracker->destination.ip), "%u.%u.%u.%u", IPV4_ADDR(sock_addr->sin_addr.s_addr));

    IOLockUnlock(tracker->lock);

    return 0;
}

errno_t	IOWebFilterClass::tl_connect_in_func(void *cookie, socket_t so,
                                                const struct sockaddr *from)
{
    return 0;
}

errno_t	IOWebFilterClass::tl_data_in_func(void *cookie, socket_t so,
                                          const struct sockaddr *from, mbuf_t *data, mbuf_t *control,
                                          sflt_data_flag_t flags)
{
    LOG(LOG_DEBUG, "I am in");
    SocketTracker *tracker = (SocketTracker*)cookie;

    if(tracker==NULL || data==NULL || (tracker->magic&(kSocketTrackerInvalid|kSocketTrackerDetach))!=0) return 0;
    if(tracker->lock==NULL)
    {
        tracker->magic=kSocketTrackerInvalid;
        return 0;
    }

    IOLockLock(tracker->lock);
    mbuf_t head = *data;
    uint64_t len=0;

    if(head==NULL)
    {
        tracker->magic=kSocketTrackerInvalid;
        IOLockUnlock(tracker->lock);
        return 0;
    }

    while(head)
    {
        len += mbuf_len(head);
        head = mbuf_next(head);
    }

    if(len>sizeof(tracker->request_meg)-1)
    {
        tracker->magic=kSocketTrackerInvalid;
        IOLockUnlock(tracker->lock);
        return 0;
    }
    bzero(tracker->request_meg, sizeof(tracker->request_meg));
    mbuf_copydata(*data, 0, len, tracker->request_meg);

    //todo: sync to shared memory， record a new request
    if(_queue)
    {
        LOG(LOG_DEBUG, "ready to enter queue");
        _queue->EnqueueTracker((DataArgs*)tracker);
    }

    IOLockUnlock(tracker->lock);
    return 0;
}

errno_t	IOWebFilterClass::tl_data_out_func(void *cookie, socket_t so,
                                          const struct sockaddr *from, mbuf_t *data, mbuf_t *control,
                                          sflt_data_flag_t flags)
{
    return 0;
}