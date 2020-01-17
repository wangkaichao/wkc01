/**
* @file	 ThreadObject.cpp
*
* @brief  ThreadObject	Base class for objects running in their own thread.
*
* @author  Peter Wierzba
*
* $Revision: 1.15 $
* $Date: 2012/01/30 10:26:49CET $
*
* @note Copyright (c) 2009 Metz-Werke GmbH  CoKG \n
*	All rights reserved
*
*	This base class can be used to derive classes where
*	each instance should run in a separate thread.
*	The derived class has to implement the function
*	"void ThreadWorkFunction()", which performes the
*	real work of the object.
*
* Usage:		CTestThreadObject myTO1(3);
*			myTO1.StartThread();
*
* History:
*
* DD.MM.YY Who Ticket  Description
* 12.02.09 PWI 1       First Issue
*/
/* Archiv: $Source: basics/src/ThreadObject.cpp $ */
/*==========================================================================*/

/*--------------------------------------------------------------------------*/
/*	INCLUDE FILES							    */
/*--------------------------------------------------------------------------*/

#include <errno.h>
#include <syscall.h>
#include <sys/prctl.h>
#include <unistd.h>	//sleep, usleep

//#include "MetzTrace.h"
//#include "MetzTraceUnitsGeneral.h"
#include "ThreadObject.h"

/*--------------------------------------------------------------------------*/
/*	INTERNAL CONSTANTS						    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*	INTERNAL TYPE DEFINITIONS					    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*	GLOBAL DATA							    */
/*--------------------------------------------------------------------------*/

// Create instances of static data

std::list<T_GlobalThreadInfo> CThreadObject::globalThreadObjectList;
Mutex_Type CThreadObject::m_metzGlobalThreadMutex;
bool	   CThreadObject::m_metzGlobalThreadMutexInitialized = false;
bool	   CThreadObject::m_metzGlobalThreadHandlingTerminated = false;

/*--------------------------------------------------------------------------*/
/*	INTERNAL DATA							    */
/*--------------------------------------------------------------------------*/

static const char * myDefaultString = "(unknown)";

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
/* CThreadObject							    */
/*==========================================================================*/

/*--------------------------------------------------------------------------*/
/**
@brief	 Constructor
@param	 none
@return	 none

	 Will be called when object instance is created
*/
/*--------------------------------------------------------------------------*/

CThreadObject::CThreadObject()
		: m_stopRequested(false), m_thread(0), m_running(false), m_threadId(0), m_processId(0)
{
    pthread_mutex_init(&m_mutex, 0);
    m_dbgId = 0;
}

/*--------------------------------------------------------------------------*/
/**
@brief	 Constructor
@param	 argDbgId	Just a number to identify the object for debug purpose.
@return	 none

	 Will be called when object instance is created
*/
/*--------------------------------------------------------------------------*/

CThreadObject::CThreadObject(int argDbgId)
		: m_stopRequested(false), m_thread(0), m_running(false), m_threadId(0), m_processId(0)
{
    pthread_mutex_init(&m_mutex, 0);
    m_dbgId = argDbgId;
}

/*--------------------------------------------------------------------------*/
/**
@brief	 Destructor
@param	 none
@return	 none

	 Will be called when object instance is deleted
*/
/*--------------------------------------------------------------------------*/

CThreadObject::~CThreadObject()
{
    CThreadObject::StopThreadAndWait(); // Call function of this class, and not a derived version.
    pthread_mutex_destroy(&m_mutex);
}

/*--------------------------------------------------------------------------*/
/**
@brief	 StartThread
@param	 none
@return	 none

	 Creates the POSIX thread and adds info to the global thread list
*/
/*--------------------------------------------------------------------------*/

