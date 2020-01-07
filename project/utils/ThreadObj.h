/**
 * @file ThreadObj.h
 * @brief 
 * @author wangkaichao
 * @version 1.0
 * @date 2020-01-04
 */
#ifndef __THREADOBJ_H__
#define __THREADOBJ_H__

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
    THREAD_TYPE_WITH_QUEUE_RPC  = 4,
    THREAD_TYPE_BLOCKING_WAIT   = 5

} M_ThreadType;


class ThreadObj
{
protected:
    std::string m_threadName;
    volatile bool m_stopRequested;

private:
    /**
     * @brief Manager of ThreadObj.
     */
    static std::list<ThreadObj *> gList;
    static std::mutex gListMtx;

    //std::mutex m_mtx;
    std::thread m_thread;
    pid_t m_tid;

public:
    ThreadObj():m_stopRequested(true), m_tid(0) {};
    virtual ~ThreadObj();
    /**
     * @brief joinable a thread.
     *
     * @param name
     */
    virtual void StartThread(const char *name = nullptr);
    /**
     * @brief stop a thead with detach.
     */
    virtual void StopThread();
    /**
     * @brief stop a thread with join.
     */
    virtual void StopThreadAndWait();
    /**
     * @brief After this class inherited, the child class is used to implement Loop() for the work of thread.
     */
    virtual void Loop() = 0;

    const char *GetThreadName() {return m_threadName.c_str();};
    pthread_t GetPOSIXTid() {return m_thread.native_handle();};
    pid_t GetTid() {return m_tid;};

    virtual M_ThreadType GetThreadType() {return THREAD_TYPE_WORK_FUNCTION;};
    virtual unsigned int GetNrOfProcessedMsgs() const {return 0;};

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

