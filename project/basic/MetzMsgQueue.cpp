/**
* @file	 MetzMsgQueue.cpp
*
* @brief  MetzMsgQueue class for sending and receiving Metz messages.
*
* @author  Peter Wierzba
*
* $Revision: 1.22 $
* $Date: 2012/06/01 14:34:48GMT $
*
* @note Copyright (c) 2009 Metz-Werke GmbH  CoKG \n
*	All rights reserved
*
* This file provides a class for sending and receiving Metz messages via
* POSIX queues. Information about the queues is held in a list to be shown
* by the mshell command "m_queuelist".
*
* @history
*
* DD.MM.YY Who Ticket  Description
* 12.02.09 PWI 1       First Issue
*/
/* Archiv: $Source: communication/src/MetzMsgQueue.cpp $ */
/*==========================================================================*/

/*--------------------------------------------------------------------------*/
/*	INCLUDE FILES							    */
/*--------------------------------------------------------------------------*/

#include <syscall.h>	// for syscall(SYS_gettid)
#include <unistd.h>	//sleep, usleep, getpid, sync
#include <errno.h>
#include <string.h>
#include "MetzMsgQueue.h"
//#include "MetzTrace.h"
//#include "MetzTraceUnitsGeneral.h"
#include "SignalNo.h"

/*--------------------------------------------------------------------------*/
/*	INTERNAL CONSTANTS						    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*	INTERNAL TYPE DEFINITIONS					    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*	GLOBAL DATA							    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*	INTERNAL DATA							    */
/*--------------------------------------------------------------------------*/

// Create instances of static data

std::list<T_GlobalMsgQueueInfo> CMetzMsgQueue::globalMsgQueueList;
pthread_mutex_t CMetzMsgQueue::m_metzMsgQueueMutex;
bool	   CMetzMsgQueue::m_metzMsgQueueMutexInitialized = false;
bool	   CMetzMsgQueue::m_metzMsgQueueQuickAccessArrayInitialized = false;
bool	   CMetzMsgQueue::m_metzMsgQueueHandlingTerminated = false;
CMetzMsgQueue * CMetzMsgQueue::m_metzMsgQueueQuickAccessArray[MMQ_QUICK_ACCESS_ARRAY_SIZE];

// Just for test
int CMetzMsgQueue::totalNrOfSentMsgs = 0;

/*--------------------------------------------------------------------------*/
/*	FUNCTION PROTOTYPES OF INTERNAL FUNCTIONS			    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*	INTERNAL FUNCTIONS						    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*	GLOBAL FUNCTIONS						    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/**
@brief	 QueueTypeIsBlocking
@param	 argQueueHandle		Handle of the queue
@return	 true	Queue is blocking
	 false	Queue is non-blocking
@note	 status: tested

	 Returns if a queue has been opend for blocking or non-blocking access.
*/
/*--------------------------------------------------------------------------*/

bool QueueTypeIsBlocking(mqd_t argQueueHandle)
{
    struct mq_attr lMqattr;
    int rc = 0;

    rc = mq_getattr(argQueueHandle, &lMqattr);
    if (rc != 0)
    {
	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR,
		 "Error getting attributes for queue handle %d\n", argQueueHandle);
    }
    if ((lMqattr.mq_flags & O_NONBLOCK) == 0)
    {
	return true;
    }
    else
    {
	return false;
    }
}

/*--------------------------------------------------------------------------*/
/**
@brief	 GetMutex
@param	 none
@return	 none
@note	 status: tested

	 Requests the mute. The mutex is automatically initialized if not yet done.
*/
/*--------------------------------------------------------------------------*/

void CMetzMsgQueue::GetMutex(void)
{
    if (m_metzMsgQueueMutexInitialized == false)
    {
	m_metzMsgQueueMutexInitialized = true;
	InitializeMutex(&m_metzMsgQueueMutex);
    }
    RequestMutex(&m_metzMsgQueueMutex);
}

/*--------------------------------------------------------------------------*/
/**
@brief	 AddQueueToGlobalMsgQueueList
@param	 argMsgQueueObjPtr	Ptr to the queue object
@param	 argMsgQueueHandle	Handle of the queue
@return	 none
@note	 status: tested

	 Adds an entry for this queue to the global message queue list.
	 The global message queue list is mainly used for the "m_queuelist"
	 mshell command.
*/
/*--------------------------------------------------------------------------*/

void CMetzMsgQueue::AddQueueToGlobalMsgQueueList(CMetzMsgQueue * argMsgQueueObjPtr, mqd_t argMsgQueueHandle)
{
    T_GlobalMsgQueueInfo msgQueueInfo = { argMsgQueueObjPtr, argMsgQueueHandle };

    GetMutex();

    // Insert at the beginning of the list
    globalMsgQueueList.push_front(msgQueueInfo);

    // We use an additional array for quicker access
    if (m_metzMsgQueueQuickAccessArrayInitialized == false)
    {
	// Init the quick access array
	for (int i=0; i < MMQ_QUICK_ACCESS_ARRAY_SIZE; i++)
	{
	    m_metzMsgQueueQuickAccessArray[i] = 0;
	}
	m_metzMsgQueueQuickAccessArrayInitialized = true;
    }

    if (argMsgQueueHandle < MMQ_QUICK_ACCESS_ARRAY_SIZE)
    {
	m_metzMsgQueueQuickAccessArray[argMsgQueueHandle] = argMsgQueueObjPtr;
    }

    ReleaseMutex(&m_metzMsgQueueMutex);

    // It is intended, that this is done after releasing the mutex
    if (argMsgQueueHandle >= MMQ_QUICK_ACCESS_ARRAY_SIZE)
    {
	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_WARNING,
		 "CMetzMsgQueue::AddQueueToGlobalMsgQueueList - Quick access array too small - %d instead of %d\n",
		 MMQ_QUICK_ACCESS_ARRAY_SIZE, argMsgQueueHandle);
    }
}

