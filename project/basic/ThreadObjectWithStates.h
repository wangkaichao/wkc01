/**
* @file ThreadObjectWithStates.h
*
* @brief ThreadObjectWithStates
*
* @author Peter Wierzba
*
* $Revision: 1.2 $
* $Date: 2011/05/19 14:03:07GMT $
*
* @note Copyright (c) 2009 Metz-Werke GmbH  CoKG \n
*	All rights reserved
*
*	This base class can be used to derive classes where
*	each instance should run in a separate thread and
*	uses a state machine.
*
* Usage:	CTestThreadObjectWithStates myTO1(3);
*		myTO1.Init();
*		myTO1.StartThread();
*
* History:
*
* DD.MM.YY Who Ticket  Description
* 12.02.09 PWI 1       First Issue
*/
/* Archiv: $Source: basics/src/ThreadObjectWithStates.h $ */
/*==========================================================================*/

#ifndef __THREADOBJECTWITHSTATES_H__
#define __THREADOBJECTWITHSTATES_H__

/*--------------------------------------------------------------------------*/
/*	INCLUDE FILES							    */
/*--------------------------------------------------------------------------*/

#include <string>
#include "MetzMsgQueue.h"
#include "ThreadObjectWithQueue.h"

/*--------------------------------------------------------------------------*/
/*	CONSTANTS							    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*	TYPE DEFINITIONS						    */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/*	CLASS DECLARATIONS						    */
/*--------------------------------------------------------------------------*/

#define LEV1_ANY 0
#define LEV2_ANY 0
#define ANY_MESSAGE SIG_ID_NULL // Has to be a value not used for a message.


/*--------------------------------------------------------------------------*/

class CThreadObjectWithStates : public CThreadObjectWithQueue
{
protected:
    // Template used for defining the state table

    template<class GeneralState, class SubState, class SignalType, class Instance, class MsgType>

    struct StateTransition
    {
       GeneralState generalState;
       SubState	subState;
       SignalType	msgId;
       void (Instance::*callback)(MsgType *msg);
    };

    // --- Member variables

private:

protected:

    // --- Member functions

public:
    CThreadObjectWithStates();			// Constructor
    virtual ~CThreadObjectWithStates();		// Destructor (ALLWAYS virtual)

    virtual int	 Init(const char * argQueueNamePrefix, int argQueueSize);
    virtual M_ThreadType GetThreadType(void) { return THREAD_TYPE_WITH_STATES; };
    
private:

private:
};

/*--------------------------------------------------------------------------*/

#endif	/* __THREADOBJECTWITHSTATES_H__ */

/*--------------------------------------------------------------------------*/
/* end of ThreadObjectWithStates.h */
