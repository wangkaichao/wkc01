#include <list>
#include "ThreadWithMsgQueue.h"
#include "ObservableQueue.h"
#include "ObservableFunction.h"
#include "wm_log.h"

class ThreadApp : public ThreadWithMsgQueue
{
public:
    ThreadApp();
    void ProcessMsg(Mesg *pMsg);
    int FunctionMsg(Mesg *pMsg);

public:
    std::function<int(Mesg*)> m_fun;
};

ThreadApp::ThreadApp()
{
    using std::placeholders::_1;
    m_fun = std::bind(&ThreadApp::FunctionMsg, this, _1);
}

void ThreadApp::ProcessMsg(Mesg *pMsg)
{
    char *ptr = nullptr;
    int size = 0;

    std::tie(ptr, size) = pMsg->SigData();
    LOGD("sigId:%lu, data:%s, size:%d", pMsg->SigName(), ptr, size);
    pMsg->FreeSignal();
}

int ThreadApp::FunctionMsg(Mesg *pMsg)
{
    char *ptr = nullptr;
    int size = 0;

    std::tie(ptr, size) = pMsg->SigData();
    LOGD("sigId:%lu, data:%s, size:%d", pMsg->SigName(), ptr, size);
    pMsg->FreeSignal();
    return 0;
}

static void printUsage(void)
{
    LOGD("Usage:");
    LOGD("    input ? :print usage");
    LOGD("    input 1 :create obj");
    LOGD("    input 2 :send msg to all obj");
    LOGD("    input 3 :dump msg queue map");
    LOGD("    input 4 :dump thread list");
    LOGD("    input d :destroy obj");
    LOGD("    input q :quit.");
}

int main()
{
    char as8Buff[256]; 
    int isQuit = 0;
    std::list<ThreadWithMsgQueue *> DemoList; 
    ThreadApp *pObj = nullptr;
    unsigned long  sigName = 3; // Note SIG_ID_STOP_THREAD = 2;
    std::string name;
    Mesg msg;
    REQ_DATA_U unReq;
    ACK_DATA_U unAck;
    ObservableQueue observer;
    ObservableFunction observer2;

    LOG_OPEN("demo");
    printUsage();

    do
    {
        fgets(as8Buff, sizeof(as8Buff), stdin);

        switch (as8Buff[0])
        {
            case '?':
                printUsage();
                break;
            case '1':
                pObj = new ThreadApp;
                name = "/test";
                pObj->Init(name.c_str(), 100);

                name = "thread[" + std::to_string(DemoList.size()) + "]";
                pObj->StartThread(name.c_str());

                DemoList.push_front(pObj);
                observer.Add(pObj->MsgQueueFd());
                observer2.Add(pObj->m_fun);
                break;
            case '2':
                msg.SigName(sigName);
                unReq.s32Data = sigName;
                unAck.s32Data = sigName;
                msg.mMsg.tpSigCmd = std::make_tuple(unReq, unAck);
                msg.SigData(new char[10] {'1', '2', '3', '4', '5'}, 10);
                observer.Notify(&msg);
                observer2.Notify(&msg);
                msg.FreeSignal();
                sigName++;
                break;
            case '3':
                MsgQueue::DumpMsgQueueMap();
                break;
            case '4':
                ThreadObj::DumpThreadObjList();
                break;
            case 'd':
                if (!DemoList.empty())
                {
                    pObj = (ThreadApp *)DemoList.front();
                    observer.Del(pObj->MsgQueueFd());
                    observer2.Del(pObj->m_fun);
                    DemoList.pop_front();
                    delete pObj;
                }
                break;
            case 'q':
                isQuit = 1;
                observer.Clear();
                observer2.Clear();
                for (auto p : DemoList)
                {
                    delete p;
                }
                DemoList.clear();
                break;
            default:
                break;
        }

    } while (!isQuit);
 
    return 0;
}