/*--------------------------------------------------------------------------*/
/**
@brief	 RemoveQueueFromGlobalMsgQueueList
@param	 argMsgQueuePtr
@param	 argMsgQueueHandle
@return	 error code	    (in case of success "ERR_NO_ERROR")
@note	 status: tested

	 Removes the entry for this queue from the global message queue list.
*/
/*--------------------------------------------------------------------------*/

int CMetzMsgQueue::RemoveQueueFromGlobalMsgQueueList(CMetzMsgQueue * argMsgQueueObjPtr, mqd_t argMsgQueueHandle)
{
    std::list<T_GlobalMsgQueueInfo>::iterator it;

    if (m_metzMsgQueueHandlingTerminated == false) // Only if not disabled during program termination
    {
	GetMutex();

	// Search the element containing the function reference
	for (it = CMetzMsgQueue::globalMsgQueueList.begin(); it != CMetzMsgQueue::globalMsgQueueList.end(); it++)
	{
	    if (it->msgQueuePtr == argMsgQueueObjPtr)
	    {
		// As the element is found, remove it from the list and update the iterator
		it = CMetzMsgQueue::globalMsgQueueList.erase(it);
		// We removed it, so the job is done
		break;
	    }
	}

	// Clear also entry in the quick access array
	if (argMsgQueueHandle < MMQ_QUICK_ACCESS_ARRAY_SIZE)
	{
	    m_metzMsgQueueQuickAccessArray[argMsgQueueHandle] = 0;
	}

	ReleaseMutex(&m_metzMsgQueueMutex);
    }
    return ERR_NO_ERROR;
}


/*--------------------------------------------------------------------------*/
/**
@brief	 GetQueueNameByHandle
@param	 argMsgQueueHandle	Handle of the queue
@return	 name			Name of queue as a std::string.
@note	 status: tested

	 Returns the queue name as a string for a given queue handle.
*/
/*--------------------------------------------------------------------------*/

std::string CMetzMsgQueue::GetQueueNameByHandle(mqd_t argMsgQueueHandle)
{
    std::string retString= "Unknown";
    std::list<T_GlobalMsgQueueInfo>::iterator it;

    GetMutex();

    // Search the element containing the function reference
    for (it = CMetzMsgQueue::globalMsgQueueList.begin(); it != CMetzMsgQueue::globalMsgQueueList.end(); it++)
    {
	if (it->msgQueueHandle == argMsgQueueHandle)
	{
	    // The element is found
	    retString = it->msgQueuePtr->m_msgQueueName;
	    break;
	}
    }
    ReleaseMutex(&m_metzMsgQueueMutex);
    return retString;
}

/*--------------------------------------------------------------------------*/
/**
@brief	 GetQueueObjPtrByHandle
@param	 argMsgQueueHandle	Handle of the queue
@return	 ptr			Ptr to the queue object
@note	 status: tested

	 Returns a pointer to the queue object for a given queue handle.
*/
/*--------------------------------------------------------------------------*/

CMetzMsgQueue * CMetzMsgQueue::GetQueueObjPtrByHandle(mqd_t argMsgQueueHandle)
{
    std::list<T_GlobalMsgQueueInfo>::iterator it;
    CMetzMsgQueue * lMsgQueueObjPtr = 0;

    GetMutex();

    if (argMsgQueueHandle < MMQ_QUICK_ACCESS_ARRAY_SIZE)
    {
	// Data is available in the quick access array
	lMsgQueueObjPtr = m_metzMsgQueueQuickAccessArray[argMsgQueueHandle];
    }
    else
    {
	// Element containing the queue handle has to be searched in the list
	for (it = CMetzMsgQueue::globalMsgQueueList.begin(); it != CMetzMsgQueue::globalMsgQueueList.end(); it++)
	{
	    if (it->msgQueueHandle == argMsgQueueHandle)
	    {
		// The element is found
		lMsgQueueObjPtr = it->msgQueuePtr;
		break;
	    }
	}
    }
    ReleaseMutex(&m_metzMsgQueueMutex);
    return lMsgQueueObjPtr;
}

/*--------------------------------------------------------------------------*/
/**
@brief	 ListGlobalMsgQueueList
@param	 none
@return	 none
@note	 status: tested

	 Displayes a list of all message queues with some additional info.
	 Used for the corresponding mshell command.
*/
/*--------------------------------------------------------------------------*/

