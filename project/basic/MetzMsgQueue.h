/**
* @file MetzMsgQueue.h
*
* @brief MetzMsgQueue
*
* @author Peter Wierzba
*
* $Revision: 1.17 $
* $Date: 2011/09/16 13:28:17GMT $
*
* @note Copyright (c) 2009 Metz-Werke GmbH  CoKG \n1
*	All rights reserved
*
* History:
*
* DD.MM.YY Who Ticket  Description
* 12.02.09 PWI 1       First Issue
*/
/* Archiv: $Source: communication/src/MetzMsgQueue.h $ */
/*==========================================================================*/

#ifndef __METZMESSAGEQUEUE_H__
#define __METZMESSAGEQUEUE_H__

/*--------------------------------------------------------------------------*/
/*	INCLUDE FILES							    */
/*--------------------------------------------------------------------------*/

#include <string>
#include <list>
#include <mqueue.h>
#include "MetzMsg.h"
#include "MetzStdDefs.h"	// for Mutex_Type

/*--------------------------------------------------------------------------*/
/*	CONSTANTS							    */
/*--------------------------------------------------------------------------*/

#define MSG_QUEUE_HANDLE_UNDEFINED			-1

// Size of the array used for quick translation of queue handle to queue object address
// It should be as big as the highest queue handle value.
// But it even works if it is too small.
#define MMQ_QUICK_ACCESS_ARRAY_SIZE			250

// ERROR CODES for this unit (range: -1400 .. -1499)

// All error codes have negative values.
// ERR_NO_ERROR is used to indicate "no error" (value = 0)
#define ERR_METZ_MSG_QUEUE_CLOSE_FAILED			-1410 // 0xA7E
#define ERR_METZ_MSG_QUEUE_CLOSE_NO_HANDLE		-1411 // 0xA7D
#define ERR_METZ_MSG_QUEUE_GETATTR_FAILED		-1412 // 0xA7C
#define ERR_METZ_MSG_QUEUE_GETATTR_NO_HANDLE		-1413 // 0xA7B
#define ERR_METZ_MSG_QUEUE_GETNR_FAILED			-1414 // 0xA7A
#define ERR_METZ_MSG_QUEUE_GETNR_NO_HANDLE		-1415 // 0xA79
#define ERR_METZ_MSG_QUEUE_HANDLE_UNDEFINED		-1416 // 0xA78
#define ERR_METZ_MSG_QUEUE_NAME_UNKNOWN			-1417 // 0xA77
#define ERR_METZ_MSG_QUEUE_OPEN_FAILED			-1418 // 0xA76
#define ERR_METZ_MSG_QUEUE_RECEIVE_FAILED		-1419 // 0xA75
#define ERR_METZ_MSG_QUEUE_RECEIVE_NO_HANDLE		-1420 // 0xA74
#define ERR_METZ_MSG_QUEUE_SEND_FAILED			-1421 // 0xA73
#define ERR_METZ_MSG_QUEUE_SEND_NO_HANDLE		-1422 // 0xA72
#define ERR_METZ_MSG_QUEUE_SEND_NO_TARGET_HANDLE	-1423 // 0xA71
#define ERR_METZ_MSG_QUEUE_UNLINK_FAILED		-1424 // 0xA70
#define ERR_METZ_MSG_QUEUE_SELECT_FAILED		-1425 // 0xA6F
#define ERR_METZ_MSG_PTR_IS_NULL			-1426 // 0xA6E
#define ERR_METZ_MSG_QUEUE_SEND_HANDLE_IS_NULL		-1427 // 0xA6D
#define ERR_METZ_MSG_QUEUE_SELECT_BUT_NOT_OUR_QUEUE	-1428 // 0xA6C

/*--------------------------------------------------------------------------*/
/*	TYPE DEFINITIONS						    */
/*--------------------------------------------------------------------------*/

class CMetzMsgQueue;	// Forward declaration

// Structure which holds information about a queue object
typedef struct
{
    CMetzMsgQueue * msgQueuePtr;
    mqd_t	    msgQueueHandle;

} T_GlobalMsgQueueInfo;

/*--------------------------------------------------------------------------*/
/*	CLASS DECLARATIONS						    */
/*--------------------------------------------------------------------------*/

class CMetzMsgQueue
{
 private:
    // --- Member variables

    mqd_t	    m_msgQueueHandle;	// handle returned by mq_open()
    std::string	    m_msgQueueName;	// name of the queue
    int		    m_maxUsedSize;	// max fill level of the queue
    unsigned int  m_nrOfMsgsReadFromQueue; // only for debug
    struct mq_attr mqattr;
    fd_set	    rfds;		// used for select()
    bool	    m_msgQueueNotInUse;	// to ignore errors intentionally
    unsigned long m_lastMsgReadFromQueue; // only for debug

    // Global list of pointers to all active message queues
    static std::list<T_GlobalMsgQueueInfo> globalMsgQueueList;

