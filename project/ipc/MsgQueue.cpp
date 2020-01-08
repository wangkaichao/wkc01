/**
 * @file MsgQueue.cpp
 * @brief 
 * @author wangkaichao
 * @version 1.0
 * @date 2020-01-07
 */
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <sys/select.h>
#include <errno.h>

#include "MsgQueue.h"
#include "SignalNo.h"
#include "wm_log.h"

unsigned long MsgQueue::gReceiveMsgCnt = 0;
unsigned long MsgQueue::gSendMsgCnt = 0;

std::unordered_map<mqd_t, MsgQueue *> MsgQueue::gMap;
std::mutex MsgQueue::gMapMtx;

MsgQueue::~MsgQueue()
{
    Unlink();
}

int MsgQueue::Open(const char *name, int flag, mode_t mode, long maxmsg)
{
    struct mq_attr mqAttr;

    CHK_ARG_RE(name == nullptr, ERR_MSG_QUEUE_PTR_IS_NULL);

    LOGD("MsgQueue name:%s", name);
    mq_unlink(name);
    m_receiveMsgCnt = 0;
    m_sendMsgCnt = 0;
    m_lastMsgSigName = 0;

    m_msgQueueName = name;
    m_maxMsgInQueue = 0;

    mqAttr.mq_maxmsg = maxmsg;
    mqAttr.mq_msgsize = Mesg::MsgSize();
    mqAttr.mq_flags = 0;
    mqAttr.mq_curmsgs = 0;
    mqAttr.__pad[0] = 0;
    mqAttr.__pad[1] = 0;
    mqAttr.__pad[2] = 0;
    mqAttr.__pad[3] = 0;
    m_msgQueueFd = mq_open(name, flag, mode, &mqAttr);
    if (m_msgQueueFd < 0)
    {
        LOGE("mq_open failed:%m");
        return ERR_MSG_QUEUE_OPEN_FAILED;
    }
    
    AddMsgQueue(m_msgQueueFd, this);

    // select need.
    FD_ZERO(&m_rfds);
    FD_SET(m_msgQueueFd, &m_rfds);
    return 0;
}

int MsgQueue::Open(const char *name, int flag, mode_t mode, struct mq_attr *pattr)
{
    CHK_ARG_RE(name == nullptr || pattr == nullptr, ERR_MSG_QUEUE_PTR_IS_NULL);

    LOGD("MsgQueue name:%s", name);
    mq_unlink(name);
    m_receiveMsgCnt = 0;
    m_sendMsgCnt = 0;

    m_msgQueueName = name;
    m_maxMsgInQueue = 0;

    pattr->mq_msgsize = Mesg::MsgSize();
    m_msgQueueFd = mq_open(name, flag, mode, pattr);
    if (m_msgQueueFd < 0)
    {
        LOGE("mq_open failed:%m");
        return ERR_MSG_QUEUE_OPEN_FAILED;
    }
    
    AddMsgQueue(m_msgQueueFd, this);

    // select need.
    FD_ZERO(&m_rfds);
    FD_SET(m_msgQueueFd, &m_rfds);
    return 0;
}

int MsgQueue::Close()
{
    CHK_ARG_RE(m_msgQueueFd < 0, ERR_MSG_QUEUE_CLOSE_NO_HANDLE);
    CHK_FUN_RE_M(mq_close(m_msgQueueFd), ERR_MSG_QUEUE_CLOSE_FAILED);
    DelMsgQueue(m_msgQueueFd, this);
    return 0;
}

int MsgQueue::Unlink()
{
    if (m_msgQueueName.empty())
    {
        return ERR_MSG_QUEUE_NAME_UNKNOWN;
    }

    ClearQueue();
    Close();
    CHK_FUN_RE_M(mq_unlink(m_msgQueueName.c_str()), ERR_MSG_QUEUE_UNLINK_FAILED);
    return 0;
}

void MsgQueue::ClearQueue()
{
    DumpMsgQueue(m_msgQueueFd);
}

