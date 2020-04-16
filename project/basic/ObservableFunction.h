#ifndef OBSERVABLEFUNCTION_H
#define OBSERVABLEFUNCTION_H

#include <list>
#include <mutex>
#include <tuple>
#include <functional>
#include "Mesg.h"

class ObservableFunction
{
    struct Element_T
    {
        std::function<int(Mesg *)>& fun;
        unsigned long ulMsgId;
        unsigned long ulCbId;
    };

private:
    std::list<Element_T> m_list;
    std::mutex m_mtx;
    static int gExecuteCnt;

public:
    ObservableFunction() {};
    virtual ~ObservableFunction() {};

    virtual void Add(std::function<int(Mesg *)>& fun, unsigned long ulMsgId = 0, unsigned long ulCbId = 0);
    virtual void Del(std::function<int(Mesg *)>& fun, unsigned long ulMsgId = 0);
    virtual void Clear();
    virtual int Count(unsigned long ulMsgId = 0);
    virtual bool IsHas(std::function<int(Mesg *)>& fun, unsigned long ulMsgId = 0);
    virtual int Notify(Mesg *pMsg);
};
#endif