void CMetzMsgQueue::ListGlobalMsgQueueList(int argQueueSelection)
{
    std::list<T_GlobalMsgQueueInfo>::iterator it;
    struct mq_attr lMqattr;
    int rc = 0;
    mqd_t lQueueHandle;
    int lQueueMaxSize;
    int lQueueCurrSize;

    const char *lBlockingTypePtr = "UNKNOWN";

    GetMutex();

    MPRINT("List of queues\n");
    MPRINT("handle | size | max. used | cur. used | block | last msg | nr read | name\n");
    MPRINT("-------+------+-----------+-----------+-------+----------+---------+------------------------------------\n");

    // Go through the list and print the elements
    for (it = CMetzMsgQueue::globalMsgQueueList.begin(); it != CMetzMsgQueue::globalMsgQueueList.end(); it++)
    {
	lQueueHandle = it->msgQueueHandle;
	rc = mq_getattr(lQueueHandle, &lMqattr);
	if (rc == 0)
	{
	    lQueueMaxSize  = lMqattr.mq_maxmsg;
	    lQueueCurrSize = lMqattr.mq_curmsgs;
	    if ((lMqattr.mq_flags & O_NONBLOCK) == 0)
	    {
		lBlockingTypePtr = "BLOCK";
	    }
	    else
	    {
		lBlockingTypePtr = "NONBL";
	    }
	}
	else
	{
	    MPRINT("Error getting attributes for queue handle %d \n", lQueueHandle);
	    lQueueMaxSize = 0;
	    lQueueCurrSize = 0;
	    lBlockingTypePtr = "UNKWN";
	}
	if (   (argQueueSelection == 0)			   // list all
            || (argQueueSelection == (int) lQueueHandle)   // list specific
           )
	{
	    MPRINT("%6d | %4d | %9d | %9d | %s | %8u | %7u | %s\n", lQueueHandle, lQueueMaxSize, it->msgQueuePtr->m_maxUsedSize, lQueueCurrSize, lBlockingTypePtr, it->msgQueuePtr->GetLastMsgReadFromQueue(), it->msgQueuePtr->m_nrOfMsgsReadFromQueue, it->msgQueuePtr->m_msgQueueName.c_str());
	}
    }
    MPRINT("-------+------+-----------+-----------+-------+----------+---------+------------------------------------\n");
    ReleaseMutex(&m_metzMsgQueueMutex);
}

/*--------------------------------------------------------------------------*/
/*	PUBLIC CLASSES							    */
/*--------------------------------------------------------------------------*/

/*==========================================================================*/
/* CMetzMsgQueue							    */
/*==========================================================================*/

CMetzMsgQueue::CMetzMsgQueue()		// Constructor
{
    m_msgQueueName = "";			   // Clear name of queue
    m_msgQueueHandle = MSG_QUEUE_HANDLE_UNDEFINED; // Clear handle of queue
    m_maxUsedSize = 0;
    m_msgQueueNotInUse = false;
    m_lastMsgReadFromQueue = 0;
    m_nrOfMsgsReadFromQueue = 0;
}

/*--------------------------------------------------------------------------*/

CMetzMsgQueue::~CMetzMsgQueue()		// Destructor
{
    Unlink();	// Remove all messages from the queue and remove the queue.
}

/*--------------------------------------------------------------------------*/
/**
@brief	 Open
@param	 argName	Name of the queue
@param	 argOflag	Bitwise OR of flags (e.g. O_CREAT, O_RDWR, O_NONBLOCK)
@param	 argMode	Mode
@param	 argMaxNrOfMsgs Max number of messages the queue can hold
@return	 error code	    (in case of success "ERR_NO_ERRROR")
@note	 status: tested

	 Opens a message queue.
	 First of all the queue is removed from linux, if it is already existing.
*/
/*--------------------------------------------------------------------------*/

int CMetzMsgQueue::Open(const char *argName, int argOflag, int argMode, int argMaxNrOfMsgs)
{
    int rc = ERR_NO_ERROR;

    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_DEBUG, "CMetzMsgQueue::Open: %s\n", argName);

    // First of all lets remove the queue from the system, if it is already existing.
    mq_unlink(argName);

    // Store name of queue
    m_msgQueueName = argName;

    m_maxUsedSize = 0;

    // Fill attribute structure
    mqattr.mq_maxmsg = argMaxNrOfMsgs;
    mqattr.mq_msgsize = CMetzMsg::GetMsgDataSize();
    mqattr.mq_flags = 0;
    mqattr.mq_curmsgs = 0;
    mqattr.__pad[0] = 0;
    mqattr.__pad[1] = 0;
    mqattr.__pad[2] = 0;
    mqattr.__pad[3] = 0;

    // Open queue
    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_DEBUG, "CMetzMsgQueue:: Opening queue: %s\n", m_msgQueueName.c_str());

    m_msgQueueHandle = mq_open(m_msgQueueName.c_str(),argOflag,argMode,&mqattr);
    // Upon successful completion,  mq_open returns a message queue
    // descriptor. Otherwise, the function returns -1 and sets
    // errno to indicate the error.

    if (m_msgQueueHandle == -1)
    {
	rc = ERR_METZ_MSG_QUEUE_OPEN_FAILED;
	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::Open queue FAILED - errno = %d\n", errno);
    }
    else
    {
	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_DEBUG, "CMetzMsgQueue::Open queue successful - Result: %d\n", m_msgQueueHandle);
	AddQueueToGlobalMsgQueueList(this, m_msgQueueHandle);

	// Set parameters for select()
	FD_ZERO(&rfds);			    // reset data set
	FD_SET(m_msgQueueHandle, &rfds);    // Add the queue to the data set
    }

    return rc;
}

/*--------------------------------------------------------------------------*/
/**
@brief	 OpenWithAttrStruct
@param	 argName	Name of the queue
@param	 argOflag	Bitwise OR of flags (e.g. O_CREAT, O_RDWR, O_NONBLOCK)
@param	 argMode	Mode
@param	 argAttrPtr	Pointer to an attribute structure
@return	 error code	    (in case of success "ERR_NO_ERRROR")
@note	 status: tested

	 Opens a message queue using an additional attribute structure.
	 First of all the queue is removed from linux, if it is already existing.
*/
/*--------------------------------------------------------------------------*/

