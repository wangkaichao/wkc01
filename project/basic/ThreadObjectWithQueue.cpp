/**
* @file	 ThreadObjectWithQueue.cpp
*
* @brief  ThreadObjectWithQueue		Base class for objects running in their own thread and processing messages from their queue
*
* @author  Peter Wierzba
*
* $Revision: 1.10 $
* $Date: 2012/06/01 14:33:26GMT $
*
* @note Copyright (c) 2009 Metz-Werke GmbH  CoKG \n
*	All rights reserved
*
*	This base class can be used to derive classes where
*	each instance should run in a separate thread and processes messages
*	from their own input queue.
*	The derived class has to implement the function
*	"void ProcessMetzMsg()", which performes the real work of the object.
*
* Usage:	CTestThreadObjectWithQueue myTO1(3);
*		myTO1.Init();
*		myTO1.StartThread();
*
* History:
*
* DD.MM.YY Who Ticket  Description
* 12.02.09 PWI 1       First Issue
*/
/* Archiv: $Source: basics/src/ThreadObjectWithQueue.cpp $ */
/*==========================================================================*/

/*--------------------------------------------------------------------------*/
/*	INCLUDE FILES							    */
/*--------------------------------------------------------------------------*/

#include <errno.h>
#include <iostream>
#include <ostream>
#include <sstream>
#include <unistd.h>	//sleep, usleep, getpid, sync

//#include "MetzTrace.h"
//#include "MetzTraceUnitsGeneral.h"
#include "ThreadObjectWithQueue.h"
#include "SignalNo.h"

using namespace std;

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
/*	PUBLIC CLASSES							    */
/*--------------------------------------------------------------------------*/

/*==========================================================================*/
/* CThreadObjectWithQueue						    */
/*==========================================================================*/

/*--------------------------------------------------------------------------*/
/**
@brief	 Constructor
@param	 none
@return	 none
@note	 status: tested

	 Will be called when object instance is created
*/
/*--------------------------------------------------------------------------*/

CThreadObjectWithQueue::CThreadObjectWithQueue()
			    : CThreadObject()		// Call constructor of baseclass
{
    m_initialized = false;
    m_QueueSize1 = 0;
    m_nrOfProcessedMsgs = 0; // Just used for debug
}

/*--------------------------------------------------------------------------*/
/**
@brief	 Constructor
@param	 argDbgId	Just a number to identify the object for debug purpose.
@return	 none
@note	 status: tested

	 Will be called when object instance is created
*/
/*--------------------------------------------------------------------------*/

CThreadObjectWithQueue::CThreadObjectWithQueue(int argDbgId)
			    : CThreadObject(argDbgId)		// Call constructor of baseclass
{
    m_initialized = false;
    m_QueueSize1 = 0;
    m_nrOfProcessedMsgs = 0; // Just used for debug
}

/*--------------------------------------------------------------------------*/
/**
@brief	 Destructor
@param	 none
@return	 none
@note	 status: tested

	 Will be called when object instance is deleted
*/
/*--------------------------------------------------------------------------*/

CThreadObjectWithQueue::~CThreadObjectWithQueue()
{
    // MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_SYSINFO_GREEN, "CThreadObjectWithQueue:: Destructor - Stopping thread and wait - m_threadName = %s\n", m_threadName.c_str());

    CThreadObjectWithQueue::StopThreadAndWait();  // Call function of this class, and not a derived version.

    // This will call a pthread_join() to wait until the thread is stopped.
    // This call has to be done explicitly from this location!!!
    // Calling it automatically from inside the destructor of the base class
    // is not sufficient, as before this the message queue will get destructed
    // first. Therefore the stop message will never be received in this case,
    // and the thread does not terminate.

    // Note: Destructors of the base classes are called automatically,
    // but the desctructors of own member varialbes are called before.
}

/*--------------------------------------------------------------------------*/
/**
@brief	 Init
@param	 argQueueNamePrefix	Meaningful string for naming the input queue.
@param	 argQueueSize		Maximum number of messages in the queue.
@return  error code    		(in case of success "ERR_NO_ERROR").
@note	 status: tested

	 Initializes the object. Has to be called before any usage.
*/
/*--------------------------------------------------------------------------*/

