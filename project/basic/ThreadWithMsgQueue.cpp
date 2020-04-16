#include <sstream>
#include <chrono>

#include "ThreadWithMsgQueue.h"
#include "EnumMsg.h"

ThreadWithMsgQueue::~ThreadWithMsgQueue()
{
    ThreadWithMsgQueue::StopThread(true);
}

int ThreadWithMsgQueue::Init(const char *ps8MsgQueueName, long lMsgQueueSize)
{
    std::string name(ps8MsgQueueName);
    std::ostringstream os;
    os << (intptr_t)this;
    name += os.str();

    int rc = m_msgQueue.Open(name.c_str(), O_CREAT | O_RDWR | O_NONBLOCK, 0, lMsgQueueSize);
    if (rc >= 0)
    {
        m_initialized = true;
    }

    return rc;
}

int ThreadWithMsgQueue::SendMsg(Mesg *pMsg)
{
    int rc = 0;

    if (m_initialized)
    {
        rc = m_msgQueue.Send(pMsg);
    }
    else
    {
        if (pMsg->MsgId() != MSG_ID_STOP_THREAD)
        {
            LOGE("ThreadWithMsgQueue not initialized, SendMsg Failed. pthread_t:%lu", pthread_self());
            rc = ERR_THREAD_QUEUE_NOT_INITIALIZED;
        }
        pMsg->Free();
    }
    return rc;
}

int ThreadWithMsgQueue::StartThread(const char *name)
{
    int rc = 0;

    if (m_initialized)
    {
        ThreadObj::StartThread(name);
    }
    else
    {
        rc = ERR_THREAD_QUEUE_NOT_INITIALIZED;
    }
    return rc;
}

void ThreadWithMsgQueue::StopThread(bool isWaiting)
{
    if (m_initialized)
    {
        Mesg msg;
        msg.MsgId(MSG_ID_STOP_THREAD);
        msg.SendMsg(&m_msgQueue);
    }
    ThreadObj::StopThread(isWaiting);
}

void ThreadWithMsgQueue::ThreadFunction()
{
    LoopReadMsg(true);
    m_msgQueue.Clear();
}

void ThreadWithMsgQueue::LoopReadMsg(bool isPolling)
{
    int rc = 0;
    Mesg msg;

    while (!m_stopRequested)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        rc = m_msgQueue.Receive(&msg, MESG_PRIO_LOW, isPolling);
        if (rc >= 0)
        {
            if (msg.MsgId() != MSG_ID_STOP_THREAD)
            {
                ProcessMsg(&msg);
                m_nrOfProcessedMsgs++;
            }
            else
            {
                m_stopRequested = true;
            }
        }
        else
        {
            if (rc == ERR_MSG_QUEUE_SELECT_BUT_NOT_OUR_QUEUE)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(20000));
            }
            else
            {
                LOGE("ThreadWithMsgQueue read msg failed rc:%d tid:%d", rc, Tid());
            }
        }
    }
}

