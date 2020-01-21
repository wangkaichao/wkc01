/**
 * @file ThreadObj.h
 * @brief 
 * @author wangkaichao
 * @version 1.0
 * @date 2020-01-04
 */
#ifndef THREADOBJ_H
#define THREADOBJ_H

#include <string>
#include <thread>
#include <mutex>
#include <memory>
#include <list>
#include <pthread.h>
#include <sys/types.h>

#include "wm_log.h"

typedef enum
{
    THREAD_TYPE_NONE            = 0,
    THREAD_TYPE_WORK_FUNCTION   = 1,
    THREAD_TYPE_WITH_QUEUE      = 2,
    THREAD_TYPE_WITH_STATES     = 3,
    THREAD_TYPE_WITH_QUEUE_RPC  = 4

} THREAD_TYPE_E;


class ThreadObj
{
protected:
    std::string m_threadName;
    volatile bool m_stopRequested;

    std::mutex m_mtx;
    std::thread m_thread;

private:
    pid_t m_tid;

    /**
     * @brief Manager of ThreadObj.
     */
    static std::list<ThreadObj *> gList;
    static std::mutex gListMtx;

public:
    ThreadObj():m_stopRequested(true), m_tid(0) {};
    virtual ~ThreadObj();

    /**
     * @brief 
     *
     * @param name
     *
     * @return 
     */
    virtual int StartThread(const char *name = nullptr);

    /**
     * @brief 
     *
     * @param isWaiting
     */
    virtual void StopThread(bool isWaiting = true);

    /**
     * @brief After this class inherited, the child class is used to implement Loop() for the work of thread.
     */
    virtual void ThreadFunction() = 0;

    virtual THREAD_TYPE_E ThreadType() {return THREAD_TYPE_WORK_FUNCTION;};
    virtual unsigned int NrOfProcessedMsgs() const {return 0;};

    const char *ThreadName() {return m_threadName.c_str();};
    pthread_t POSIXTid() {return m_thread.native_handle();};
    pid_t Tid() {return m_tid;};

    /**
     * @brief the manager stop and clear all threads.
     */
    static void StopAllThreads();
    static void DumpThreadObjList(long selection = 0);
private:
    static void AddThreadObj(ThreadObj *pThreadObj);
    static void DelThreadObj(ThreadObj *pThreadObj);
};

#endif