int CThreadObjectWithQueue::Init(const char * argQueueNamePrefix, int argQueueSize)
{
    int rc = 0;

    // Every object instance needs an unique queue name. Therefore the name
    // is constructed by combining a prefix with the address of the object.

    m_QueueName1 = argQueueNamePrefix;
    m_QueueSize1 = argQueueSize;

    std::ostringstream os;
    os << (intptr_t) this;
    m_QueueName1 += os.str();

    MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_DEBUG, "CThreadObjectWithQueue:: Queuename = %s\n", m_QueueName1.c_str());

    // Try to kill an already existing queue (which may have survived due to
    // a crash of the system).

    mq_unlink(m_QueueName1.c_str());

    // Create the queue
    // Options: O_NONBLOCK	Queue reads/writes will be non blocking
    
    // Now we use a nonblocking queue (means no blocking during write into queue),
    // together with select (which supports to make a blocking read on a nonblocking queue).,
    rc = m_Queue1.Open(m_QueueName1.c_str(),O_CREAT | O_RDWR | O_NONBLOCK, 0, argQueueSize);

    if (rc >= ERR_NO_ERROR)
    {
	m_initialized = true;	// Set it only if everything was successful
	MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_DEBUG, "CThreadObjectWithQueue::Init() - %d - %s\n", rc, m_QueueName1.c_str());	
    }
    else
    {
        MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_WARNING, "CThreadObjectWithQueue::Init() FAILED - %d - %s\n", rc, m_QueueName1.c_str());
    }
    return rc;
}

/*--------------------------------------------------------------------------*/
/**
@brief	 SendCmd
@param	 argMsg			Message to be sent into the own input queue.
@return  error code       	(in case of success "ERR_NO_ERROR").
@note	 status: tested

	 Sends a mesage into the own input queue.
*/
/*--------------------------------------------------------------------------*/

int  CThreadObjectWithQueue::SendCmd(CMetzMsg *argMsg)
{
    int rc = 0;

    if (m_initialized == true)
    {
	rc = m_Queue1.Send(argMsg, M_SQ_PRIO_LOW); // Append the msg to the queue
	MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_DEBUG, "CThreadObjectWithQueue::SendCmd - %d - %s\n", rc, m_QueueName1.c_str());	
    }
    else
    {
	if (argMsg->GetSignalName() != SIG_ID_STOP_THREAD)  // Thread is not initialized, so stopping is not required.
	{
            MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_ERROR, "CThreadObjectWithQueue::SendCmd when not initialized - Calling thread id = %lu\n", pthread_self());
	    rc = ERR_THREAD_OBJ_QUEUE_NOT_INITIALIZED;
	}
	argMsg->FreeSignal();
    }
    return rc;
}

/*--------------------------------------------------------------------------*/
/**
@brief	 ThreadWorkFunction	Working funktion of this thread
@param	 none
@return  none
@note	 status: tested

	 Calls the (possible existing derived) function BeforeProcessingMsgs()
	 before entering the loop for processing messages from the input queue.
	 Can be terminated by sending a SIG_ID_STOP_THREAD message into the queue.
*/
/*--------------------------------------------------------------------------*/

void CThreadObjectWithQueue::ThreadWorkFunction()
{
    // Call a derived function to allow execute actions before processing messages.
    BeforeProcessingMsgs();

    // Enter loop for processing messages.
    ReadMsgsFromNonBlockingQueue();

    // Clear queue, as other messages may have arrived in the meantime.
    m_Queue1.ClearQueue();
}

/*--------------------------------------------------------------------------*/
/**
@brief	 BeforeProcessingMsgs	Optional derived function called before processing messages from the input queue.
@param	 none
@return  none
@note	 status: tested

         This function may be overwritten by derived classes to execute some
    	 actions before processing the messages.
    	 Typically this may be used to send an initial message.
*/
/*--------------------------------------------------------------------------*/

