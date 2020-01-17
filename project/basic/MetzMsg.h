/**
* @file MetzMsg.h
*
* @brief MetzMsg	Class which represents a Metz-Message.
*
* @author Peter Wierzba
*
* $Revision: 1.21 $
* $Date: 2012/02/06 14:46:27CET $
*
* @note Copyright (c) 2010 Metz-Werke GmbH  CoKG
*	All rights reserved
*
* History:
*
* DD.MM.YY Who		Ticket	Description
* 12.02.09 P. Wierzba		First Issue
*/
/* Archiv: $Source: communication/src/MetzMsg.h $ */
/*==========================================================================*/

#ifndef __METZMSG_H__
#define __METZMSG_H__

/*--------------------------------------------------------------------------*/
/*	INCLUDE FILES							    */
/*--------------------------------------------------------------------------*/

#include <mqueue.h>
#include "wm_log.h"

class CMetzMsgQueue;	    // Forward declaration (instead of #include "MetzMsgQueue.h")
class CMetzMsgSignalObj;    // Forward declaration

/*--------------------------------------------------------------------------*/
/*	CONSTANTS							    */
/*--------------------------------------------------------------------------*/

// Priorities for sending messages
#define M_SQ_PRIO_HIGH  1
#define M_SQ_PRIO_LOW   0

// ERROR CODES for this unit (range: -1300 .. -1309)

// All error codes have negative values.
// ERR_NO_ERROR is used to indicate "no error" (value = 0)

#define ERR_METZ_MSG_RESPONSE_QUEUE_NOT_SET		    -1300 // 0xAEC
#define ERR_METZ_MSG_SIGNALSIZE_NOT_SET			    -1301 // 0xAEB
#define ERR_METZ_MSG_COPY_MSG_ALLOC_FAILED		    -1302 // 0xAEA

/*--------------------------------------------------------------------------*/
/*	TYPE DEFINITIONS						    */
/*--------------------------------------------------------------------------*/

typedef union
{
    long   longData;
    int	   intData;
    void * ptrData;

} M_ResponseInfo;

typedef union
{
    long   longData;
    int	   intData;
    bool   boolData[4];
    void * ptrData;

} M_SignalData;

typedef union
{
    long   longData;		// Just for easy copying the whole union
    char   charData[4];		// 4 bytes to be used
				// -> Byte 0 : Message version info (default: 0)
				// -> Byte 1 : Retry counter (default: 0)
				//             to allow the application to indicate resent messages due to timeout,...
				// -> Byte 2 : RPC sequence ctr (default: 0)
				// -> Byte 3 : unused
} M_MetzMsgAuxData1;

enum M_MetzMsgAuxData1Index
{
    METZMSG_AUXDATA_INDEX_MSGVERSION = 0,
    METZMSG_AUXDATA_INDEX_RETRYCTR   = 1,
    METZMSG_AUXDATA_INDEX_RPCSEQCTR  = 2
};


typedef struct
{
    unsigned long SignalName;		// Identifies the message
    M_SignalData   SignalData;		// Optional additional data
					// May be used if only one data item is necessary
    mqd_t	   ResponseQueueHandle; // Handle of the queue receiving the response.
					// Optional queue handle where the response message has
					// to be sent.
    M_ResponseInfo ResponseInfo1;	// For free use by the application
					// If the receiver of this message sends a response,
					// then this field has to be contained in the response
					// message unchanged.
    M_ResponseInfo ResponseInfo2;	// Just a second separate response info
					// For free use by the application
					// If the receiver of this message sends a response,
					// then this field has to be contained in the response
					// message unchanged.
    unsigned long callbackId;		// Used by the callback functions.
					// In case of a callback message this field will contain
					// the value provided when registering for the callback.
    unsigned long callbackId2;		// Just a second separate callbackId.
					// Used by the callback functions.
					// In case of a callback message this field will contain
					// the value provided when registering for the callback.
					// Used by RPC functions as a sequence counter to check for
					// messages lost on the interface.
    M_MetzMsgAuxData1 auxData1;		// Message version info, retry counter.
					// For free use by the application
    unsigned int seqCtr;		// Sequence counter. Field for optional use by the application.
    int		   signalSize;		// Size (in bytes) of the data structure pSignal points to.
    void *	   pSignal;		// Pointer to an optional data structure
					// containing more data. This data structure has
					// to be released by the receiver of the message.
    CMetzMsgSignalObj * pSignalObj;     // Pointer to an optional data object
					// containing more data. This data object has
					// to be released by the receiver of the message.
} MetzMessage;