int CMetzMsgQueue::OpenWithAttrStruct(const char *argName, int argOflag, int argMode, struct mq_attr * argAttrPtr)
{
    int rc = ERR_NO_ERROR;

    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_DEBUG, "CMetzMsgQueue::OpenWithAttrStruct: %s, %d\n", argName, strlen(argName));

    // First of all lets remove the queue from the system, if it is already existing.
    mq_unlink(argName);

    // Store name of queue
    m_msgQueueName = argName;

    m_maxUsedSize = 0;

    // Set the correct message size in structure in any case
    argAttrPtr->mq_msgsize = CMetzMsg::GetMsgDataSize();

    // Open queue
    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_DEBUG, "CMetzMsgQueue::OpenWithAttrStruct Opening queue: %s\n", m_msgQueueName.c_str());

    m_msgQueueHandle = mq_open(m_msgQueueName.c_str(),argOflag,argMode,argAttrPtr);
    // Upon successful completion,  mq_open returns a message queue
    // descriptor. Otherwise, the function returns -1 and sets
    // errno to indicate the error.

    if (m_msgQueueHandle == -1)
    {
	rc = ERR_METZ_MSG_QUEUE_OPEN_FAILED;
	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::OpenWithAttrStruct Open queue FAILED - errno = %d\n", errno);
    }
    else
    {
	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_DEBUG, "CMetzMsgQueue::OpenWithAttrStruct Open queue successful - Result: %d\n", m_msgQueueHandle);
	AddQueueToGlobalMsgQueueList(this, m_msgQueueHandle);

	// Set parameters for select()
	FD_ZERO(&rfds);			    // reset data set
	FD_SET(m_msgQueueHandle, &rfds);    // Add the queue to the data set
    }

    return rc;
}

/*--------------------------------------------------------------------------*/
/**
@brief	 Close
@param	 none
@return	 error code	    (in case of success "ERR_NO_ERRROR")
@note	 status: tested

	 Closes a message queue. Removes it from the global message queue list.
*/
/*--------------------------------------------------------------------------*/

int CMetzMsgQueue::Close(void)
{
    int rc = ERR_NO_ERROR;

    if (m_msgQueueHandle == MSG_QUEUE_HANDLE_UNDEFINED)
    {
	rc = ERR_METZ_MSG_QUEUE_CLOSE_NO_HANDLE;
	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::Close queue FAILED - No handle given\n");
    }
    else
    {
	int rc2 = mq_close(m_msgQueueHandle);

	if (rc2 != 0)
	{
	    // Close failed
	    rc = ERR_METZ_MSG_QUEUE_CLOSE_FAILED;
	    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::Close queue FAILED - Errno = %d\n", errno);
	}
	else
	{
	    RemoveQueueFromGlobalMsgQueueList(this, m_msgQueueHandle);
	}

    }
    return rc;
}

/*--------------------------------------------------------------------------*/
/**
@brief	 Getattr
@param	 argStatStructPtr   Ptr to a structure receiving the attribute data
@return	 error code	    (in case of success "ERR_NO_ERRROR")
@note	 status: tested

	 Fills a provided structure with the attributes of the queue.
*/
/*--------------------------------------------------------------------------*/

int CMetzMsgQueue::Getattr(struct mq_attr *argStatStructPtr)
{
    int rc = ERR_NO_ERROR;

    if (m_msgQueueHandle == MSG_QUEUE_HANDLE_UNDEFINED)
    {
	rc = ERR_METZ_MSG_QUEUE_GETATTR_NO_HANDLE;
	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::Getattr FAILED - No handle given\n");

    }
    else
    {
	int rc2 = mq_getattr(m_msgQueueHandle, argStatStructPtr);
	// Upon successful completion, the mq_getattr() function returns zero.
	// Otherwise, the function returns -1 and sets errno to indicate the error.
	if (rc2 != 0)
	{
	    // Getattr failed
	    rc = ERR_METZ_MSG_QUEUE_GETATTR_FAILED;
	    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::Getattr FAILED - errno = %d\n", errno);
	}
    }
    return rc;
}

/*--------------------------------------------------------------------------*/
/**
@brief	 ClearQueue
@param	 none
@return	 none
@note	 status: tested

	 Removes all messages from a queue and frees the attached memory.
*/
/*--------------------------------------------------------------------------*/

void CMetzMsgQueue::ClearQueue(void)
{
    int rc_size = 0;
    CMetzMsg receivedMsg;

    do
    {
	// Read a msg
	rc_size = mq_receive(m_msgQueueHandle, (char *) receivedMsg.GetMsgDataAddr(), CMetzMsg::GetMsgDataSize(), NULL);

	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_INFO,
		 "CMetzMsgQueue::ClearQueue - Msg read from queue - size: %d  errno=%d\n",
		 rc_size, errno);
	if (rc_size > 0)
	{
	    receivedMsg.FreeSignal();
	}
    }
    while (rc_size > 0);
}

/*--------------------------------------------------------------------------*/
/**
@brief	 DumpQueue
@param	 argMsgQueueHandle	Handle of the queue
@return	 none
@note	 status: tested

	 Removes all messages from a queue(!), displays the signal name and frees the attached memory.
	 ONLY USED FOR DEBUGGING VIA MSHELL COMMAND.
*/
/*--------------------------------------------------------------------------*/

