/**
 * @file ThreadWithMsgQueue.h
 * @brief 
 * @author wangkichao
 * @version 1.0
 * @date 2020-01-08
 */
#ifndef THREADWITHMSGQUEUE_H
#define THREADWITHMSGQUEUE_H

#include "Mesg.h"
#include "MsgQueue.h"
#include "ThreadObj.h"

#define ERR_THREAD_QUEUE_NOT_INITIALIZED        -1110

class ThreadWithMsgQueue : public ThreadObj
{
protected:
    MsgQueue m_msgQueue;
    bool m_initialized;
    unsigned int m_nrOfProcessedMsgs;

public:
    ThreadWithMsgQueue():m_initialized(false), m_nrOfProcessedMsgs(0) {};
    virtual ~ThreadWithMsgQueue();

    int Init(const char *ps8MsgQueueName, long lMsgQueueSize);
    virtual int SendMsg(Mesg *pMsg);
    
    virtual int StartThread(const char *name = nullptr);
    virtual void StopThread(bool isWaiting = true);

    virtual THREAD_TYPE_E ThreadType() {return THREAD_TYPE_WITH_QUEUE;};
    virtual unsigned int NrOfProcessedMsgs() const {return m_nrOfProcessedMsgs;};

    mqd_t MsgQueueFd() const {return m_msgQueue.MsgQueueFd();};
    const char *MsgQueueName() const {return m_msgQueue.MsgQueueName();};

protected:
    virtual void ProcessMsg(Mesg *pMsg) = 0;

private:
    virtual void ThreadFunction();
    virtual void LoopReadMsg(bool isPolling);
};

#endif
