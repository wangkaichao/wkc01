
/**
* @file ThreadObject.h
*
* @brief ThreadObject	Base class for objects running in their own thread.
*
* @author Peter Wierzba
*
* $Revision: 1.10 $
* $Date: 2012/01/25 08:38:51CET $
*
* @note Copyright (c) 2009 Metz-Werke GmbH  CoKG \n
*	All rights reserved
*
* 	This base class can be used to derive classes where
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
/* Archiv: $Source: basics/src/ThreadObject.h $ */
/*==========================================================================*/

#ifndef __THREADOBJECT_H__
#define __THREADOBJECT_H__

/*--------------------------------------------------------------------------*/
/*	INCLUDE FILES							    */
/*--------------------------------------------------------------------------*/

#ifdef CC_PCTOOL
#else
#include <pthread.h>
#endif
#include <string>
#include <list>
#include "wm_log.h"
//#include "StdtypedefsNoWizard.h"
//#include "Stduser.h"	// for Mutex_Type
#include "MetzStdDefs.h"

/*--------------------------------------------------------------------------*/
/*	CONSTANTS							    */
/*--------------------------------------------------------------------------*/

// ERROR CODES for this unit (range: -1100 .. -1109)

// All error codes have negative values.
// ERR_NO_ERROR is used to indicate "no error" (value = 0)

#define ERR_THREAD_OBJ_START_THREAD_FAILED		     -1100 // 0xBB4

typedef enum
{
    THREAD_TYPE_NONE	        = 0,
    THREAD_TYPE_WORK_FUNCTION   = 1,
    THREAD_TYPE_WITH_QUEUE      = 2,
    THREAD_TYPE_WITH_STATES     = 3,
    THREAD_TYPE_WITH_QUEUE_RPC  = 4,
    THREAD_TYPE_BLOCKING_WAIT   = 5

} M_ThreadType;

/*--------------------------------------------------------------------------*/
/*	TYPE DEFINITIONS						    */
/*--------------------------------------------------------------------------*/

class CThreadObject;	// Forward declaration

// Structure which holds information about a thread object
typedef struct
{
    CThreadObject * threadObjPtr;

} T_GlobalThreadInfo;

/*--------------------------------------------------------------------------*/
/*	CLASS DECLARATIONS						    */
/*--------------------------------------------------------------------------*/

class CThreadObject
{
    // --- Member variables

protected:
    volatile bool	m_stopRequested; // ^^
    pthread_mutex_t	m_mutex;	 // A mutex for the thread
    int			m_dbgId;	 // Only used for debug purpose
    std::string	        m_threadName;

private:

    // Global list of pointers to all active threads
    static std::list<T_GlobalThreadInfo> globalThreadObjectList;

    // Mutex used for access of globalThreadObjectList
    static Mutex_Type m_metzGlobalThreadMutex;
    static bool	      m_metzGlobalThreadMutexInitialized;
    static bool	      m_metzGlobalThreadHandlingTerminated; // Workaround for the destructor problem

    pthread_t		m_thread;	 // The thread
    pthread_attr_t	m_threadAttr;
    pthread_mutexattr_t m_mutexAttr;
    volatile bool	m_running;
    long		m_threadId;
    unsigned long	m_processId;

    // --- Member functions

public:
    CThreadObject();			// Constructor
    CThreadObject(int argDbgId);	// Constructor with dbgId
    virtual ~CThreadObject();		// Destructor (ALLWAYS virtual)

    virtual int StartThread(const char *argName = 0, int argStackSize = 0, int argPriority = 0); // Starts the thread which executes RealThreadFunction()
    virtual void StopThread();        // Stops the thread. Returns immediately.
    virtual void StopThreadAndWait(); // Stops the thread. Defers the calling thread until it is really stopped.

    pthread_t GetThreadAddr(void) { return m_thread; };
    long GetThreadId(void) { return m_threadId; };
    virtual M_ThreadType GetThreadType(void) { return THREAD_TYPE_WORK_FUNCTION; };
    virtual unsigned int GetNrOfProcessedMsgs() const { return 0; }; // Makes only sense for msg threads

    static void ListGlobalThreadObjectList(long argThreadSelection = 0);
    static const char * GetThreadName(unsigned int argThreadAddr);
    const char * GetThreadName(void) { return m_threadName.c_str(); };
    static void Terminate(void);
    static void StopAllThreads(void);

protected:
    void LockThreadMutex(void)   { pthread_mutex_lock(&m_mutex); };
    void UnlockThreadMutex(void) { pthread_mutex_unlock(&m_mutex); };

    static void* StaticThreadFunction(void *obj); // Just a helper
						  // function to get
						  // from C -> C++
    void RealThreadFunction();	// Starts the work in the thread

    // This method has to be defined by the derived class.
    // It will be called once when the thread is started and performes the
    // real work.
    virtual void ThreadWorkFunction() = 0;

private:

    static void GetMutex(void);
    void AddThreadToGlobalThreadList(CThreadObject * argThreadObjPtr);
    int RemoveThreadFromGlobalThreadList(CThreadObject * argThreadObjPtr);
};

/*--------------------------------------------------------------------------*/

#endif	/* __THREADOBJECT_H__ */

/*--------------------------------------------------------------------------*/
/* end of ThreadObject.h */
