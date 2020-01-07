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
    StopThreadAndWait();
}

void ThreadObj::StartThread(const char *name)
{
    if (m_thread.joinable())
    {
        LOGW("%s is already running", m_threadName.c_str());
        return;
    }

    m_threadName = name ? name : "(Unknown)";
    m_stopRequested = false;
    ThreadObj::AddThreadObj(this);
    m_thread = std::thread([this]() {
        prctl(PR_SET_NAME, m_threadName.c_str(), 0, 0, 0, 0);
        m_tid = syscall(SYS_gettid);

        LOGD("name:%s thread_id:%#x enter...", m_threadName.c_str(), m_tid);
        Loop();
        LOGD("name:%s thread_id:%#x leave...", m_threadName.c_str(), m_tid);
    });
}

void ThreadObj::StopThread()
{
    if (!m_thread.joinable())
        return;

    m_stopRequested = true;
    m_thread.detach();
    ThreadObj::DelThreadObj(this);
}

void ThreadObj::StopThreadAndWait()
{
    if (!m_thread.joinable())
        return;

    m_stopRequested = true;
    m_thread.join();
    ThreadObj::DelThreadObj(this);
}

void ThreadObj::StopAllThreads()
{
    std::lock_guard<std::mutex> lock(gListMtx);
    std::for_each(ThreadObj::gList.begin(), ThreadObj::gList.end(), 
        [](ThreadObj *pObj) {
            if (!pObj->m_thread.joinable())
                return;
            pObj->m_stopRequested = true;
            pObj->m_thread.join();
        });
    ThreadObj::gList.clear();
}

void ThreadObj::DumpThreadObjList(long selection)
{
    LOGD("\nList of thread objects");
    LOGD("\nTypes: 1 = while-loop, 2 = msg-loop, 3 = msg-loop with states\n");
    LOGD("Thread-Addr | Thread-Id | Type | Msgs    | Name");
    LOGD("------------+-----------+------+---------+-------------------------------------------");

    std::lock_guard<std::mutex> lock(gListMtx);
    for (const auto pObj : ThreadObj::gList)
    {
        if (selection == 0 || selection == (long)pObj->GetPOSIXTid()
                || selection == (long)pObj->GetTid())
        {
            LOGD("0x%8lx | %9ld | %4d | %7d | %s", (long)pObj->GetPOSIXTid(), (long)pObj->GetTid(), pObj->GetThreadType(), pObj->GetNrOfProcessedMsgs(), pObj->m_threadName.c_str());
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
    ThreadObj::gList.remove_if([pThreadObj](ThreadObj *p){return p == pThreadObj;});
}