    // Mutex used for access of globalMsgQueueList
    static Mutex_Type m_metzMsgQueueMutex;
    static bool	m_metzMsgQueueMutexInitialized;
    static bool	m_metzMsgQueueHandlingTerminated; // Workaround for the destructor problem

    // Array used for quick translation of queue handle to queue object address
    static CMetzMsgQueue * m_metzMsgQueueQuickAccessArray[MMQ_QUICK_ACCESS_ARRAY_SIZE];
    static bool	    m_metzMsgQueueQuickAccessArrayInitialized;

    // Just for test
    static int totalNrOfSentMsgs;

 public:

    // --- Member functions

    CMetzMsgQueue();	// Constructor
    ~CMetzMsgQueue();	// Destructor

    // Creates or reopens a message queue with the given name.
    // argOflag determines the type of access used. If O_CREAT is set on
    // argOflag, the argMode argument is taken as a `mode_t', the mode
    // of the created message queue. Argument argMaxNrOfMsgs specifies
    // the maximum number of messages the queue may contain.

    int Open (const char *argName, int argOflag, int argMode, int argMaxNrOfMsgs);

    // Creates or reopens a message queue with the given name.
    // argOflag determines the type of access used. If O_CREAT is set on
    // argOflag, the argMode argument is taken as a `mode_t', the mode
    // of the created message queue, and the argAttrPtr argument is
    // taken as `struct mq_attr *', pointer to message queue
    // attributes.  If the argAttrPtr argument is NULL, default
    // attributes are used.

    int OpenWithAttrStruct (const char *argName, int argOflag, int argMode, struct mq_attr * argAttrPtr);

    // Removes the association between the message object and its
    // message queue.
    // Note: This does NOT remove the messages contained in the
    //	     queue. If the queue is reopened the "old" messages are
    //	     still in the queue.
    //	     Use "Unlink" to remove the queue and all messages.

    int Close (void);

    // Remove message queue completely from the system.
    // Note: This should be used only for special cases, as memory
    // attached to the messages is NOT freed.

    int Unlink (void);

    // Clear queue removes any messages from the queue and frees
    // attached memory. This is the function which should be used in
    // normal cases to empty a queue.

    void ClearQueue (void);

    // Receive a message from a nonblocking message queue. Contents will be
    // copied to Object pointed by argMsgObjPtr.
    // If the queue is empty the function will not return until a messsage arrives.
    // Note: Parameter "argMsgPriority" is obsolete and should not be used.

    int ReceiveFromNonblockingQueue (CMetzMsg * argMsgObjPtr, unsigned int argMsgPriority = M_SQ_PRIO_LOW);

    // Receive a message from the message queue. Contents will be
    // copied to Object pointed by argMsgObjPtr.

    int Receive (CMetzMsg * argMsgObjPtr, unsigned int argMsgPriority = M_SQ_PRIO_LOW);

    // Add message pointed by argMsgObjPtr to message queue.

    int Send (CMetzMsg * argMsgObjPtr, unsigned int argMsgPriority = M_SQ_PRIO_LOW);

    // Add message pointed by argMsgObjPtr to message queue specified via handle.

    static int Send (mqd_t argTargetQueue, CMetzMsg * argMsgObjPtr, unsigned int argMsgPriority = M_SQ_PRIO_LOW);

    // Query status and attributes of message queue.

    int Getattr (struct mq_attr *argStatStructPtr);

    // Returns the number of messages currently in the queue.

    int GetNrOfMsgsInQueue();

    // Returns the handle of the queue.

    mqd_t GetQueueHandle() const { return m_msgQueueHandle; }; // Needed for callback registration

    // To suppress queue full errors in case the queue is not really used

    void SetQueueNotInUse(void) { m_msgQueueNotInUse = true; }

    // Global list of pointers to all active message queues (Just used for debug)
    static void ListGlobalMsgQueueList(int argQueueSelection = 0);
    static std::string GetQueueNameByHandle(mqd_t argMsgQueueHandle);
    static CMetzMsgQueue * GetQueueObjPtrByHandle(mqd_t argMsgQueueHandle);
    static void DumpQueue(mqd_t argMsgQueueHandle);
    static void Terminate(void);

    // Just for test
    static int	GetTotalNrOfSentMsgs(void);
    static void ClearTotalNrOfSentMsgs(void);
    void usleepOnQueue(unsigned long argInterval);
    unsigned int GetLastMsgReadFromQueue(void) { return m_lastMsgReadFromQueue; }


 private:
    static void GetMutex(void);
    void AddQueueToGlobalMsgQueueList(CMetzMsgQueue * argMsgQueuePtr, mqd_t argMsgQueueHandle);
    int RemoveQueueFromGlobalMsgQueueList(CMetzMsgQueue * argMsgQueuePtr, mqd_t argMsgQueueHandle);
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

bool QueueTypeIsBlocking(mqd_t argQueueHandle);

/*--------------------------------------------------------------------------*/

#endif	/* __METZMESSAGEQUEUE_H__ */
/*--------------------------------------------------------------------------*/
/* end of MetzMsgQueue.h */