int CThreadObject::StartThread(const char *argName, int argStackSize, int argPriority)
{
    int rc = ERR_NO_ERROR;
    pthread_attr_t attr;
    
    if (m_running == false)
    {
	m_stopRequested = false; // Clear previous requests

	MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_DEBUG, "CThreadObject::Starting thread - %s\n", argName);

        // Store name of thread
	if (argName != 0)
	{
	    m_threadName = argName;
	}
	
	if (argStackSize > 0)
	{
	    // Stacksize is given by caller, so change it
	    pthread_attr_init(&attr);
            rc = pthread_attr_setstacksize(&attr,argStackSize);
            if (rc == 0)
            {
        	MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_DEBUG, "CThreadObject::Set stacksize for thread successful\n");
            }
            else
            {
        	// Error code 22 = [EINVAL] = The value of stacksize is less than PTHREAD_STACK_MIN or exceeds a system-imposed limit.
        	MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_ERROR, "CThreadObject::Set stacksize for thread FAILED - rc=%d errno=%d name=%s\n", rc, errno, m_threadName.c_str());
            }
	    if (argPriority > 0)
	    {
		if (pthread_attr_setschedpolicy(&attr, SCHED_FIFO))
		{
		    MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_ERROR, "CThreadObject::Set schedule policy for thread failed\n");
    //^^^           ret = -1;
		}
		else
		{
		    MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_DEBUG, "CThreadObject::Set schedule policy SCHED_FIFO for thread successful\n");
		}
	    }
	    rc = pthread_create(&m_thread, &attr, &CThreadObject::StaticThreadFunction, this);
	}
	else
	{
	    rc = pthread_create(&m_thread, NULL, &CThreadObject::StaticThreadFunction, this);
	}
	// If successful, the pthread_create() function shall return zero;
	// otherwise, an error number shall be returned to indicate the error.

	if (rc == 0)
	{
	    MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_INFO, "CThreadObject::Create thread successful - rc=%d name=%s id=%ld type=%d  this=%p\n", rc, m_threadName.c_str(), m_thread, GetThreadType(), this);
	    m_running = true;

	    // Get current thread priority
	    sched_param param;
	    int priority;
	    int policy;
	    int ret;
	    /* scheduling parameters of target thread */
	    ret = pthread_getschedparam (m_thread, &policy, &param);
	    /* sched_priority contains the priority of the thread */
	    priority = param.sched_priority;
	    MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_DEBUG, "CThreadObject:: Thread priority = %d, ret = %d  id=%ld\n", priority, ret, m_thread);

	    int scheduler = sched_getscheduler(0);
	    MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_DEBUG, "CThreadObject:: Thread scheduler = %d  id=%ld\n", scheduler, m_thread);

	    if (argPriority > 0)
	    {	    
		// Set thread priority
		// /* sched_priority will be the priority of the thread */
		param.sched_priority = argPriority;
		/* only supported policy, others will result in ENOTSUP */
    //          policy = SCHED_OTHER;
		policy = SCHED_FIFO;
		/* scheduling parameters of target thread */
		ret = pthread_setschedparam(m_thread, policy, &param);
		MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_DEBUG, "CThreadObject:: Changed Thread priority = %d, ret = %d  id=%ld\n", argPriority, ret, m_thread);
	    }
	    // Get current thread priority

	    /* scheduling parameters of target thread */
	    ret = pthread_getschedparam (m_thread, &policy, &param);
	    /* sched_priority contains the priority of the thread */
	    priority = param.sched_priority;
	    MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_DEBUG, "CThreadObject:: Thread priority = %d, ret = %d id=%ld\n", priority, ret, m_thread);

	    size_t lSize;

	    pthread_attr_init(&attr);
            rc = pthread_attr_getstacksize(&attr,&lSize);
            if (rc == 0)
            {
        	MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_DEBUG, "CThreadObject::Get stacksize for thread successful = %d\n", lSize);
            }
            else
            {
        	// [EINVAL] = 22 = The value of stacksize is less than PTHREAD_STACK_MIN or exceeds a system-imposed limit.
        	MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_ERROR, "CThreadObject::Get stacksize for thread FAILED - rc=%d errno=%d name=%s\n", rc, errno, m_threadName.c_str());
            }

	    AddThreadToGlobalThreadList(this);
	}
	else
	{
	    MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_ERROR, "CThreadObject::Create thread FAILED - rc=%d errno=%d name=%s\n", rc, errno, m_threadName.c_str());
	    rc = ERR_THREAD_OBJ_START_THREAD_FAILED;
	}
    }
    else
    {
	MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_ERROR, "CThreadObject::Starting thread - Thread is already running - name=%s\n", m_threadName.c_str());
	// ?? ^^^
    }
    return rc;
}