int MsgQueue::Send(Mesg *pclsMsg, unsigned int s32Prio)
{
    CHK_ARG_RE(pclsMsg == nullptr, ERR_MSG_QUEUE_PTR_IS_NULL);
    SIGNAL_ID_E enSigId = (SIGNAL_ID_E)pclsMsg->SigName();
    if (m_msgQueueFd <= 0)
    {
        pclsMsg->FreeSignal();
        LOGE("msg queue handle<=0. signal=%d %s", enSigId, GetSignalIdName(enSigId));
        return ERR_MSG_QUEUE_SEND_HANDLE_IS_NULL;
    }

    if (pclsMsg->SigDataPtr() && pclsMsg->SigDataSize() == 0)
    {
        LOGW("Signal pointer is set, but size is 0. signal=%d %s", enSigId, GetSignalIdName(enSigId));
    }
    if (pclsMsg->SigName() == 0)
    {
        LOGW("signal = 0 Thread-Id = %ld", syscall(SYS_gettid));
    }

    int rc = mq_send(m_msgQueueFd, pclsMsg->MsgAddr(), Mesg::MsgSize(), s32Prio);
    if (rc == -1)
    {
        if (!m_msgQueueNotInUse)
        {
            LOGE("msg send failed:%m. signal=%d %s", enSigId, GetSignalIdName(enSigId));
        }
        pclsMsg->FreeSignal();
        return ERR_MSG_QUEUE_SEND_FAILED;
    }

    pclsMsg->SigData(nullptr, 0);
    pclsMsg->SigObjPtr(nullptr);
    m_sendMsgCnt++;
    gSendMsgCnt++;
    return 0;
}

// static
int MsgQueue::Send(mqd_t fd, Mesg *pclsMsg, unsigned int s32Prio)
{
    CHK_ARG_RE(pclsMsg == nullptr, ERR_MSG_QUEUE_PTR_IS_NULL);

    MsgQueue *p = MsgQueue::MsgQueuePtr(fd);
    if (p)
    {
        return p->Send(pclsMsg, s32Prio);
    }

    SIGNAL_ID_E enSigId = (SIGNAL_ID_E)pclsMsg->SigName();
    if (fd <= 0)
    {
        pclsMsg->FreeSignal();
        LOGE("msg queue handle<=0. signal=%d %s", enSigId, GetSignalIdName(enSigId));
        return ERR_MSG_QUEUE_SEND_HANDLE_IS_NULL;
    }

    if (pclsMsg->SigDataPtr() && pclsMsg->SigDataSize() == 0)
    {
        LOGW("Signal pointer is set, but size is 0. signal=%d %s", enSigId, GetSignalIdName(enSigId));
    }
    if (pclsMsg->SigName() == 0)
    {
        LOGW("signal = 0 Thread-Id = %ld", syscall(SYS_gettid));
    }

    int rc = mq_send(fd, pclsMsg->MsgAddr(), Mesg::MsgSize(), s32Prio);
    if (rc == -1)
    {
        LOGE("msg send failed:%m. signal=%d %s", enSigId, GetSignalIdName(enSigId));
        pclsMsg->FreeSignal();
        return ERR_MSG_QUEUE_SEND_FAILED;
    }

    pclsMsg->SigData(nullptr, 0);
    pclsMsg->SigObjPtr(nullptr);
    return 0;
}

int MsgQueue::Receive(Mesg *pclsMsg, unsigned int s32Prio)
{
    CHK_ARG_RE(pclsMsg == nullptr, ERR_MSG_QUEUE_PTR_IS_NULL);
    if (m_msgQueueFd <= 0)
    {
        LOGE("msg queue handle<=0");
        return ERR_MSG_QUEUE_SEND_HANDLE_IS_NULL;
    }
    m_msgQueueNotInUse = false;

    int rc_size = mq_receive(m_msgQueueFd, pclsMsg->MsgAddr(), Mesg::MsgSize(), NULL);
    //LOGD("receive size:%d, response info:%d", rc_size, \
        std::get<1>(pclsMsg->mMsg.tpSigCmd).s32Data);
    if (rc_size <= 0)
    {
        LOGE("msg queue rcv failed:%m, fd:%d rc_size:%d", m_msgQueueFd, rc_size);
        return ERR_MSG_QUEUE_RECEIVE_FAILED;
    }

    m_lastMsgSigName = pclsMsg->SigName();
    m_receiveMsgCnt++;
    gReceiveMsgCnt++;

    long curmsgs = CurMsgsInQueue() + 1;
    if (curmsgs > m_maxMsgInQueue)
    {
        m_maxMsgInQueue = curmsgs;
    }
    return 0;
}

