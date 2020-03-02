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
    SigData(nullptr, 0);
    SigObjPtr(nullptr);
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

    FreeSignal();

    mMsg.ulSigName = clsMsg.mMsg.ulSigName;
    mMsg.tpSigCmd = clsMsg.mMsg.tpSigCmd;
    mMsg.ulCbId = clsMsg.mMsg.ulCbId;
    mMsg.u32SeqCnt = clsMsg.mMsg.u32SeqCnt;

    char *sigPtr;
    int sigSize;
    std::tie(sigPtr, sigSize) = clsMsg.mMsg.tpSigData;

    if (sigPtr && sigSize > 0)
    {
        char *ptr = (char *)malloc(sigSize);
        if (ptr)
        {
            memcpy(ptr, sigPtr, sigSize);
            mMsg.tpSigData = std::make_tuple(ptr, sigSize);
        }
        else
        {
            LOGE("Mesg new memory failed. signal:%lu", clsMsg.mMsg.ulSigName);
            rc = ERR_MSG_COPY_MSG_ALLOC_FAILED;
            mMsg.tpSigData = std::make_tuple(nullptr, 0);
        }
    }
    else
    {
        mMsg.tpSigData = std::make_tuple(nullptr, 0);
    }

    if (clsMsg.mMsg.pclsSigObj)
    {
        mMsg.pclsSigObj = clsMsg.mMsg.pclsSigObj->Clone();
    }
    else
    {
        mMsg.pclsSigObj = nullptr;
    }
    return rc;
}

void Mesg::FreeSignal()
{
    char *p = (char *)std::get<0>(mMsg.tpSigData);
    if (p)
    {
        free(p);
        mMsg.tpSigData = std::make_tuple(nullptr, 0);
    }

    if (mMsg.pclsSigObj)
    {
        delete mMsg.pclsSigObj;
        mMsg.pclsSigObj = nullptr;
    }
}

int Mesg::SendMsg(MsgQueue *pclsMsgQueue, int s32Prio)
{
    int rc = MsgQueue::Send(pclsMsgQueue->MsgQueueFd(), this, s32Prio);
    if (rc != 0)
    {
        FreeSignal();
    }
    return rc;
}

int Mesg::SendMsg(mqd_t fd, int s32Prio)
{
    int rc = MsgQueue::Send(fd, this, s32Prio);
    if (rc != 0)
    {
        FreeSignal();
    }
    return rc;
}

int Mesg::ReceiveMsg(MsgQueue *pclsMsgQueue, int s32Prio, bool isPolling)
{
    int rc = pclsMsgQueue->Receive(this, s32Prio, isPolling);
    return rc;
}

