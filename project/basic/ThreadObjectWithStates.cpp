/**
* @file	 ThreadObjectWithStates.cpp
*
* @brief  ThreadObjectWithStates
*
* @author  Peter Wierzba
*
* $Revision: 1.1 $
* $Date: 2010/05/18 07:30:53GMT $
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
/* Archiv: $Source: basics/src/ThreadObjectWithStates.cpp $ */
/*==========================================================================*/
// Todo : Einrueckungen schoener machen (4er)
/*--------------------------------------------------------------------------*/
/*	INCLUDE FILES							    */
/*--------------------------------------------------------------------------*/

#include <errno.h>
#include <iostream>
#include <ostream>
#include <sstream>

//#include "MetzTrace.h"
#include "ThreadObjectWithStates.h"

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
/* CThreadObjectWithStates						    */
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

CThreadObjectWithStates::CThreadObjectWithStates()
			    : CThreadObjectWithQueue() // Call constructor of baseclass
{

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

CThreadObjectWithStates::~CThreadObjectWithStates()
{
    // Note: Destructors of the base classes are called automatically.
}

/*--------------------------------------------------------------------------*/
/**
@brief	 Init
@param	 argQueueNamePrefix	Meaningful string for naming the input queue.
@param	 argQueueSize		Maximum number of messages in the queue.
@return	 none
@note	 status: tested

	 Initializes the object. Has to be called before any usage.
*/
/*--------------------------------------------------------------------------*/

int CThreadObjectWithStates::Init(const char * argQueueNamePrefix, int argQueueSize)
{
    int rc = 0;

    rc = CThreadObjectWithQueue::Init(argQueueNamePrefix, argQueueSize);
    // An error trace is already printed by the base class

    return rc;
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
/* end of  ThreadObjectWithStates.cpp */