void CMetzMsgQueue::DumpQueue(mqd_t argMsgQueueHandle)
{
    int rc_size = 0;
    unsigned int msgNr = 0;
    CMetzMsg receivedMsg;

    do
    {
	// Read a msg
	rc_size = mq_receive(argMsgQueueHandle, (char *) receivedMsg.GetMsgDataAddr(), CMetzMsg::GetMsgDataSize(), NULL);
	if (rc_size > 0)
	{
	   msgNr++;
	   MPRINT("CMetzMsgQueue::DumpQueue - Msg read from queue - nr: %d  signalname: %ld\n",
		   msgNr, receivedMsg.GetSignalName());
	    receivedMsg.FreeSignal();
	}
    }
    while (rc_size > 0);
}

/*--------------------------------------------------------------------------*/
/**
@brief	 Unlink
@param	 none
@return	 error code	    (in case of success "ERR_NO_ERRROR")
@note	 status: tested

	 Remove message queue from system. The queue will be cleared first.
*/
/*--------------------------------------------------------------------------*/

int CMetzMsgQueue::Unlink(void)
{
    int rc = ERR_NO_ERROR;
    if (m_msgQueueName.empty())
    {
	// No queue name is known, so no unlink is possible
	rc = ERR_METZ_MSG_QUEUE_NAME_UNKNOWN;
    }
    else
    {
	// Remove all messages from the queue
	ClearQueue();

	// Close queue and remove it from the global list
	Close();

	// Remove the queue from linux
	int rc2 = mq_unlink(m_msgQueueName.c_str());
	if (rc2 != 0)
	{
	    // Unlink failed
	    rc = ERR_METZ_MSG_QUEUE_UNLINK_FAILED;
	    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::Unlink FAILED - Errno = %d\n", errno);
	}
    }
    return rc;
}

/*--------------------------------------------------------------------------*/
/**
@brief	 ReceiveFromNonblockingQueue
@param	 argMsgObjPtr		Ptr to a message object
@param	 argMsgPriority		(not used, obsolete)
@return	 error status
@note	 status: tested

	 Receives the oldest message with the highest priority from the message queue.
	 Waits until a message arrives, even if the queue is non-blocking.
*/
/*--------------------------------------------------------------------------*/

int CMetzMsgQueue::ReceiveFromNonblockingQueue(CMetzMsg * argMsgObjPtr, unsigned int argMsgPriority)
{
    int rc = ERR_NO_ERROR;
    int rc_size = 0;

    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_DEBUG, "CMetzMsgQueue::ReceiveFromNonblockingQueue called - handle=%d msg_obj_ptr=%p\n",
						   m_msgQueueHandle, argMsgObjPtr);

    m_msgQueueNotInUse = false;		// Reset it, as we are using the queue

    if (m_msgQueueHandle == MSG_QUEUE_HANDLE_UNDEFINED)
    {
	rc = ERR_METZ_MSG_QUEUE_RECEIVE_NO_HANDLE;
	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::ReceiveFromNonblockingQueue FAILED - No handle given\n");
    }
    else if (argMsgObjPtr == 0)
    {
	rc = ERR_METZ_MSG_PTR_IS_NULL;
	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::ReceiveFromNonblockingQueue FAILED - handle=%d argMsgObjPtr=0\n", m_msgQueueHandle);
    }
    else
    {
	// Wait on the non-blocking queue
	int rc_select = select(m_msgQueueHandle+1, &rfds, NULL, NULL, NULL);

	if (rc_select >= 0)
	{
	    // Check if select indicates that it was our queue
	    // (This should be the case,as there is only one queue)
	    if(FD_ISSET(m_msgQueueHandle,&rfds))
	    {
		// Data is available in the queue, so read a msg

		rc_size = mq_receive(m_msgQueueHandle, (char *) argMsgObjPtr->GetMsgDataAddr(), CMetzMsg::GetMsgDataSize(), NULL);
		// Upon successful completion, mq_receive() returns the length of the
		// selected message in bytes and the message is removed from the queue.
		// Otherwise, no message is removed from the queue, the function returns
		// a value of -1, and sets errno to indicate the error.

		//^^ Should we check if the size matches with the message size, or is at least the message size ?!
		if (rc_size > 0)
		{
		    m_lastMsgReadFromQueue = ((CMetzMsg *) argMsgObjPtr)->GetSignalName();
		    m_nrOfMsgsReadFromQueue++;
		    // Update more statistics (Moved here from send(), as only one thread uses the receive function, so there will be no concurrency.
		    //			       And we are always sure that the message queue object exists.)
		    int lCurrNrOfMsgs = GetNrOfMsgsInQueue() + 1;
		    if (lCurrNrOfMsgs > m_maxUsedSize)
		    {
			m_maxUsedSize = lCurrNrOfMsgs;
		    }

		    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_INFO, "CMetzMsgQueue::ReceiveFromNonblockingQueue - receive - handle=%d size: %d  errno=%d  signal name = %ld\n",
			    m_msgQueueHandle, rc_size, errno, ((CMetzMsg *) argMsgObjPtr)->GetSignalName());
		    return rc;
		}
		else
		{
		    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::ReceiveFromNonblockingQueue - receive failed - handle=%d size: %d  errno=%d\n", m_msgQueueHandle, rc_size, errno);
		    return ERR_METZ_MSG_QUEUE_RECEIVE_FAILED;
		}
	    }
	    else
	    {
		MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_WARNING, "CMetzMsgQueue::ReceiveFromNonblockingQueue - select, but not our queue - handle=%d rc_select: %d errno=%d\n", m_msgQueueHandle, rc_select, errno);
		return ERR_METZ_MSG_QUEUE_SELECT_BUT_NOT_OUR_QUEUE;
	    }
	}
	else
	{
	    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::ReceiveFromNonblockingQueue - select failed - handle=%d rc_select: %d errno=%d\n", m_msgQueueHandle, rc_select, errno);
	    return ERR_METZ_MSG_QUEUE_SELECT_FAILED;
	}
    }
    return rc;
}

