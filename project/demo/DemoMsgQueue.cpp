#include <list>
#include "MsgQueue.h"
#include "Mesg.h"
#include "wm_log.h"

static void printUsage(void)
{
    LOGD("Usage:");
    LOGD("    input ? :print usage");
    LOGD("    input 1 :open one msg queue");
    LOGD("    input 2 :send msg");
    LOGD("    input 3 :receive msg");
    LOGD("    input 4 :dump msg queue map");
    LOGD("    input d :unlink one msg queue");
    LOGD("    input q :quit.");
}

int main()
{
    char as8Buff[256]; 
    int isQuit = 0;
    std::list<MsgQueue *> DemoList; 
    MsgQueue *pObj = nullptr;
    int cnt = 0, sigCnt = 0;
    std::string name;
    Mesg msg, msgRcv;
    char *ptr = nullptr;
    int size = 0;
    REQ_DATA_U unReq;
    ACK_DATA_U unAck;
 
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
                name = "/test_" + std::to_string(cnt++);
                pObj = new MsgQueue;
                pObj->Open(name.c_str(), O_CREAT | O_RDWR | O_NONBLOCK, 0, 100);
                DemoList.push_front(pObj);
                break;
            case '2':
                msg.mMsg.ulSigName = sigCnt;
                unReq.s32Data = sigCnt;
                unAck.s32Data = sigCnt;
                msg.mMsg.tpSigCmd = std::make_tuple(unReq, unAck);
                size = 10;
                ptr = (char *)malloc(size);
                strncpy(ptr, "12345", size);
                msg.mMsg.tpSigData = std::make_tuple(ptr, size);

                for (auto p : DemoList)
                {
                    Mesg msgSnd(msg);
                    p->Send(&msgSnd);
                }

                msg.FreeSignal();
                sigCnt++;
                break;
            case '3':
                for (auto p : DemoList)
                {
                    ptr = nullptr;
                    size = 0;
                    p->Receive(&msgRcv);
                    std::tie(ptr, size) = msgRcv.mMsg.tpSigData;
                    LOGD("sigId:%lu, data:%s, size:%d", msgRcv.mMsg.ulSigName, ptr, size);
                    msgRcv.FreeSignal();
                }
                break;
            case '4':
                MsgQueue::DumpMsgQueueMap();
                break;
            case 'd':
                if (!DemoList.empty())
                {
                    pObj = DemoList.front();
                    delete pObj;
                    DemoList.pop_front();
                }
                break;
            case 'q':
                isQuit = 1;
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

