#include "ObservableFunction.h"
#include "wm_error.h"
#include "wm_log.h"

int ObservableFunction::gExecuteCnt = 0;

void ObservableFunction::Add(std::function<int(Mesg *)>& fun, unsigned long ulMsgId, unsigned long ulCbId)
{
    LOGD("this:%p, fun:%p, signame:%lu", this, &fun, ulMsgId);
    CHK_ARG_RV(fun == nullptr);

    std::lock_guard<std::mutex> lock(m_mtx);
    for (const auto e : m_list)
    {
        if ((&e.fun == &fun && e.ulMsgId == 0)
            || (&e.fun == &fun && e.ulMsgId == ulMsgId))
        {
            LOGD("already exist.");
            return;
        }
    }

    Element_T e = {fun, ulMsgId, ulCbId};
    m_list.push_front(e);
}

void ObservableFunction::Del(std::function<int(Mesg *)>& fun, unsigned long ulMsgId)
{
    LOGD("this:%p, fun:%p, signame:%lu", this, &fun, ulMsgId);

    std::lock_guard<std::mutex> lock(m_mtx);
    m_list.remove_if([&fun, ulMsgId](const Element_T& e) {
        return (&e.fun == &fun && e.ulMsgId == ulMsgId);
    });
}

void ObservableFunction::Clear()
{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_list.clear();
}

int ObservableFunction::Count(unsigned long ulMsgId)
{
    int cnt = 0;

    std::lock_guard<std::mutex> lock(m_mtx);
    for (const auto e : m_list)
    {
        if ((e.ulMsgId == 0) || (e.ulMsgId == ulMsgId))
        {
            cnt++;
        }
    }
    return cnt;
}

bool ObservableFunction::IsHas(std::function<int(Mesg *)>& fun, unsigned long ulMsgId)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    for (const auto e: m_list)
    {
        if ((&e.fun == &fun && e.ulMsgId == 0)
            || (&e.fun == &fun && e.ulMsgId == ulMsgId))
        {
            return true;
        }
    }
    return false;
}

int ObservableFunction::Notify(Mesg *pMsg)
{
    int rc = 0;

    CHK_ARG_RE(!pMsg, -1);
    std::lock_guard<std::mutex> lock(m_mtx);

    if (pMsg->MsgDataPtr() && !pMsg->MsgDataSize())
    {
        LOGE("msg size not set.");
        return ERR_OBS_MSG_SIZE_NOT_SET;
    }
 
    for (const auto e: m_list)
    {
        if ((e.ulMsgId == 0) || (e.ulMsgId == pMsg->MsgId()))
        {
            Mesg tmpMsg;
            
            pMsg->CbId(e.ulCbId);
            tmpMsg = *pMsg;
            LOGD("function-> fun:%p, signame:%lu, d_addr:%p, cb_id:%lu",
                &e.fun, tmpMsg.MsgId(), tmpMsg.MsgDataPtr(), tmpMsg.CbId());
            CHK_FUN(e.fun(&tmpMsg), rc);
            gExecuteCnt++;
        }
    }
    return rc;
}

