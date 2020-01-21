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

// All error codes have negative values.
// ERR_NO_ERROR is used to indicate "no error" (value = 0)
#define ERR_MSG_QUEUE_CLOSE_FAILED              -1410 // 0xA7E
#define ERR_MSG_QUEUE_CLOSE_NO_HANDLE           -1411 // 0xA7D
#define ERR_MSG_QUEUE_GETATTR_FAILED            -1412 // 0xA7C
#define ERR_MSG_QUEUE_GETATTR_NO_HANDLE         -1413 // 0xA7B
#define ERR_MSG_QUEUE_GETNR_FAILED              -1414 // 0xA7A
#define ERR_MSG_QUEUE_GETNR_NO_HANDLE           -1415 // 0xA79
#define ERR_MSG_QUEUE_HANDLE_UNDEFINED          -1416 // 0xA78
#define ERR_MSG_QUEUE_NAME_UNKNOWN              -1417 // 0xA77
#define ERR_MSG_QUEUE_OPEN_FAILED               -1418 // 0xA76
#define ERR_MSG_QUEUE_RECEIVE_FAILED            -1419 // 0xA75
#define ERR_MSG_QUEUE_RECEIVE_NO_HANDLE         -1420 // 0xA74
#define ERR_MSG_QUEUE_SEND_FAILED               -1421 // 0xA73
#define ERR_MSG_QUEUE_SEND_NO_HANDLE            -1422 // 0xA72
#define ERR_MSG_QUEUE_SEND_NO_TARGET_HANDLE     -1423 // 0xA71
#define ERR_MSG_QUEUE_UNLINK_FAILED             -1424 // 0xA70
#define ERR_MSG_QUEUE_SELECT_FAILED             -1425 // 0xA6F
#define ERR_MSG_QUEUE_PTR_IS_NULL               -1426 // 0xA6E
#define ERR_MSG_QUEUE_SEND_HANDLE_IS_NULL       -1427 // 0xA6D
#define ERR_MSG_QUEUE_SELECT_BUT_NOT_OUR_QUEUE  -1428 // 0xA6C

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

    static std::unordered_map<mqd_t, MsgQueue *> gMap;
    static std::mutex gMapMtx;

public:
    MsgQueue():m_msgQueueFd(-1), m_msgQueueName(""), 
        m_msgQueueNotInUse(false), m_msgQueueSize(0), m_maxMsgInQueue(0) {};
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
