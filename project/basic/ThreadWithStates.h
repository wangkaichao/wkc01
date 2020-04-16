#ifndef THREADWITHSTATES_H
#define THREADWITHSTATES_H

#include <functional>
#include "ThreadWithMsgQueue.h"

class ThreadWithStates : public ThreadWithMsgQueue
{
protect:
    template<class State, class SubState, class clsObj>
    struct StateTransition
    {
        State state;
        SubState sub;
        unsigned long ulMsgId;
        std::function<void(const clsObj&, Mesg *)> fun;
    };

public:
    ThreadWithStates() {};
    virtual ~ThreadWithStates() {};
    virtual THREAD_TYPE_E ThreadType() {return THREAD_TYPE_WITH_STATES;};

};

#endif

