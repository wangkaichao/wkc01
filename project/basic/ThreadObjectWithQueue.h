/**
* @file ThreadObjectWithQueue.h
*
* @brief ThreadObjectWithQueue		Base class for objects running in their own thread and processing messages from their queue
*
* @author Peter Wierzba
*
* $Revision: 1.11 $
* $Date: 2012/06/01 14:30:33GMT $
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
/* Archiv: $Source: basics/src/ThreadObjectWithQueue.h $ */
/*==========================================================================*/

#ifndef __THREADOBJECTWITHQUEUE_H__
#define __THREADOBJECTWITHQUEUE_H__

/*--------------------------------------------------------------------------*/
/*	INCLUDE FILES							    */
/*--------------------------------------------------------------------------*/

#include <string>
#include "MetzMsgQueue.h"
#include "ThreadObject.h"
//#include "TimeOutTimer.h"

/*--------------------------------------------------------------------------*/
/*	CONSTANTS							    */
/*--------------------------------------------------------------------------*/

// ERROR CODES for this unit (range: -1110 .. -1119)

// All error codes have negative values.
// ERR_NO_ERROR is used to indicate "no error" (value = 0)
#define ERR_THREAD_OBJ_QUEUE_NOT_INITIALIZED			-1110
#define ERR_THREAD_OBJ_NOT_INITIALIZED				-1111

/*--------------------------------------------------------------------------*/
/*	TYPE DEFINITIONS						    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*	CLASS DECLARATIONS						    */
/*--------------------------------------------------------------------------*/

class CThreadObjectWithQueue : public CThreadObject
{
    // --- Member variables

private:

protected:
    CMetzMsgQueue	m_Queue1;	 // Needed for callback registration    
    std::string		m_QueueName1;
    int			m_QueueSize1;
    bool		m_initialized;
    unsigned int	m_nrOfProcessedMsgs; // Just used for performance tests
    //TimeOutTimer 	m_dummyMsgTimer; // Only for test

    // --- Member functions

public:
    CThreadObjectWithQueue();			// Constructor
    CThreadObjectWithQueue(int argDbgId);	// Constructor
    virtual ~CThreadObjectWithQueue();		// Destructor (ALLWAYS virtual)

    int	 Init(const char * argQueueNamePrefix, int argQueueSize);
    virtual int	 SendCmd(CMetzMsg *argMsg);
    mqd_t GetQueueHandle() const { return m_Queue1.GetQueueHandle(); };
    const char * GetQueueName() const { return m_QueueName1.c_str(); };    
    virtual M_ThreadType GetThreadType(void) { return THREAD_TYPE_WITH_QUEUE; };
    virtual int StartThread(const char *argName = 0, int argStackSize = 0, int argPriority = 0); // Starts the thread which executes RealThreadFunction()
    virtual void StopThread();        // Stops the thread. Returns immediately.
    virtual void StopThreadAndWait(); // Stops the thread. Defers the calling thread until it is really stopped.

    virtual unsigned int GetNrOfProcessedMsgs() const { return m_nrOfProcessedMsgs; };

protected:

    // This method has to be defined by the derived class.
    // It will be called once for every msg fetched from the input queue.
    virtual void ProcessMetzMsg(CMetzMsg *argMsgPtr) = 0;

    // This method may be defined by the derived class.
    // It will be called before the first msg is fetched from the input queue.
    virtual void BeforeProcessingMsgs();
    
private:

    virtual void ThreadWorkFunction();	// Does the real work in the thread
    virtual void ReadMsgsFromBlockingQueue();
    virtual void ReadMsgsFromNonBlockingQueue();    

private:
};

/*--------------------------------------------------------------------------*/

#endif	/* __THREADOBJECTWITHQUEUE_H__ */

/*--------------------------------------------------------------------------*/
/* end of ThreadObjectWithQueue.h */