/*--------------------------------------------------------------------------*/
/**
@brief	 Receive
@param	 argMsgObjPtr		Ptr to a message object
@param	 argMsgPriority		(not used, obsolete)
@return	 error status
@note	 status: tested

	 Receives the oldest message with the highest priority from the message queue.
	 It depends on the type of the queue (blocking or non-blocking) if
	 the function waits on an empty queue or not.
*/
/*--------------------------------------------------------------------------*/

int CMetzMsgQueue::Receive(CMetzMsg * argMsgObjPtr, unsigned int argMsgPriority)
{
    int rc = ERR_NO_ERROR;

    m_msgQueueNotInUse = false;		// Reset it, as we are using the queue

    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_DEBUG, "CMetzMsgQueue::Receive (obj) called - handle=%d msg_data_ptr=%p\n",m_msgQueueHandle, argMsgObjPtr);
    if (m_msgQueueHandle == MSG_QUEUE_HANDLE_UNDEFINED)
    {
	rc = ERR_METZ_MSG_QUEUE_RECEIVE_NO_HANDLE;
	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::Receive (obj) FAILED - No handle given\n");
    }
    else if (argMsgObjPtr == 0)
    {
	rc = ERR_METZ_MSG_PTR_IS_NULL;
	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::Receive (obj) FAILED - argMsgObjPtr = 0\n");
    }
    else
    {
	int rc_size = mq_receive(m_msgQueueHandle, (char *) argMsgObjPtr->GetMsgDataAddr(), CMetzMsg::GetMsgDataSize(), NULL);
	// Upon successful completion, mq_receive() returns the length of the
	// selected message in bytes and the message is removed from the queue.
	// Otherwise, no message is removed from the queue, the function returns
	// a value of -1, and sets errno to indicate the error.
	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_DEBUG, "CMetzMsgQueue::Receive (obj) - size: %d	 errno=%d  response info = %i\n",
						       rc_size, errno, ((CMetzMsg *) argMsgObjPtr)->data.ResponseInfo1.intData);

	//^^ Should we check if the size matches with the message size, or is at least the message size ?!
	if (rc_size > 0)
	{
	    m_lastMsgReadFromQueue = ((CMetzMsg *) argMsgObjPtr)->GetSignalName();
	    m_nrOfMsgsReadFromQueue++;
	    // Update more statistics (Moved here from send(), as only one thread uses the receive function, so there will be no concurrency.
	    //			       And we are always sure that the message queue object exists.)
	    int lCurrNrOfMsgs = GetNrOfMsgsInQueue() + 1;
	    if (lCurrNrOfMsgs > m_maxUsedSize)
	    {
		m_maxUsedSize = lCurrNrOfMsgs;
	    }
	    return rc;
	}
	else
	{
	    // Do not print an error message here, as someone may use it to read from a nonblocking queue
	    return ERR_METZ_MSG_QUEUE_RECEIVE_FAILED;
	}
    }
    return rc;
}

/*--------------------------------------------------------------------------*/
/**
@brief	 Send
@param	 argMsgObjPtr		Ptr to a message object
@param	 argMsgPriority		Priority of the message
@return	 none
@note	 status: tested

	 Sends a message to a message queue.
*/
/*--------------------------------------------------------------------------*/