/*--------------------------------------------------------------------------*/
/**
@brief	 StopThread
@param	 none
@return	 none

	 Requests termination of the thread.
	 Sets a flag to signal the thread work funktion that the thread
	 should be terminated and calls StopThreadAction() in derived class.
	 It's up to the (user defined) thread function to check this flag
	 and take the proper actions.

	 THE CALL DOES NOT WAIT UNTIL THE THREAD IS REALLY STOPPED!
*/
/*--------------------------------------------------------------------------*/

void CThreadObject::StopThread()
{
    MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_INFO3, "CThreadObject::StopThread() - Stopping thread\n");

    m_stopRequested = true;   // Just set the flag
}

/*--------------------------------------------------------------------------*/
/**
@brief	 StopThreadAndWait
@param	 none
@return	 none

	 Requests termination of the thread.
	 Sets a flag to signal the thread work funktion that the thread
	 should be terminated and calls StopThreadAction() in derived class.
	 It's up to the (user defined) thread function to check this flag
	 and take the proper actions.

	 THE CALL WAITS UNTIL THE THREAD IS REALLY STOPPED!
*/
/*--------------------------------------------------------------------------*/

void CThreadObject::StopThreadAndWait()
{
    MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_INFO2, "CThreadObject::StopThreadAndWait() - Stopping thread - m_thread = 0x%08lx = %s\n", m_thread, m_threadName.c_str());

    m_stopRequested = true;   // Just set the flag

    if (m_thread != 0)
    {
	// wait until the thread is stopped (see note below)
	pthread_join(m_thread, 0);
    }
    MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_INFO2, "CThreadObject::StopThreadAndWait() - Thread stopped - m_thread = 0x%08lx = %s\n", m_thread, m_threadName.c_str());

    m_thread = 0;
    m_running = false;       // Thread is finished
}

/*--------------------------------------------------------------------------*/
/**
@brief	 StaticThreadFunction
@param	 obj	Pointer to the thread object
@return	 none

	 This is just a utility function to call the RealThreadFunction of the
	 object instance. This is a static class function that serves as a
	 C style function pointer for the pthread_create call.
*/
/*--------------------------------------------------------------------------*/

void * CThreadObject::StaticThreadFunction(void *obj)
{
    //All we do here is call the RealThreadFunction() function
    reinterpret_cast<CThreadObject *>(obj)->RealThreadFunction();
    return NULL;
}

/*--------------------------------------------------------------------------*/
/**
@brief	 RealThreadFunction
@param	 none
@return	 none

	 This is the work function of the thread object.
	 Performs some actions before calling the work function of the derived
	 class.
	 - Sets the name of the thread
	 - Stores some info about the thread (process id, thread id)
*/
/*--------------------------------------------------------------------------*/

void CThreadObject::RealThreadFunction()
{
    if (!m_threadName.empty())
    {    
        // Set the name of the thread
        prctl(PR_SET_NAME, m_threadName.c_str(), 0, 0, 0, 0) ;

	// NOTE: This name will show up in the output of the "top" command,
	//       but not in the output of the "ps" command.
    }

    m_threadId = syscall(SYS_gettid);
    m_processId = getpid();

    MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_INFO, "CThreadObject::Calling ThreadWorkFunction - name=%s Thread-Id = %ld\n", m_threadName.c_str(), GetThreadId());
