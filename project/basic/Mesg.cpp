/**
 * @file Mesg.cpp
 * @brief 
 * @author wangkaichao
 * @version 1.0
 * @date 2020-01-07
 */
#include "Mesg.h"
#include "MsgQueue.h"
#include "wm_log.h"
#include <stdlib.h>

Mesg::Mesg(const Mesg& clsMsg)
{
    MsgData(nullptr, 0);
    MsgObjPtr(nullptr);
    CopyMsg(clsMsg);
}

Mesg& Mesg::operator=(const Mesg& clsMsg)
{
    if (this != &clsMsg)
    {
        CopyMsg(clsMsg);
    }

    return *this;
}

int Mesg::CopyMsg(const Mesg& clsMsg)
{
    int rc = 0;

    Free();

    mMsg.ulMsgId = clsMsg.mMsg.ulMsgId;
    mMsg.tpMsgCmd = clsMsg.mMsg.tpMsgCmd;
    mMsg.ulCbId = clsMsg.mMsg.ulCbId;
    mMsg.u32SeqCnt = clsMsg.mMsg.u32SeqCnt;

    char *sigPtr;
    int sigSize;
    std::tie(sigPtr, sigSize) = clsMsg.mMsg.tpMsgData;

    if (sigPtr && sigSize > 0)
    {
        char *ptr = (char *)malloc(sigSize);
        if (ptr)
        {
            memcpy(ptr, sigPtr, sigSize);
            mMsg.tpMsgData = std::make_tuple(ptr, sigSize);
        }
        else
        {
            LOGE("Mesg new memory failed. msgId:%lu", clsMsg.mMsg.ulMsgId);
            rc = ERR_MSG_COPY_MSG_ALLOC_FAILED;
            mMsg.tpMsgData = std::make_tuple(nullptr, 0);
        }
    }
    else
    {
        mMsg.tpMsgData = std::make_tuple(nullptr, 0);
    }

    if (clsMsg.mMsg.pclsMsgObj)
    {
        mMsg.pclsMsgObj = clsMsg.mMsg.pclsMsgObj->Clone();
    }
    else
    {
        mMsg.pclsMsgObj = nullptr;
    }
    return rc;
}

void Mesg::Free()
{
    char *p = (char *)std::get<0>(mMsg.tpMsgData);
    if (p)
    {
        free(p);
        mMsg.tpMsgData = std::make_tuple(nullptr, 0);
    }

    if (mMsg.pclsMsgObj)
    {
        delete mMsg.pclsMsgObj;
        mMsg.pclsMsgObj = nullptr;
    }
}

int Mesg::SendMsg(MsgQueue *pclsMsgQueue, int s32Prio)
{
    int rc = MsgQueue::Send(pclsMsgQueue->MsgQueueFd(), this, s32Prio);
    if (rc != 0)
    {
        Free();
    }
    return rc;
}

int Mesg::SendMsg(mqd_t fd, int s32Prio)
{
    int rc = MsgQueue::Send(fd, this, s32Prio);
    if (rc != 0)
    {
        Free();
    }
    return rc;
}

int Mesg::ReceiveMsg(MsgQueue *pclsMsgQueue, int s32Prio, bool isPolling)
{
    int rc = pclsMsgQueue->Receive(this, s32Prio, isPolling);
    return rc;
}

