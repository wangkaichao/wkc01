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

#define ERR_MSG_RESPONSE_QUEUE_NOT_SET         -1300 // 0xAEC
#define ERR_MSG_SIGNALSIZE_NOT_SET             -1301 // 0xAEB
#define ERR_MSG_COPY_MSG_ALLOC_FAILED          -1302 // 0xAEA

class MsgQueue;

/**
 * @brief 
 */
class SigObj
{
public:
    SigObj() {};
    virtual ~SigObj() {};

    /**
     * @brief 
     *
     * @return 
     */
    virtual SigObj* Clone() = 0;
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
    unsigned long ulSigName;
    std::tuple<REQ_DATA_U, ACK_DATA_U, ACK_DATA_U> tpSigCmd;
    std::tuple<char *, int> tpSigData;
    std::tuple<unsigned long, unsigned long> tpCbId;
    unsigned int u32SeqCnt;
    SigObj *pclsSigObj;
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
    Mesg(unsigned long ulSigName) {Mesg(); mMsg.ulSigName = ulSigName;};
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
    void FreeSignal();

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
    unsigned long SigName() const {return mMsg.ulSigName;};
    void SigName(unsigned long ulSigName) {mMsg.ulSigName = ulSigName;};
    long SigReqLong() const {return std::get<0>(mMsg.tpSigCmd).slData;};
    void SigReqLong(long slData) {std::get<0>(mMsg.tpSigCmd).slData = slData;};

    char *SigDataPtr() const {return std::get<0>(mMsg.tpSigData);};
    int SigDataSize() const {return std::get<1>(mMsg.tpSigData);};
    void SigData(char *ptr, int size) {
        std::get<0>(mMsg.tpSigData) = ptr;
        std::get<1>(mMsg.tpSigData) = size;
    };
    std::tuple<char *, int> SigData() const {return mMsg.tpSigData;};
    unsigned long CbId() const {return std::get<0>(mMsg.tpCbId);};
    void CbId(unsigned long ulCbId) {std::get<0>(mMsg.tpCbId) = ulCbId;};
    //....
    unsigned int SeqCnt() const {return mMsg.u32SeqCnt;};
    void SeqCnt(unsigned int s32SeqCnt) {mMsg.u32SeqCnt = s32SeqCnt;};
    SigObj *SigObjPtr() const {return mMsg.pclsSigObj;};
    void SigObjPtr(SigObj *pclsSigObj) {mMsg.pclsSigObj = pclsSigObj;};
};

#endif