//    MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_INFO, "T self: %lu\n", pthread_self());
//    MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_INFO, "T tid: %lu\n", syscall(SYS_gettid));
//    MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_INFO, "T pid: %u\n", getpid());

    ThreadWorkFunction();    // Calls function in derived class

    MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_INFO, "CThreadObject::ThreadWorkFunction ended - name=%s\n", m_threadName.c_str());

    RemoveThreadFromGlobalThreadList(this);

    m_running = false;	     // Thread is finished
    m_thread = 0;
}

/*--------------------------------------------------------------------------*/
/**
@brief   GetMutex
@param   none
@return  none
@note    status: tested

         Requests the mutex. The mutex is automatically initialized if not yet done.
*/
/*--------------------------------------------------------------------------*/

void CThreadObject::GetMutex(void)
{
    if (m_metzGlobalThreadMutexInitialized == false)
    {
	m_metzGlobalThreadMutexInitialized = true;
	InitializeMutex(&m_metzGlobalThreadMutex);    
    }
    RequestMutex(&m_metzGlobalThreadMutex);
}

/*--------------------------------------------------------------------------*/
/*	INTERNAL CLASSES						    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*	DEBUG FUNCTIONS							    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/**
@brief   AddThreadToGlobalThreadList
@param   argThreadObjPtr	Ptr to the thread object
@return  none
@note    status: tested

         Adds an entry for this thread to the global thread list.
         The global thread list is mainly used for the "m_threadlist"
         mshell command.
*/
/*--------------------------------------------------------------------------*/

void CThreadObject::AddThreadToGlobalThreadList(CThreadObject * argThreadObjPtr)
{
    T_GlobalThreadInfo threadInfo = { argThreadObjPtr };

    GetMutex();
    
    // Insert at the beginning of the list
    globalThreadObjectList.push_front(threadInfo);

    ReleaseMutex(&m_metzGlobalThreadMutex);
}

/*--------------------------------------------------------------------------*/
/**
@brief   RemoveThreadFromGlobalThreadList
@param   argThreadObjPtr	Ptr to the thread object
@return  error code         (in case of success "ERR_NO_ERROR")
@note    status: tested

         Removes the entry for this thread from the global thread list.
*/
/*--------------------------------------------------------------------------*/

int CThreadObject::RemoveThreadFromGlobalThreadList(CThreadObject * argThreadObjPtr)
{
    std::list<T_GlobalThreadInfo>::iterator it;

    if (m_metzGlobalThreadHandlingTerminated == false) // Only if not disabled during program termination
    {
	GetMutex();
	// Search the element containing the object reference
	for (it = CThreadObject::globalThreadObjectList.begin(); it != CThreadObject::globalThreadObjectList.end(); it++)
	{
	    if (it->threadObjPtr == argThreadObjPtr)
	    {
		// As the element is found, remove it from the list and update the iterator
		it = CThreadObject::globalThreadObjectList.erase(it);
		// We removed it, so the job is done
		break;
	    }
	}

	ReleaseMutex(&m_metzGlobalThreadMutex);
    }
    return ERR_NO_ERROR;
}

/*--------------------------------------------------------------------------*/
/**
@brief   ListGlobalThreadObjectList
@param   none
@return  none
@note    status: tested

         Displayes a list of all thread objects with some additional info.
         Used for the corresponding mshell command.
*/
/*--------------------------------------------------------------------------*/

void CThreadObject::ListGlobalThreadObjectList(long argThreadSelection)
{
    std::list<T_GlobalThreadInfo>::iterator it;

    //int rc = 0;

    GetMutex();
    
    MPRINT("\nList of thread objects\n");
    MPRINT("\nTypes: 1 = while-loop, 2 = msg-loop, 3 = msg-loop with states\n\n");
    MPRINT("Thread-Addr | Thread-Id | Type | Msgs    | Name\n");
    MPRINT("------------+-----------+------+---------+-------------------------------------------\n");

    // Go through the list and print the elements
    // If argThreadSelection is not 0, then print only the matching ones.
    for (it = CThreadObject::globalThreadObjectList.begin(); it != CThreadObject::globalThreadObjectList.end(); it++)
    {
	if (   (argThreadSelection == 0)
            || ((argThreadSelection == (long) it->threadObjPtr->m_thread) || (argThreadSelection == it->threadObjPtr->m_threadId))
           )
	{
            MPRINT("0x%8lx | %9ld | %4d | %7d | %s\n", it->threadObjPtr->m_thread, it->threadObjPtr->m_threadId, it->threadObjPtr->GetThreadType(), it->threadObjPtr->GetNrOfProcessedMsgs(), it->threadObjPtr->m_threadName.c_str());
	}
    }
    MPRINT("------------+-----------+------+---------+-------------------------------------------\n");
    ReleaseMutex(&m_metzGlobalThreadMutex);
}

