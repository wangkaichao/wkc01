#include <algorithm>
#include <sys/prctl.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "ThreadObj.h"
#include "wm_log.h"

std::list<ThreadObj *> ThreadObj::gList;
std::mutex ThreadObj::gListMtx;

ThreadObj::~ThreadObj()
{
    StopThread(true);
}

int ThreadObj::StartThread(const char *name)
{
    if (m_thread.joinable())
    {
        LOGW("%s is already running", m_threadName.c_str());
        return 0;
    }

    m_threadName = name ? name : "(Unknown)";
    m_stopRequested = false;
    ThreadObj::AddThreadObj(this);
    m_thread = std::thread([this]() {
        prctl(PR_SET_NAME, m_threadName.c_str(), 0, 0, 0, 0);
        m_tid = syscall(SYS_gettid);

        LOGD("name:%s thread_id:%#x enter...", m_threadName.c_str(), m_tid);
        ThreadFunction();
        LOGD("name:%s thread_id:%#x leave...", m_threadName.c_str(), m_tid);
    });
    return 0;
}

void ThreadObj::StopThread(bool isWaiting)
{
    if (!m_thread.joinable())
        return;

    m_stopRequested = true;
    if (isWaiting)
    {
        m_thread.join();
    }
    else
    {
        m_thread.detach();
    }

    ThreadObj::DelThreadObj(this);
}

void ThreadObj::StopAllThreads()
{
    std::lock_guard<std::mutex> lock(gListMtx);
    std::for_each(gList.begin(), gList.end(), [](ThreadObj *pObj) {
        if (!pObj->m_thread.joinable())
            return;
        pObj->m_stopRequested = true;
        pObj->m_thread.join();
    });
    gList.clear();
}

void ThreadObj::DumpThreadObjList(long selection)
{
    LOGD("\nList of thread objects");
    LOGD("\nTypes: 1 = while-loop, 2 = msg-loop, 3 = msg-loop with states\n");
    LOGD("Thread-Addr | Thread-Id | Type | Msgs    | Name");
    LOGD("------------+-----------+------+---------+-------------------------------------------");

    std::lock_guard<std::mutex> lock(gListMtx);
    for (const auto pObj : gList)
    {
        if (selection == 0 || selection == (long)pObj->POSIXTid()
                || selection == (long)pObj->m_tid)
        {
            LOGD("0x%8lx | %9ld | %4d | %7d | %s", (long)pObj->POSIXTid(), (long)pObj->m_tid, pObj->ThreadType(), pObj->NrOfProcessedMsgs(), pObj->m_threadName.c_str());
        }
    }
    LOGD("------------+-----------+------+---------+-------------------------------------------");
}

void ThreadObj::AddThreadObj(ThreadObj *pThreadObj)
{
    CHK_ARG_RV(pThreadObj == nullptr);
    std::lock_guard<std::mutex> lock(gListMtx);
    gList.push_front(pThreadObj);
}

void ThreadObj::DelThreadObj(ThreadObj *pThreadObj)
{
    CHK_ARG_RV(pThreadObj == nullptr);
    std::lock_guard<std::mutex> lock(gListMtx);
    gList.remove_if([pThreadObj](ThreadObj *p){return p == pThreadObj;});
}

