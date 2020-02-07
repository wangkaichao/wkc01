#include "ObservableQueue.h"
#include "wm_log.h"

int ObservableQueue::gExecuteCnt = 0;

void ObservableQueue::Add(mqd_t fd, unsigned long ulSigName, unsigned long ulCbId)
{
    LOGD("this:%p, fd:%d, name:%s, signame:%lu", this, fd, MsgQueue::MsgQueueName(fd).c_str(), ulSigName);
    CHK_ARG_RV(!fd || MsgQueue::IsBlocking(fd));

    std::lock_guard<std::mutex> lock(m_mtx);
    for (const auto e : m_list)
    {
        if ((e.fd == fd && e.ulSigName == 0)
            || (e.fd == fd && e.ulSigName == ulSigName))
        {
            LOGD("already exist.");
            return;
        }
    }

    Element_T e;
    e.fd = fd;
    e.ulSigName = ulSigName;
    e.ulCbId = ulCbId;
    m_list.push_front(e);
}

void ObservableQueue::Del(mqd_t fd, unsigned long ulSigName)
{
    LOGD("this:%p, fd:%d, name:%s, signame:%lu", this, fd, MsgQueue::MsgQueueName(fd).c_str(), ulSigName);

    std::lock_guard<std::mutex> lock(m_mtx);
    m_list.remove_if([fd, ulSigName](const Element_T& e) {
        return (e.fd == fd && e.ulSigName == ulSigName);
    });
}

void ObservableQueue::Clear()
{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_list.clear();
}

int ObservableQueue::Count(unsigned long ulSigName)
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

bool ObservableQueue::IsHas(mqd_t fd, unsigned long ulSigName)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    for (const auto e: m_list)
    {
        if ((e.fd == fd && e.ulSigName == 0)
            || (e.fd == fd && e.ulSigName == ulSigName))
        {
            return true;
        }
    }
    return false;
}

int ObservableQueue::Notify(Mesg *pMsg)
{
    int rc = 0;

    CHK_ARG_RE(!pMsg, -1);
    std::lock_guard<std::mutex> lock(m_mtx);

    if (pMsg->SigDataPtr() && !pMsg->SigDataSize())
    {
        pMsg->FreeSignal();
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
            LOGD("send-> fd:%d, name:%s, signame:%lu, d_addr:%p, cb_id:%lu",
                e.fd, MsgQueue::MsgQueueName(e.fd).c_str(), tmpMsg.SigName(), tmpMsg.SigDataPtr(), tmpMsg.CbId());
            CHK_FUN(MsgQueue::Send(e.fd, &tmpMsg), rc);
            gExecuteCnt++;
        }
    }

    pMsg->FreeSignal();
    return rc;
}