int MsgQueue::ReceiveByPoll(Mesg *pclsMsg, unsigned int s32Prio)
{
    CHK_ARG_RE(pclsMsg == nullptr, ERR_MSG_QUEUE_PTR_IS_NULL);
    if (m_msgQueueFd <= 0)
    {
        LOGE("msg queue handle<=0");
        return ERR_MSG_QUEUE_SEND_HANDLE_IS_NULL;
    }
    m_msgQueueNotInUse = false;

    int rc = select(m_msgQueueFd + 1, &m_rfds, NULL, NULL, NULL);
    if (rc < 0)
    {
        LOGE("msg queue rcv select failed:%m, fd:%d rc:%d", m_msgQueueFd, rc);
        return ERR_MSG_QUEUE_SELECT_FAILED;
    }

    if (FD_ISSET(m_msgQueueFd, &m_rfds))
    {
        int rc_size = mq_receive(m_msgQueueFd, pclsMsg->MsgAddr(), Mesg::MsgSize(), NULL);
        //LOGD("receive size:%d, response info:%d", rc_size,\
            std::get<1>(pclsMsg->mMsg.tpSigCmd).s32Data);
        if (rc_size <= 0)
        {
            LOGE("msg queue rcv failed:%m, fd:%d rc_size:%d", m_msgQueueFd, rc_size);
            return ERR_MSG_QUEUE_RECEIVE_FAILED;
        }

        m_lastMsgSigName = pclsMsg->SigName();
        m_receiveMsgCnt++;
        gReceiveMsgCnt++;

        long curmsgs = CurMsgsInQueue() + 1;
        if (curmsgs > m_maxMsgInQueue)
        {
            m_maxMsgInQueue = curmsgs;
        }
    }
    else
    { 
        LOGE("msg queue rcv select but not our queue, fd:%d rc:%d", m_msgQueueFd, rc);
        return ERR_MSG_QUEUE_SELECT_BUT_NOT_OUR_QUEUE;
    }

    return 0;
}

long MsgQueue::CurMsgsInQueue()
{
    struct mq_attr mqAttr;

    CHK_ARG_RE(m_msgQueueFd < 0, ERR_MSG_QUEUE_GETNR_NO_HANDLE);
    CHK_FUN_RE_M(mq_getattr(m_msgQueueFd, &mqAttr), ERR_MSG_QUEUE_GETNR_FAILED);
    return mqAttr.mq_curmsgs;
}

// static
std::string MsgQueue::MsgQueueName(mqd_t fd)
{
    std::lock_guard<std::mutex> lock(gMapMtx);
    auto it = gMap.find(fd);
    return it != gMap.end() ? it->second->m_msgQueueName : "Unknown";
}

MsgQueue *MsgQueue::MsgQueuePtr(mqd_t fd)
{
    std::lock_guard<std::mutex> lock(gMapMtx);
    auto it = gMap.find(fd);
    return it != gMap.end() ? it->second : nullptr;
}

void MsgQueue::DumpMsgQueueMap(int selection)
{
    struct mq_attr mqAttr;
    mqd_t fd;
    long lMaxMsg;
    long lCurMsgs;
    const char *ps8Block = "UNKWN";

    LOGD("List of queues");
    LOGD("handle | size | max. used | cur. used | block | last msg | nr read | name");
    LOGD("-------+------+-----------+-----------+-------+----------+---------+------------------------------------");

    std::lock_guard<std::mutex> lock(gMapMtx);
    for (const auto &p : gMap)
    {
        fd = p.first;

        if (selection != 0 && selection != (int)fd)
        {
            continue;
        }

        if (!mq_getattr(fd, &mqAttr))
        {
            lMaxMsg  = mqAttr.mq_maxmsg;
            lCurMsgs = mqAttr.mq_curmsgs;
            ps8Block = mqAttr.mq_flags & O_NONBLOCK ? "NONBL" : "BLOCK";
        }
        else
        {
            LOGE("get msg queue attributes by mq_t:%d failed", fd);
            lMaxMsg  = 0;
            lCurMsgs = 0;
            ps8Block = "UNKWN";
        }
        LOGD("%6d | %4ld | %9ld | %9ld | %s | %8lu | %7lu | %s\n",
            fd, lMaxMsg, p.second->m_maxMsgInQueue, lCurMsgs, ps8Block,
            p.second->m_lastMsgSigName,
            p.second->m_receiveMsgCnt,
            p.second->m_msgQueueName.c_str());
    }
    LOGD("-------+------+-----------+-----------+-------+----------+---------+------------------------------------");
}

void MsgQueue::DumpMsgQueue(mqd_t fd)
{
    int rc_size = 0;
    unsigned int msgNr = 0;
    Mesg clsMsg;

    do
    {
        rc_size = mq_receive(fd, (char *)clsMsg.MsgAddr(), Mesg::MsgSize(), NULL);
        if (rc_size > 0)
        {
            msgNr++;
            LOGD("Msg read from queue - nr: %d  signalname: %lu",
                msgNr, clsMsg.SigName());
            clsMsg.FreeSignal();
        }
    }
    while (rc_size > 0);
}

void MsgQueue::AddMsgQueue(mqd_t fd, MsgQueue *pclsMsgQueue)
{
    CHK_ARG_RV(pclsMsgQueue == nullptr);
    std::lock_guard<std::mutex> lock(gMapMtx);
    gMap[fd] = pclsMsgQueue;
}

void MsgQueue::DelMsgQueue(mqd_t fd, MsgQueue *pclsMsgQueue)
{
    CHK_ARG_RV(pclsMsgQueue == nullptr);
    std::lock_guard<std::mutex> lock(gMapMtx);
    gMap.erase(fd);
}