int CMetzMsgQueue::Send(CMetzMsg * argMsgObjPtr,
			unsigned int argMsgPriority)
{
    int rc = ERR_NO_ERROR;
    M_ESignalId lSignalName;

    if (argMsgObjPtr == 0)
    {
	rc = ERR_METZ_MSG_PTR_IS_NULL;
	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::Send (obj) - argMsgObjPtr is 0\n");
	return rc;
    }

    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_INFO, "CMetzMsgQueue::Send (obj) called - msg_obj=%p prio=%d msg_queue=%d signalname=%ld signalptr=%p\n", argMsgObjPtr, argMsgPriority, m_msgQueueHandle, argMsgObjPtr->GetSignalName(), argMsgObjPtr->GetSignalPtr() );

    lSignalName = (M_ESignalId) argMsgObjPtr->GetSignalName();

    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_DEBUG, "CMetzMsgQueue::Send (obj) - response info = %i - signal=%d %s\n", (argMsgObjPtr)->data.ResponseInfo1.intData, lSignalName, GetSignalIdNamePtr(lSignalName));

    if (m_msgQueueHandle == MSG_QUEUE_HANDLE_UNDEFINED)
    {
	rc = ERR_METZ_MSG_QUEUE_SEND_NO_HANDLE;
	argMsgObjPtr->FreeSignal();	// Free the attached memory if sending was unsuccessful
					// Has to be performed here also, as function may be called directly by the user
	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::Send (obj) - Handle is undefined - signal=%d %s\n", lSignalName, GetSignalIdNamePtr(lSignalName));
    }
    else if (m_msgQueueHandle == 0)
    {
	rc = ERR_METZ_MSG_QUEUE_SEND_HANDLE_IS_NULL;
	argMsgObjPtr->FreeSignal();	// Free the attached memory if sending was unsuccessful
					// Has to be performed here also, as function may be called directly by the user
	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::Send (obj) - Handle is 0 - signal=%d %s\n", lSignalName, GetSignalIdNamePtr(lSignalName));
    }
    else
    {
	if ((argMsgObjPtr->GetSignalPtr() != 0) && (argMsgObjPtr->GetSignalSize() == 0))
	{
	    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_WARNING, "CMetzMsgQueue::Send (obj) - Signal pointer is set, but signalSize is 0 - signal=%d %s\n", lSignalName, GetSignalIdNamePtr(lSignalName));
	}

	if (argMsgObjPtr->GetSignalName() == 0)
	{
	    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_WARNING, "CMetzMsgQueue::Send (obj) - Signal name = 0 Thread-Id = %ld\n", syscall(SYS_gettid));
	}


	int rc2 = mq_send (m_msgQueueHandle, (char *) argMsgObjPtr->GetMsgDataAddr(),
			   CMetzMsg::GetMsgDataSize(), argMsgPriority);
	// Upon successful completion, the mq_send() function returns a value of
	// zero. Otherwise, no message is enqueued, the function returns -1, and
	// errno is set to indicate the error.
	// Tip: errno EBADF (value=9, Bad file number) arises if the queue was not
	//	created with the "writeable" flag (e.g. O_RDWR)

	if (rc2 == -1)
	{
	    if (m_msgQueueNotInUse == false)
	    {
		// Signal an error and give a trace only if the queue is in use
		MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::Send (obj) FAILED - handle=%d rc2=%d errno=%d signal=%d %s\n", m_msgQueueHandle, rc2, errno, lSignalName, GetSignalIdNamePtr(lSignalName));
		rc = ERR_METZ_MSG_QUEUE_SEND_FAILED;
	    }
	    argMsgObjPtr->FreeSignal();	    // Free the attached memory if sending was unsuccessful
					    // Has to be performed here also, as function may be called directly by the user
	}
	else
	{
	    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_DEBUG, "CMetzMsgQueue::Send (obj) to queue successful- handle=%d rc2=%d signal=%d\n", m_msgQueueHandle, rc2, lSignalName);
	    // Clear the signal pointer in the original message, so that it can not be used erroneous multiple times.
	    argMsgObjPtr->SetSignalPtr(0,0);
	    argMsgObjPtr->SetSignalObjPtr(0);

	    totalNrOfSentMsgs++; // This is only for a rough estimation. Therefore there is no need for thread-safety here,
				 // which may cause additional context switches.
	}
    }
    return rc;
}

/*--------------------------------------------------------------------------*/
/**
@brief	 Send
@param	 argTargetQueue		Queue handle
@param	 argMsgObjPtr		Ptr to a message object
@param	 argMsgPriority		Priority of the message
@return	 none
@note	 status: tested

	 Sends a message to a message queue.
*/
/*--------------------------------------------------------------------------*/

int CMetzMsgQueue::Send(mqd_t	   argTargetQueue,
			CMetzMsg * argMsgObjPtr,
			unsigned int argMsgPriority)
{
    int rc = ERR_NO_ERROR;
    M_ESignalId lSignalName;

    if (argMsgObjPtr == 0)
    {
	rc = ERR_METZ_MSG_PTR_IS_NULL;
	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::Send (obj to handle) - argMsgObjPtr = 0\n");
	return rc;
    }

    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_INFO, "CMetzMsgQueue::Send (obj to handle) called - target_handle=%d msg_obj=%p prio=%d msg_queue=%d signalname=%ld signalptr=%p\n", (int) argTargetQueue, argMsgObjPtr, argMsgPriority, argTargetQueue,argMsgObjPtr->GetSignalName(), argMsgObjPtr->GetSignalPtr() );

    lSignalName = (M_ESignalId) argMsgObjPtr->GetSignalName();

    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_DEBUG, "CMetzMsgQueue::Send (obj to handle) - response info = %i - signal=%d %s\n", (argMsgObjPtr)->data.ResponseInfo1.intData, lSignalName, GetSignalIdNamePtr(lSignalName));

    if (argTargetQueue == MSG_QUEUE_HANDLE_UNDEFINED)
    {
	rc = ERR_METZ_MSG_QUEUE_SEND_NO_TARGET_HANDLE;
	argMsgObjPtr->FreeSignal();	// Free the attached memory if sending was unsuccessful
					// Has to be performed here also, as function may be called directly by the user
	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::Send (obj to handle) - Handle is undefined - signal=%d %s\n", lSignalName, GetSignalIdNamePtr(lSignalName));
    }
    else if (argTargetQueue == 0)
    {
	rc = ERR_METZ_MSG_QUEUE_SEND_HANDLE_IS_NULL;
	argMsgObjPtr->FreeSignal();	// Free the attached memory if sending was unsuccessful
					// Has to be performed here also, as function may be called directly by the user
	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::Send (obj to handle) - Handle is 0 - signal=%d %s\n", lSignalName, GetSignalIdNamePtr(lSignalName));
    }
    else
    {
	if ((argMsgObjPtr->GetSignalPtr() != 0) && (argMsgObjPtr->GetSignalSize() == 0))
	{
	    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_WARNING, "CMetzMsgQueue::Send (obj to handle) - Signal pointer is set, but signalSize is 0 - signal=%d %s\n", lSignalName, GetSignalIdNamePtr(lSignalName));
	}

	int rc2 = mq_send (argTargetQueue, (char *) argMsgObjPtr->GetMsgDataAddr(),
			   CMetzMsg::GetMsgDataSize(), argMsgPriority);
	// Upon successful completion, the mq_send() function returns a value of
	// zero. Otherwise, no message is enqueued, the function returns -1, and
	// errno is set to indicate the error.
	// Tip: errno EBADF (value=9, Bad file number) arises if the queue was not
	//	created with the "writeable" flag (e.g. O_RDWR)

	CMetzMsgQueue * lMsgQueue;

	// Get ptr to queue object
	lMsgQueue = CMetzMsgQueue::GetQueueObjPtrByHandle(argTargetQueue);

	// There is a chance of thread interference at this point, which can not easily be prevented, without impacting the whole system.
	// It would require a global mutex to be used by all queues (as we can not be sure that the queue object still exists at this time),
	// leading to many unwanted context switches.
	// But this should not lead to any serious problems.
	// The queue object may get deleted at this point by another thread.
	// But as we only use the queue object to READ the information if the queue is still in use, we may only get a wrong information.
	// This would only lead to an unwanted printed trace message, which is indeed not so serious.

	if (rc2 == -1)
	{
	    // Signal an error and give a trace only if the queue is in use
	    if ((lMsgQueue != 0) && (lMsgQueue->m_msgQueueNotInUse == true))
	    {
		; // Ignore error
	    }
	    else
	    {
		    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::Send (obj to handle) to queue FAILED (obj to handle) - handle=%d rc=%d errno=%d signal=%d %s\n", argTargetQueue, rc, errno, lSignalName, GetSignalIdNamePtr(lSignalName));
		    rc = ERR_METZ_MSG_QUEUE_SEND_FAILED;
	    }
	    argMsgObjPtr->FreeSignal();	    // Free the attached memory if sending was unsuccessful
					    // Has to be performed here also, as function may be called directly by the user
	}
	else
	{
	    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_DEBUG, "CMetzMsgQueue::Send (obj to handle) to queue successful- handle=%d rc=%d signal=%d\n", argTargetQueue, rc, lSignalName);
	    // Clear the signal pointers in the original message, so that it can not be used erroneous multiple times.
	    argMsgObjPtr->SetSignalPtr(0,0);
	    argMsgObjPtr->SetSignalObjPtr(0);

	    totalNrOfSentMsgs++; // This is only for a rough estimation. Therefore there is no need for thread-safety here,
				 // which may cause additional context switches.
	}
    }
    return rc;
}

