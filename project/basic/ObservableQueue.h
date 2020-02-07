#ifndef OBSERVABLEQUEUE_H
#define OBSERVABLEQUEUE_H

#include <list>
#include <mutex>
#include <tuple>
#include <mqueue.h>
#include "MsgQueue.h"

class ObservableQueue
{

    struct Element_T
    {
        mqd_t fd;
        unsigned long ulSigName;
        unsigned long ulCbId;
    };

private:
    std::list<Element_T> m_list;
    std::mutex m_mtx;
    static int gExecuteCnt;

public:
    ObservableQueue() {};
    virtual ~ObservableQueue() {};

    virtual void Add(mqd_t fd, unsigned long ulSigName = 0, unsigned long ulCbId = 0);
    virtual void Del(mqd_t fd, unsigned long ulSigName = 0);
    virtual void Clear();
    virtual int Count(unsigned long ulSigName = 0);
    virtual bool IsHas(mqd_t fd, unsigned long ulSigName = 0);
    virtual int Notify(Mesg *pMsg);
};
#endif
