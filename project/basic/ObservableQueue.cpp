#include "ObservableQueue.h"
#include "wm_log.h"

int ObservableQueue::gExecuteCnt = 0;

void ObservableQueue::Add(mqd_t fd, unsigned long ulMsgId, unsigned long ulCbId)
{
    LOGD("this:%p, fd:%d, name:%s, signame:%lu", this, fd, MsgQueue::MsgQueueName(fd).c_str(), ulMsgId);
    CHK_ARG_RV(!fd || MsgQueue::IsBlocking(fd));

    std::lock_guard<std::mutex> lock(m_mtx);
    for (const auto e : m_list)
    {
        if ((e.fd == fd && e.ulMsgId == 0)
            || (e.fd == fd && e.ulMsgId == ulMsgId))
        {
            LOGD("already exist.");
            return;
        }
    }

    Element_T e = {fd, ulMsgId, ulCbId};
    m_list.push_front(e);
}

void ObservableQueue::Del(mqd_t fd, unsigned long ulMsgId)
{
    LOGD("this:%p, fd:%d, name:%s, signame:%lu", this, fd, MsgQueue::MsgQueueName(fd).c_str(), ulMsgId);

    std::lock_guard<std::mutex> lock(m_mtx);
    m_list.remove_if([fd, ulMsgId](const Element_T& e) {
        return (e.fd == fd && e.ulMsgId == ulMsgId);
    });
}

void ObservableQueue::Clear()
{
    std::lock_guard<std::mutex> lock(m_mtx);
    m_list.clear();
}

int ObservableQueue::Count(unsigned long ulMsgId)
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

bool ObservableQueue::IsHas(mqd_t fd, unsigned long ulMsgId)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    for (const auto e: m_list)
    {
        if ((e.fd == fd && e.ulMsgId == 0)
            || (e.fd == fd && e.ulMsgId == ulMsgId))
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
            LOGD("send-> fd:%d, name:%s, signame:%lu, d_addr:%p, cb_id:%lu",
                e.fd, MsgQueue::MsgQueueName(e.fd).c_str(), tmpMsg.MsgId(), tmpMsg.MsgDataPtr(), tmpMsg.CbId());
            CHK_FUN(MsgQueue::Send(e.fd, &tmpMsg), rc);
            gExecuteCnt++;
        }
    }
    return rc;
}

