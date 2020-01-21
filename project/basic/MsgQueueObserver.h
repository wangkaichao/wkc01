#ifndef MSGQUEUEOBSERVER_H
#define MSGQUEUEOBSERVER_H

#include <mqueue.h>
#include <list>
#include <tuple>
#include <mutex>

#include "MsgQueue.h"

class MsgQueueObserver
{
private:
    std::list<std::tuple<mqd_t, unsigned long, long, long>> mList;
    std::mutex mMtx;

    static long gMsgQueueNotifyCnt;

public:
    MsgQueueObserver() {};
    virtual ~MsgQueueObserver() {};
    virtual void AddMsgQueue(mqd_t fd, unsigned long ulSigName = 0, long cbId = 0, long cbId2 = 0);
    virtual void DelMsgQueue(mqd_t fd, unsigned long ulSigName = 0);

    static long MsgQueueNotifyCnt() const {return gMsgQueueNotifyCnt;};
    static void MsgQueueNotifyCnt(long cnt) {gMsgQueueNotifyCnt = cnt;};
};

#endif