void CThreadObjectWithQueue::BeforeProcessingMsgs()
{

    MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_DEBUG, "CThreadObjectWithQueue::BeforeProcessingMsgs() called\n");
    
    // Does nothing in this base class.
    return;

}

/*--------------------------------------------------------------------------*/
/**
@brief	 ReadMsgsFromNonBlockingQueue	Reads messages from the own input queue.
@param	 none
@return  none
@note	 status: tested

	 Enters the loop for reading messages from the input queue. Calls
	 the derived function ProcessMetzMsg() for processing the messages. 
	 Can be terminated by sending a SIG_ID_STOP_THREAD message into the queue.
*/
/*--------------------------------------------------------------------------*/

void CThreadObjectWithQueue::ReadMsgsFromNonBlockingQueue()
{
    int lCtr = 0;	// Just for debug
    int rc = 0;
    CMetzMsg  myMsg2;

    while (!m_stopRequested)
    {
	// Do we need the mutext here?
	pthread_mutex_lock(&m_mutex);

	MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_DEBUG, "CThreadObjectWithQueue::ReadMsgsFromNonBlockingQueue() - %d - %d\n", m_dbgId, lCtr);
	lCtr++;

	// Read msg from queue
	MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_DEBUG, " CThreadObjectWithQueue::Reading message\n");

        rc = m_Queue1.ReceiveFromNonblockingQueue(&myMsg2, M_SQ_PRIO_LOW);

	if (rc >= ERR_NO_ERROR)
	{
	    MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_INFO, " CThreadObjectWithQueue::Received Msg rc=%i Signal name = %li Thread-Id = %ld\n", rc, myMsg2.GetSignalName(), GetThreadId());

	    if (myMsg2.GetSignalName() != SIG_ID_STOP_THREAD) // Stop thread when this msg arrives.
	    {
	        ProcessMetzMsg(&myMsg2);	// Call the function defined by the derived class.
	        m_nrOfProcessedMsgs++;
	    }
	    else
	    {
		MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_INFO2, "CThreadObjectWithQueue::Received SIG_ID_STOP_THREAD - Thread-Id = %ld\n", GetThreadId());
		m_stopRequested = true; // Set the flag. This leads to an exit of the while loop, which will end the thread.
	    }
	}
	else
	{
	    if (rc == ERR_METZ_MSG_QUEUE_SELECT_BUT_NOT_OUR_QUEUE)
	    {
		// This is not really an error, but should not occur.
		usleep(20000); // Prevent us from having an endless loop leading to a high processor load.
	    }
	    else
	    {
	        MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_ERROR, " CThreadObjectWithQueue::ERROR - Read message failed - rc=%d - Thread-Id = %ld\n", rc, GetThreadId());
	    }
	}

	pthread_mutex_unlock(&m_mutex);
    }
}

/*--------------------------------------------------------------------------*/
/**
@brief	 ReadMsgsFromBlockingQueue	Reads messages from the own input queue.
@param	 none
@return  none
@note	 status: tested

	 Enters the loop for reading messages from the input queue. Calls
	 the derived function ProcessMetzMsg() for processing the messages. 
	 Can be terminated by sending a SIG_ID_STOP_THREAD message into the queue.

	 THIS FUNCTION IS NOT USED INTHE DEFAULT CASE, AS THE QUEUE IS NON-BLOCKING.
*/
/*--------------------------------------------------------------------------*/