/*--------------------------------------------------------------------------*/
/**
@brief	 GetThreadName
@param	 threadid		Thread id
@return	 pointer to name string

	 Gets the name string of a thread.
*/
/*--------------------------------------------------------------------------*/

const char * CThreadObject::GetThreadName(unsigned int argThreadAddr)
{
    std::list<T_GlobalThreadInfo>::iterator it;
    const char * lResultPtr = myDefaultString;
    GetMutex();

    // Go through the list and look for the thread
    // If argThreadSelection is not 0, then print only the matching ones.
    for (it = CThreadObject::globalThreadObjectList.begin(); it != CThreadObject::globalThreadObjectList.end(); it++)
    {
	if (argThreadAddr == it->threadObjPtr->m_thread)
	{
            lResultPtr = it->threadObjPtr->m_threadName.c_str();
	}
    }

    ReleaseMutex(&m_metzGlobalThreadMutex);
    return lResultPtr;
}


/*--------------------------------------------------------------------------*/
/**
@brief	 Terminate
@param	 none
@return	 none

	 Sets the terminated flag.
	 This function may be called by a program just before exiting main().
	 It is a workaround to prevent the conflict between running the
	 destructors of the thread objects and the global thread queue object.
	 In the long run this problem should be solved in a different way.
*/
/*--------------------------------------------------------------------------*/

void CThreadObject::Terminate()
{
    m_metzGlobalThreadHandlingTerminated = true;
}

/*--------------------------------------------------------------------------*/
/**
@brief   StopAllThreads
@param   none
@return  error code         (in case of success "ERR_NO_ERROR")
@note    status: tested

         Removes the entry for this thread from the global thread list.
*/
/*--------------------------------------------------------------------------*/

void CThreadObject::StopAllThreads(void)
{
    std::list<T_GlobalThreadInfo>::iterator it;

        Terminate();    // To avoid deadlock by mutex

        GetMutex();
        // Search the element containing the object reference
        for (it = CThreadObject::globalThreadObjectList.begin(); it != CThreadObject::globalThreadObjectList.end(); it++)
        {
            if (it->threadObjPtr != 0)
            {
                CThreadObject * threadPtr = it->threadObjPtr;
                MTRACE_P(TRACE_THREADOBJ, TRACELEVEL_INFO3, "CThreadObject::StopAllThreads() - ++++ Stopping thread %s ++++\n", threadPtr->GetThreadName());
                threadPtr->StopThreadAndWait();
            }
        }

        ReleaseMutex(&m_metzGlobalThreadMutex);
}


/*--------------------------------------------------------------------------*/
/*	TEST FUNCTIONS							    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* end of  ThreadObject.cpp */

/*==========================================================================*/
#if 0
Todo:
^^^
/*
Notes:

The pthread_join() function suspends execution of the calling thread until
the target thread terminates, unless the target thread has already terminated.
On return from a successful pthread_join() call with a non-NULL value_ptr
argument, the value passed to pthread_exit() by the terminating thread
is made available in the location referenced by value_ptr. When a
pthread_join() returns successfully, the target thread has been terminated.
The results of multiple simultaneous calls to pthread_join() specifying the
same target thread are undefined. If the thread calling pthread_join() is
canceled, then the target thread will not be detached.

*/
#endif
