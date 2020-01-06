#if 0
#include <thread>
#include <chrono>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
int main()
{
    std::thread t1([] {
        printf("thread1 start. id:%#x\n", std::this_thread::get_id());
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        printf("thread1 start. id:%#x ~~~~\n", std::this_thread::get_id());
    });

    sleep(1);
    printf("before->detach id:%#x native_handle:%#x\n", t1.get_id(), t1.native_handle());
    t1.detach();
    printf("after->detach id:%#x native_handle:%#x\n", t1.get_id(), t1.native_handle());
    if (!t1.joinable())
    {
        t1 = std::thread([]{
            printf("thread2 start. id:%#x\n", std::this_thread::get_id());
        });
        printf("before->join id:%#x native_handle:%#x\n", t1.get_id(), t1.native_handle());
        t1.join();
        printf("after->join id:%#x native_handle:%#x\n", t1.get_id(), t1.native_handle());
    }
    sleep(2);

    return 0;
}
#endif

#include <chrono>
#include "ThreadObj.h"


class DemoThread : public ThreadObj
{
public:
    DemoThread() {};
    virtual ~DemoThread() {};
    virtual void Loop();
};

void DemoThread::Loop()
{
    while (!m_stopRequested)
    {
        LOGD("%s do work...", m_threadName.c_str());
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

static void printUsage(void)
{
    LOGD("Usage:");
    LOGD("    input ? :print usage");
    LOGD("    input 1 :start one thread");
    LOGD("    input 2 :stop one thread");
    LOGD("    input 3 :stop all threads");
    LOGD("    input 4 :list all threads");
    LOGD("    input q :quit.");
}

int main()
{
    char as8Buff[256]; 
    int isQuit = 0;
    std::list<DemoThread *> DemoList; 
    DemoThread *pObj = nullptr;
    int cnt;
    std::string tag;

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
                tag = "thread[" + std::to_string(cnt++) + "]";
                pObj = new DemoThread;
                pObj->StartThread(tag.c_str());
                DemoList.push_front(pObj);
                break;
            case '2':
                if (!DemoList.empty())
                {
                    pObj = DemoList.front();
                    pObj->StopThreadAndWait();
                    DemoList.pop_front();
                    delete pObj;
                }
                break;
            case '3':
                ThreadObj::StopAllThreads();
                for (auto *p : DemoList)
                {
                    delete p;
                }
                DemoList.clear();
                break;
            case '4':
                ThreadObj::DumpThreadObjList();
                break;
           case 'q':
                isQuit = 1;
                break;
            default:
                break;
        }

    } while (!isQuit);
 
    return 0;
}