/*--------------------------------------------------------------------------*/
/*	CLASS DECLARATIONS						    */
/*--------------------------------------------------------------------------*/

class CMetzMsgSignalObj
{
    // --- Member variables
 public:

private:

    // --- Member functions

 public:
    CMetzMsgSignalObj(){};					     // Constructor
    virtual ~CMetzMsgSignalObj(){};				     // Destructor
    virtual CMetzMsgSignalObj * cloneSignalObj() = 0;		     // Clone function

//  CMetzMsgSignalObj(const CMetzMsgSignalObj &origObj);	     // Copy Constructor
//  CMetzMsgSignalObj & operator=(const CMetzMsgSignalObj &origObj); // Assignment Operator
};
    
/*--------------------------------------------------------------------------*/

class CMetzMsg
{
    // --- Member variables
 public:
    MetzMessage data;	// The structure which is really sent to the POSIX queue.
			// ^^ We should provide get/set-functions to access it
 private:

    // --- Member functions

 public:
    CMetzMsg();				   // Constructor
    CMetzMsg(unsigned long argSignalName);	   // Constructor
    virtual ~CMetzMsg();			   // Destructor
    CMetzMsg(const CMetzMsg &origMsg);		   // Copy Constructor
    CMetzMsg & operator=(const CMetzMsg &origMsg); // Assignment Operator

    MetzMessage * GetMsgDataAddr() { return &data; }; // Needed by MetzMsgQueue
    
    int SendMsgIndirect(int argPrio = M_SQ_PRIO_LOW);
    int SendMsg(CMetzMsgQueue *argMsgQueue, int argPrio = M_SQ_PRIO_LOW);
    int SendMsg(mqd_t argMsgQueueHandle, int argPrio = M_SQ_PRIO_LOW);

    // Note: Parameter "argPrio" is obsolete and should not be used.
    int ReceiveMsg(CMetzMsgQueue *argMsgQueue, int argPrio = M_SQ_PRIO_LOW);

    // Note: Parameter "argPrio" is obsolete and should not be used.
    int ReceiveMsgFromNonblockingQueue(CMetzMsgQueue *argMsgQueue, int argPrio = M_SQ_PRIO_LOW);

    int	 CopyMsg(const CMetzMsg &origMsg);

    // Next has to be static because of CMetzMsgQueue::OpenQueue.
    static int GetMsgDataSize() { return (sizeof(MetzMessage)); };

    // SignalName

    unsigned long GetSignalName() const { return data.SignalName; };
    void SetSignalName(unsigned long argSignalName) { data.SignalName = argSignalName; };

    // SignalData

    long GetSignalDataLong() const { return data.SignalData.longData; };
    int	 GetSignalDataInt() const { return data.SignalData.intData; };
    bool GetSignalDataBool() const { return data.SignalData.boolData[0]; };
    bool GetSignalDataBool2() const { return data.SignalData.boolData[1]; };
    bool GetSignalDataBool3() const { return data.SignalData.boolData[2]; };
    bool GetSignalDataBool4() const { return data.SignalData.boolData[3]; };
    void * GetSignalDataVoidPtr() const { return data.SignalData.ptrData; };
    char GetMsgVersion() const { return data.auxData1.charData[METZMSG_AUXDATA_INDEX_MSGVERSION]; };
    char GetMsgRetryCtr() const { return data.auxData1.charData[METZMSG_AUXDATA_INDEX_RETRYCTR]; };
    char GetRpcSeqCtr() const { return data.auxData1.charData[METZMSG_AUXDATA_INDEX_RPCSEQCTR]; };

    void SetSignalDataLong(long argLongData ) { data.SignalData.longData = argLongData; };
    void SetSignalDataInt(int argIntData ) { data.SignalData.intData = argIntData; };
    void SetSignalDataBool(bool argBoolData )  { data.SignalData.boolData[0] = argBoolData; };
    void SetSignalDataBool2(bool argBoolData ) { data.SignalData.boolData[1] = argBoolData; };
    void SetSignalDataBool3(bool argBoolData ) { data.SignalData.boolData[2] = argBoolData; };
    void SetSignalDataBool4(bool argBoolData ) { data.SignalData.boolData[3] = argBoolData; };
    void SetSignalDataVoidPtr(void * argVoidPtrData ) { data.SignalData.ptrData = argVoidPtrData; };
    void SetMsgVersion(bool argCharData) { data.auxData1.charData[METZMSG_AUXDATA_INDEX_MSGVERSION] = argCharData; };
    void SetMsgRetryCtr(bool argCharData) { data.auxData1.charData[METZMSG_AUXDATA_INDEX_RETRYCTR] = argCharData; };
    void SetMsgRpcSeqCtr(bool argCharData) { data.auxData1.charData[METZMSG_AUXDATA_INDEX_RPCSEQCTR] = argCharData; };

