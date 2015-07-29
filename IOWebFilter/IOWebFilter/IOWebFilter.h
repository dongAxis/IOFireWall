/* add your code here */

#ifndef __IOWebFilter__IOWebFilter__
#define __IOWebFilter__IOWebFilter__

#include <IOKit/IOService.h>
#include <sys/kpi_socketfilter.h>
#include <libkern/OSMalloc.h>
#include <IOKit/IOMemoryDescriptor.h>

#include "IOSharedEventQueue.h"
#include "common.h"
#include "private.h"

class IOWebFilterClass : public IOService
{
    OSDeclareDefaultStructors(IOWebFilterClass);
private:
    static struct sflt_filter ipv4_filter;
    IOSharedEventQueue *_queue;
public:
    static volatile uint64_t CurrentNetworkData;
public:

    //@override
    virtual bool	init (OSDictionary* dictionary = NULL);
    virtual void	free (void);
    virtual IOService*	probe (IOService* provider, SInt32* score);
    virtual bool	start (IOService* provider);
    virtual void	stop (IOService* provider);
    virtual IOReturn newUserClient(task_t owningTask, void * securityID,UInt32 type,
                                   OSDictionary * properties,IOUserClient ** handler );

    //@get and set
    STATIC_GETANDSET_ATOMIC_INT64(IOWebFilterClass, CurrentNetworkData);
    //
    inline IOReturn setSharedQueueNotifyPort(mach_port_t port) {
        return this->_queue==NULL?kIOReturnError:(this->_queue->setNotificationPort(port), kIOReturnSuccess);
    }

    inline IOMemoryDescriptor* getSharedQueueDescriptor() {
        return (this->_queue==NULL?NULL:this->_queue->getMemoryDescriptor());
    }

private:
    static void tl_unregistered_func(sflt_handle handle);
    static errno_t tl_attach_func(void	**cookie, socket_t so);
    static void tl_detach_func(void *cookie, socket_t so);
    static errno_t	tl_connect_out_func(void *cookie, socket_t so, const struct sockaddr *to);
    static errno_t	tl_connect_in_func(void *cookie, socket_t so, const struct sockaddr *from);
    static errno_t	tl_data_in_func(void *cookie, socket_t so,
                                    const struct sockaddr *from, mbuf_t *data, mbuf_t *control,
                                    sflt_data_flag_t flags);
    static errno_t	tl_data_out_func(void *cookie, socket_t so,
                                     const struct sockaddr *from, mbuf_t *data, mbuf_t *control,
                                     sflt_data_flag_t flags);
};

#endif