/**
 * @file MsgQueue.h
 * @brief 
 * @author wangkaichao
 * @version 1.0
 * @date 2020-01-07
 */
#ifndef MSGQUEUE_H
#define MSGQUEUE_H

#include <string>
#include <mutex>
#include <unordered_map>
#include <mqueue.h>
#include "Mesg.h"
#include "wm_error.h"
/**
 * @brief 
 */
class MsgQueue
{
private:
    mqd_t m_msgQueueFd;
    std::string m_msgQueueName;
    bool m_msgQueueNotInUse;
    long m_msgQueueSize;
    long m_maxMsgInQueue;
    fd_set m_rfds;
    unsigned long m_lastMsgSigName; // for debug

    unsigned long m_receiveMsgCnt; // one msg queue
    unsigned long m_sendMsgCnt; // one msg queue
    static unsigned long gReceiveMsgCnt; // all msg queues
    static unsigned long gSendMsgCnt; // all msg queues
    static bool gUnlimit;

    static std::unordered_map<mqd_t, MsgQueue *> gMap;
    static std::mutex gMapMtx;

public:
    MsgQueue();
    ~MsgQueue();

    /**
     * @brief 
     *
     * @param name
     * @param flag
     * @param mode
     * @param maxmsg
     *
     * @return 
     */
    int Open(const char *name, int flag, mode_t mode, long maxmsg);
    int Open(const char *name, int flag, mode_t mode, struct mq_attr *pattr);

    /**
     * @brief 
     *
     * @return 
     */
    int Close();

    /**
     * @brief 
     *
     * @return 
     */
    int Unlink();

    /**
     * @brief 
     */
    void Clear();

    /**
     * @brief 
     *
     * @param pclsMsg
     * @param s32Prio
     *
     * @return 
     */
    int Send(Mesg *pclsMsg, unsigned int s32Prio = MESG_PRIO_LOW);
    static int Send(mqd_t fd, Mesg *pclsMsg, unsigned int s32Prio = MESG_PRIO_LOW);

    /**
     * @brief 
     *
     * @param pclsMsg
     * @param s32Prio
     * @param isPolling
     *
     * @return 
     */
    int Receive(Mesg *pclsMsg, unsigned int s32Prio = MESG_PRIO_LOW, bool isPolling = true);

    mqd_t MsgQueueFd() const {return m_msgQueueFd;};
    long MsgQueueSize() const {return m_msgQueueSize;};
    long CurMsgsInQueue();
    bool MsgQueueNotInUse() const {return m_msgQueueNotInUse;};
    void MsgQueueNotInUse(bool bVal) {m_msgQueueNotInUse = bVal;};
    unsigned long LastMsgSigName() const {return m_lastMsgSigName;};
    const char *MsgQueueName() const {return m_msgQueueName.c_str();};

    static bool IsBlocking(mqd_t fd);
    static std::string MsgQueueName(mqd_t fd);
    static MsgQueue *MsgQueuePtr(mqd_t fd);

    /**
     * @brief 
     *
     * @param selection
     */
    static void DumpMsgQueueMap(int selection = 0);

    /**
     * @brief 
     *
     * @param fd
     */
    static void DumpMsgQueue(mqd_t fd);

private:
    static void AddMsgQueue(mqd_t fd, MsgQueue *pclsMsgQueue);
    static void DelMsgQueue(mqd_t fd, MsgQueue *pclsMsgQueue);
};

#endif