void CThreadObjectWithQueue::ReadMsgsFromBlockingQueue()
{
    int lCtr = 0;	// Just for debug
    int rc = 0;
    CMetzMsg  myMsg2;

    while (!m_stopRequested)
    {
	// Do we need the mutext here?
	pthread_mutex_lock(&m_mutex);

	MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_DEBUG, "CThreadObjectWithQueue::ThreadWorkFunction() - %d - %d\n", m_dbgId, lCtr);
	lCtr++;

	// Read msg from queue
	MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_DEBUG, " CThreadObjectWithQueue::Reading message\n");
	rc = myMsg2.ReceiveMsg(&m_Queue1, M_SQ_PRIO_LOW);

	if (rc >= ERR_NO_ERROR)
	{
	    MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_DEBUG, " CThreadObjectWithQueue::Received Msg rc=%i Info = %i\n", rc, myMsg2.data.ResponseInfo1.intData);

	    if (myMsg2.GetSignalName() != SIG_ID_STOP_THREAD)  // Stop thread when this msg arrives.
	    {
	        ProcessMetzMsg(&myMsg2);	// Call the function defined by the derived class.
	        m_nrOfProcessedMsgs++;
	    }
	    else
	    {
		MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_DEBUG, "CThreadObjectWithQueue::Received SIG_ID_STOP_THREAD\n");
		m_stopRequested = true; // Set the flag. This leads to an exit of the while loop, which will end the thread.
	    }
	}
	else
	{
	    MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_DEBUG, " CThreadObjectWithQueue::ERROR - Read message failed - rc=%d\n", rc);
	}

	pthread_mutex_unlock(&m_mutex);
    }
}

/*--------------------------------------------------------------------------*/
/**
@brief	 StartThread
@param	 none
@return  error code         (in case of success "ERR_NO_ERROR")

	 Starts the thread execution.
	 This function is used to check if the queue is initialized before
	 starting the thread. It implicit provides a trigger point to catch
	 violators.
*/
/*--------------------------------------------------------------------------*/

int CThreadObjectWithQueue::StartThread(const char *argName, int argStackSize, int argPriority)
{
    int rc = ERR_NO_ERROR;

    if (m_initialized == false)
    {
	MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_ERROR, "CThreadObjectWithQueue::StartThread() Thread not started, as object is not initialized - Name: %s\n", argName);
        rc = ERR_THREAD_OBJ_NOT_INITIALIZED;
    }
    else
    {
        // Call base class function
        CThreadObject::StartThread(argName, argStackSize, argPriority);
    }

    return rc;
}

/*--------------------------------------------------------------------------*/
/**
@brief	 StopThread
@param	 none
@return	 none

	 This function is called when the thread is requested to be stopped.
	 It sends a stop request message into the queue.
*/
/*--------------------------------------------------------------------------*/

void CThreadObjectWithQueue::StopThread()
{
    if (m_initialized == true)
    {
	MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_INFO2, "CThreadObjectWithQueue::StopThread() - Sending SIG_ID_STOP_THREAD\n");

	// The thread is likely waiting for any message.
	// So send a message to get him out of the wait to stop the thread.
	CMetzMsg lMsg;

	lMsg.SetSignalName(SIG_ID_STOP_THREAD);
	lMsg.SendMsg(&m_Queue1);
    }
    else
    {
	MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_WARNING, "CThreadObjectWithQueue::StopThread() - Stop for uninitialized thread ignored\n");
    }

    // Call baseclass
    CThreadObject::StopThread();
}

/*--------------------------------------------------------------------------*/
/**
@brief	 StopThreadAndWait
@param	 none
@return	 none

	 This function is called when the thread is requested to be stopped.
	 It sends a stop request message into the queue.

 	 THE CALL WAITS UNTIL THE THREAD IS REALLY STOPPED!
*/
/*--------------------------------------------------------------------------*/

void CThreadObjectWithQueue::StopThreadAndWait()
{
    if (m_initialized == true)
    {
	MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_INFO2, "CThreadObjectWithQueue::Sending SIG_ID_STOP_THREAD\n");

	// The thread is likely waiting for any message.
	// So send a message to get him out of the wait to stop the thread.
	CMetzMsg lMsg;

	lMsg.SetSignalName(SIG_ID_STOP_THREAD);
	lMsg.SendMsg(&m_Queue1);
    }

    MTRACE_P(TRACE_TOWQUEUE, TRACELEVEL_DEBUG, "CThreadObjectWithQueue:: Wait until thread is finished\n");

    // Call baseclass
    CThreadObject::StopThreadAndWait();
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

/*--------------------------------------------------------------------------*/
/* end of  ThreadObjectWithQueue.cpp */
