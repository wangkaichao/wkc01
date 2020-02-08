#include "ObservableFunction.h"
#include "wm_error.h"
#include "wm_log.h"

int ObservableFunction::gExecuteCnt = 0;

void ObservableFunction::Add(std::function<int(Mesg *)>& fun, unsigned long ulSigName, unsigned long ulCbId)
{
    LOGD("this:%p, fun:%p, signame:%lu", this, &fun, ulSigName);
    CHK_ARG_RV(fun == nullptr);

    std::lock_guard<std::mutex> lock(m_mtx);
    for (const auto e : m_list)
    {
        if ((&e.fun == &fun && e.ulSigName == 0)
            || (&e.fun == &fun && e.ulSigName == ulSigName))
        {
            LOGD("already exist.");
            return;
        }
    }

    Element_T e = {fun, ulSigName, ulCbId};
    m_list.push_front(e);
}

void ObservableFunction::Del(std::function<int(Mesg *)>& fun, unsigned long ulSigName)
{
    LOGD("this:%p, fun:%p, signame:%lu", this, &fun, ulSigName);

    std::lock_guard<std::mutex> lock(m_mtx);
    m_list.remove_if([&fun, ulSigName](const Element_T& e) {
        return (&e.fun == &fun && e.ulSigName == ulSigName);
    });
}

void ObservableFunction::Clear()
{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_list.clear();
}

int ObservableFunction::Count(unsigned long ulSigName)
{
    int cnt = 0;

    std::lock_guard<std::mutex> lock(m_mtx);
    for (const auto e : m_list)
    {
        if ((e.ulSigName == 0) || (e.ulSigName == ulSigName))
        {
            cnt++;
        }
    }
    return cnt;
}

bool ObservableFunction::IsHas(std::function<int(Mesg *)>& fun, unsigned long ulSigName)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    for (const auto e: m_list)
    {
        if ((&e.fun == &fun && e.ulSigName == 0)
            || (&e.fun == &fun && e.ulSigName == ulSigName))
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

    if (pMsg->SigDataPtr() && !pMsg->SigDataSize())
    {
        LOGE("signal size not set.");
        return ERR_OBS_SIGNAL_SIZE_NOT_SET;
    }
 
    for (const auto e: m_list)
    {
        if ((e.ulSigName == 0) || (e.ulSigName == pMsg->SigName()))
        {
            Mesg tmpMsg;
            
            pMsg->CbId(e.ulCbId);
            tmpMsg = *pMsg;
            LOGD("function-> fun:%p, signame:%lu, d_addr:%p, cb_id:%lu",
                &e.fun, tmpMsg.SigName(), tmpMsg.SigDataPtr(), tmpMsg.CbId());
            CHK_FUN(e.fun(&tmpMsg), rc);
            gExecuteCnt++;
        }
    }
    return rc;
}