    // ResponseQueue

    void SetResponseQueueHandle(mqd_t argResponseQueueHandle) { data.ResponseQueueHandle = argResponseQueueHandle; };

    // ResponseInfo

    M_ResponseInfo GetResponseInfo1() const { return data.ResponseInfo1; };
    int	 GetResponseInfo1Int() const { return data.ResponseInfo1.intData; };
    long GetResponseInfo1Long() const { return data.ResponseInfo1.longData; };
    void * GetResponseInfo1Ptr() const { return data.ResponseInfo1.ptrData; };

    M_ResponseInfo GetResponseInfo2() const { return data.ResponseInfo2; };
    int	 GetResponseInfo2Int() const { return data.ResponseInfo2.intData; };
    long GetResponseInfo2Long() const { return data.ResponseInfo2.longData; };
    void * GetResponseInfo2Ptr() const { return data.ResponseInfo2.ptrData; };

    void SetResponseInfo1(M_ResponseInfo argResponseInfo1) { data.ResponseInfo1 = argResponseInfo1; };
    void SetResponseInfo1Int(int argResponseInfo1) { data.ResponseInfo1.intData = argResponseInfo1; };
    void SetResponseInfo1Long(long argResponseInfo1) { data.ResponseInfo1.longData = argResponseInfo1; };
    void SetResponseInfo1Ptr(void * argResponseInfo1) { data.ResponseInfo1.ptrData = argResponseInfo1; };

    void SetResponseInfo2(M_ResponseInfo argResponseInfo2) { data.ResponseInfo2 = argResponseInfo2; };
    void SetResponseInfo2Int(int argResponseInfo2) { data.ResponseInfo2.intData = argResponseInfo2; };
    void SetResponseInfo2Long(long argResponseInfo2) { data.ResponseInfo2.longData = argResponseInfo2; };
    void SetResponseInfo2Ptr(void * argResponseInfo2) { data.ResponseInfo2.ptrData = argResponseInfo2; };

    // CallbackId

    unsigned long GetCallbackId() const { return data.callbackId; };
    unsigned long GetCallbackId2() const { return data.callbackId2; };
    void SetCallbackId(unsigned long argCallbackId ) { data.callbackId = argCallbackId; };
    void SetCallbackId2(unsigned long argCallbackId2 ) { data.callbackId2 = argCallbackId2; };

    // Sequence counter

    unsigned int GetSeqCtr() const { return data.seqCtr; };
    void SetSeqCtr(unsigned int argSeqCtr ) { data.seqCtr = argSeqCtr; };

    // Additional Expiry Info for timeout timers (reusing field "CallbackId")

    unsigned long GetAddExpiryInfo() const { return data.callbackId; };
    void SetAddExpiryInfo(unsigned long argAddExpiryInfo ) { data.callbackId = argAddExpiryInfo; };

    // Attached Signal (SignalPtr and SignalSize)

    void * GetSignalPtr() const { return data.pSignal; };
//    void SetSignalPtr(void * argSignalPtr, int argSignalSize = 0) { data.pSignal = argSignalPtr; data.signalSize = argSignalSize; };
    void SetSignalPtr(void * argSignalPtr, int argSignalSize) { data.pSignal = argSignalPtr; data.signalSize = argSignalSize; };    
    int	 GetSignalSize() const { return data.signalSize; };
    CMetzMsgSignalObj * GetSignalObjPtr() const { return data.pSignalObj; };
    void SetSignalObjPtr(CMetzMsgSignalObj * argSignalObjPtr) { data.pSignalObj = argSignalObjPtr; };    
    void FreeSignal();

 private:

};

/*--------------------------------------------------------------------------*/
/*	GLOBAL DATA							    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*	INTERNAL DATA							    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*	FUNCTION PROTOTYPES OF GLOBAL FUNCTIONS				    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
#endif	/* __METZMSG_H__ */
/*--------------------------------------------------------------------------*/
/* end of MetzMsg.h */
