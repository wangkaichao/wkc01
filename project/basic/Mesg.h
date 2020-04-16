/**
 * @file Mesg.h
 * @brief 
 * @author wangkaichao
 * @version 1.0
 * @date 2020-01-07
 */
#ifndef MESG_H
#define MESG_H

#include <mqueue.h>
#include <tuple>
#include <string.h>

//[0, MQ_PRIO_MAX]
#define MESG_PRIO_HIGH  1
#define MESG_PRIO_LOW   0

class MsgQueue;

/**
 * @brief 
 */
class MsgObj
{
public:
    MsgObj() {};
    virtual ~MsgObj() {};

    /**
     * @brief 
     *
     * @return 
     */
    virtual MsgObj* Clone() = 0;
};

typedef union
{
    long    slData;
    int     s32Data;
    bool    abData[4];
    void *  pData;
} REQ_DATA_U;

typedef union
{
    long    slData;
    int     s32Data;
    void *  pData;
} ACK_DATA_U;

/**
 * @brief 
 */
typedef struct 
{
    unsigned long ulMsgId;
    std::tuple<REQ_DATA_U, ACK_DATA_U> tpMsgCmd;
    std::tuple<char *, int> tpMsgData;
    unsigned long ulCbId;
    unsigned int u32SeqCnt;
    MsgObj *pclsMsgObj;
} MESG_T;

/**
 * @brief 
 */
class Mesg
{
public:
    MESG_T mMsg;

public:
    Mesg() {memset(&mMsg, 0, sizeof(mMsg));};
    Mesg(unsigned long ulMsgId) {Mesg(); mMsg.ulMsgId = ulMsgId;};
    Mesg(const Mesg& clsMsg);
    Mesg& operator=(const Mesg& clsMsg);
    virtual ~Mesg() {};

    /**
     * @brief 
     *
     * @param clsMsg
     *
     * @return 
     */
    int CopyMsg(const Mesg& clsMsg);

    /**
     * @brief 
     */
    void Free();

    /**
     * @brief 
     *
     * @param pclsMsgQueue
     * @param s32Prio
     *
     * @return 
     */
    int SendMsg(MsgQueue *pclsMsgQueue, int s32Prio = MESG_PRIO_LOW);
    int SendMsg(mqd_t fd, int s32Prio = MESG_PRIO_LOW);

    /**
     * @brief 
     *
     * @param pclsMsgQueue
     * @param s32Prio
     * @param isPolling
     *
     * @return 
     */
    int ReceiveMsg(MsgQueue *pclsMsgQueue, int s32Prio = MESG_PRIO_LOW, bool isPolling = true);

    char *MsgAddr() {return (char *)&mMsg;};
    static size_t MsgSize() {return sizeof(MESG_T);};

    // GET/SET
    unsigned long MsgId() const {return mMsg.ulMsgId;};
    void MsgId(unsigned long ulMsgId) {mMsg.ulMsgId = ulMsgId;};
    long MsgReqLong() const {return std::get<0>(mMsg.tpMsgCmd).slData;};
    void MsgReqLong(long slData) {std::get<0>(mMsg.tpMsgCmd).slData = slData;};

    char *MsgDataPtr() const {return std::get<0>(mMsg.tpMsgData);};
    int MsgDataSize() const {return std::get<1>(mMsg.tpMsgData);};
    void MsgData(char *ptr, int size) {
        std::get<0>(mMsg.tpMsgData) = ptr;
        std::get<1>(mMsg.tpMsgData) = size;
    };
    std::tuple<char *, int> MsgData() const {return mMsg.tpMsgData;};
    unsigned long CbId() const {return mMsg.ulCbId;};
    void CbId(unsigned long ulCbId) {mMsg.ulCbId = ulCbId;};
    //....
    unsigned int SeqCnt() const {return mMsg.u32SeqCnt;};
    void SeqCnt(unsigned int s32SeqCnt) {mMsg.u32SeqCnt = s32SeqCnt;};
    MsgObj *MsgObjPtr() const {return mMsg.pclsMsgObj;};
    void MsgObjPtr(MsgObj *pclsMsgObj) {mMsg.pclsMsgObj = pclsMsgObj;};
};

#endif