/*--------------------------------------------------------------------------*/
/**
@brief	 GetNrOfMsgsInQueue
@param	 none
@return	 nr of messages		(if negative then it is an error code)
@note	 status: tested

	 Returns the number of messages currently in the queue.
*/
/*--------------------------------------------------------------------------*/

int CMetzMsgQueue::GetNrOfMsgsInQueue()
{
    int rc = ERR_NO_ERROR;
    struct mq_attr mqattrTmp;

    if (m_msgQueueHandle == MSG_QUEUE_HANDLE_UNDEFINED)
    {
	rc = ERR_METZ_MSG_QUEUE_GETNR_NO_HANDLE;
	MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::GetNrOfMsgsInQueue FAILED - No handle given\n");
    }
    else
    {
	int rc2 = mq_getattr(m_msgQueueHandle, &mqattrTmp);
	// Upon successful completion, the mq_getattr() function returns zero.
	// Otherwise, the function returns -1 and sets errno to indicate the error.

	if (rc2 >= 0)
	{
	    rc = mqattrTmp.mq_curmsgs;
	}
	else
	{
	    rc = ERR_METZ_MSG_QUEUE_GETNR_FAILED;
	    MTRACE_P(TRACE_METZMSGQUEUE, TRACELEVEL_ERROR, "CMetzMsgQueue::GetNrOfMsgsInQueue failed - return value: %d	 errno=%d\n", rc2, errno);
	}
    }
    return rc;
}

/*--------------------------------------------------------------------------*/
/**
@brief	 Terminate
@param	 none
@return	 none

	 Sets the terminated flag.
	 This function may be called by a program just before exiting main().
	 It is a workaround to prevent the conflict between running the
	 destructors of the static queue objects and the global queue list object.
	 In the long run this problem should be solved in a different way.
*/
/*--------------------------------------------------------------------------*/

void CMetzMsgQueue::Terminate()
{
    m_metzMsgQueueHandlingTerminated = true;
}

/*--------------------------------------------------------------------------*/
/*	INTERNAL CLASSES						    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*	DEBUG FUNCTIONS							    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*	TEST FUNCTIONS							    */
/*--------------------------------------------------------------------------*/

// Just for testing

int CMetzMsgQueue::GetTotalNrOfSentMsgs()
{
    return CMetzMsgQueue::totalNrOfSentMsgs;
}

/*--------------------------------------------------------------------------*/

// Just for testing

void CMetzMsgQueue::ClearTotalNrOfSentMsgs()
{
    CMetzMsgQueue::totalNrOfSentMsgs = 0;
}

/*--------------------------------------------------------------------------*/

// Just for testing

void CMetzMsgQueue::usleepOnQueue(unsigned long argInterval)
{
    struct timeval tv;

    tv.tv_sec = 0;
    tv.tv_usec = argInterval;

    /*int rc_select = */select(m_msgQueueHandle+1, &rfds, NULL, NULL, &tv);
}

/*--------------------------------------------------------------------------*/
/* end of  MetzMsgQueue.cpp */
