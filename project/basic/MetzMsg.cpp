/**
* @file	 MetzMsg.cpp
*
* @brief MetzMsg	Class which represents a Metz-Message.
*
* @author  Peter Wierzba
*
* $Revision: 1.17 $
* $Date: 2011/09/16 13:28:30GMT $
*
* @note Copyright (c) 2010 Metz-Werke GmbH  CoKG
*	All rights reserved
*
* This class represents a Metz-Message.
*
* Note:	If sending of a message fails the attached signal is automatically
* 	freed, if present.
*
* History:
*
* DD.MM.YY Who		Ticket	Description
* 12.02.09 P. Wierzba		First Issue
*
* TODO:
*       - Before indirect sending a message, we should check if the
*         destination is really a queue (e.g. via a magic).
*/
/* Archiv: $Source: communication/src/MetzMsg.cpp $ */
/*==========================================================================*/

/*--------------------------------------------------------------------------*/
/*	INCLUDE FILES							    */
/*--------------------------------------------------------------------------*/

//#include "MetzTrace.h"
//#include "MetzTraceUnitsGeneral.h"
#include "MetzMsg.h"
#include "MetzMsgQueue.h"

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
/* CMetzMsg								    */
/*==========================================================================*/

CMetzMsg::CMetzMsg()				// Constructor
{
    // Just init some values 	(should we use memset() here ?)
    data.SignalName = 0;
    data.SignalData.longData = 0;
    data.ResponseQueueHandle = 0;
    data.ResponseInfo1.longData = 0;
    data.ResponseInfo2.longData = 0;
    data.callbackId = 0;
    data.callbackId2 = 0;
    data.pSignal = 0;
    data.signalSize = 0;
    data.pSignalObj = 0;
    data.seqCtr = 0;
    data.auxData1.longData = 0;
}

/*--------------------------------------------------------------------------*/

CMetzMsg::CMetzMsg(unsigned long argSignalName)	// Constructor
{
    // Just init some values
    data.SignalName = argSignalName;
    data.SignalData.longData = 0;
    data.ResponseQueueHandle = 0;
    data.ResponseInfo1.longData = 0;
    data.ResponseInfo2.longData = 0;
    data.callbackId = 0;
    data.callbackId2 = 0;
    data.pSignal = 0;
    data.signalSize = 0;
    data.pSignalObj = 0;
    data.seqCtr = 0;
    data.auxData1.longData = 0;
}

/*--------------------------------------------------------------------------*/

CMetzMsg::~CMetzMsg()				// Destructor
{
    // Do NOT try to free the memory pointed to by pSignal, as this will be
    // done by the receiver of the message.
}

/*--------------------------------------------------------------------------*/

CMetzMsg::CMetzMsg(const CMetzMsg &origMsg)	// Copy-Constructor
{
    MTRACE_P(TRACE_METZMSG, TRACELEVEL_DEBUG, "CMetzMsg::Copy-Constructor called -- this=%p\n", this);
    data.pSignal = 0;		// Clear pointers to attached memory, as CopyMsg calls FreeSignal()
    data.signalSize = 0;
    data.pSignalObj = 0;
    CopyMsg(origMsg);
}

/*--------------------------------------------------------------------------*/

CMetzMsg & CMetzMsg::operator=(const CMetzMsg &origMsg) // Assignment-Operator
{
    MTRACE_P(TRACE_METZMSG, TRACELEVEL_DEBUG, "CMetzMsg::operator= called -- this=%p\n", this);
    if (this == &origMsg)
    {
	// Self-assignment -> do nothing
    }
    else
    {
	// FreeSignal(); // Is now called inside CopyMsg()
	CopyMsg(origMsg);
    }
    return *this;
}

/*--------------------------------------------------------------------------*/

int CMetzMsg::CopyMsg(const CMetzMsg &origMsg)
{
    int rc = ERR_NO_ERROR;
    unsigned char *charPtr;
    unsigned char *charPtr2;
    MTRACE_P(TRACE_METZMSG, TRACELEVEL_DEBUG, "CMetzMsg::CopyMsg called -- this=%p\n", this);

    FreeSignal(); // Free attached memory before attaching new

    data.SignalName = origMsg.data.SignalName;
    data.SignalData.longData = origMsg.data.SignalData.longData;
    data.ResponseQueueHandle = origMsg.data.ResponseQueueHandle;
    data.ResponseInfo1.longData = origMsg.data.ResponseInfo1.longData;
    data.ResponseInfo2.longData = origMsg.data.ResponseInfo2.longData;
    data.callbackId = origMsg.data.callbackId;
    data.callbackId2 = origMsg.data.callbackId2;
    data.signalSize = origMsg.data.signalSize;
    data.seqCtr = origMsg.data.seqCtr;
    data.auxData1.longData = origMsg.data.auxData1.longData;


    // If there is a signal structure attached, then copy it
    if ((origMsg.data.pSignal != 0) && (origMsg.data.signalSize > 0))
    {
	// The following is not elegant. Should we use a byte pointer instead of a void pointer?
	charPtr = (unsigned char*) operator new(data.signalSize);
	if (charPtr != 0)
	{
	    charPtr2 = (unsigned char *) origMsg.data.pSignal;

	    for (int i = 0; i < origMsg.data.signalSize; i++)
	    {
		charPtr[i] = charPtr2[i];
	    }
	    data.pSignal = (void *) charPtr;
	    MTRACE_P(TRACE_METZMSG, TRACELEVEL_DEBUG, "CMetzMsg::CopyMsg -- signal struct copied -- copied struct=%p, this=%p\n",
						      data.pSignal, this);

	    // There are a few signal structures which have additional pointers to allocated memory.
	    // This has to be copied also.
#if 0 //^^^^^^^^^^^Should be avoided in Fusion SW

	    if (data.SignalName == SIG_ID_TVEXTCOMMANDSIG)
	    {
		M_TvExtCommandSig *srcPtr  = reinterpret_cast<M_TvExtCommandSig *>(origMsg.data.pSignal);
		M_TvExtCommandSig *destPtr = reinterpret_cast<M_TvExtCommandSig *>(data.pSignal);
		destPtr->Text = 0;
		if (srcPtr->Text != 0)
		{
		    // Copy the text
		    unsigned long textLth = 0;
		    char *pStr = NULL;
		    textLth = strlen(srcPtr->Text);
		    if (textLth > 0)
		    {
			pStr = new char[textLth + 1];     // allocate memory for the string
			if (pStr != NULL)
			{
			    strncpy( pStr, srcPtr->Text, textLth+1 );
			    destPtr->Text = pStr;
			    MTRACE_P(TRACE_METZMSG, TRACELEVEL_DEBUG,
				     "CMetzMsg::CopyMsg -- text in signal struct copied - srcPtr=%p, destPtr=%p, srcPtr->Text=%s, destPtr->Text=%s, this=%p\n",
				     srcPtr->Text, destPtr->Text, srcPtr->Text, destPtr->Text, this);
			}
		    }
		}
	    }
	    else if (data.SignalName == SIG_ID_TVPSCOMMANDSIG)
	    {
		M_TvPSCommandSig *srcPtr  = reinterpret_cast<M_TvPSCommandSig *>(origMsg.data.pSignal);
		M_TvPSCommandSig *destPtr = reinterpret_cast<M_TvPSCommandSig *>(data.pSignal);
		destPtr->Text = 0;
		if (srcPtr->Text != 0)
		{
		    // Copy the text
		    unsigned long textLth = 0;
		    char *pStr = NULL;
		    textLth = strlen(srcPtr->Text);
		    if (textLth > 0)
		    {
			pStr = new char[textLth + 1];     // allocate memory for the string
			if (pStr != NULL)
			{
			    strncpy( pStr, srcPtr->Text, textLth+1 );
			    destPtr->Text = pStr;
			    MTRACE_P(TRACE_METZMSG, TRACELEVEL_DEBUG,
				     "CMetzMsg::CopyMsg -- text in signal struct copied - srcPtr=%p, destPtr=%p, srcPtr->Text=%s, destPtr->Text=%s, this=%p\n",
				     srcPtr->Text, destPtr->Text, srcPtr->Text, destPtr->Text, this);
			}
		    }
		}
	    }
#endif
	}
	else
	{
	    // Alloc failed - Clear pointer, means do not copy the attached signal
	    data.pSignal = 0;
	    data.signalSize = 0;
	    MTRACE_P(TRACE_METZMSG, TRACELEVEL_ERROR, "CMetzMsg::CopyMsg - Signal = %ld - Allocation failed\n", origMsg.data.SignalName);
	    rc = ERR_METZ_MSG_COPY_MSG_ALLOC_FAILED;
	}
    }
    else if ((origMsg.data.pSignal != 0) && (origMsg.data.signalSize == 0))
    {
	// This should not happen
        // Clear pointer, means do not copy the attached signal
	data.pSignal = 0;
	data.signalSize = 0;
	MTRACE_P(TRACE_METZMSG, TRACELEVEL_ERROR, "CMetzMsg::CopyMsg - Signal = %ld - Signal pointer is set, but signalSize is 0\n", origMsg.data.SignalName);
	rc = ERR_METZ_MSG_SIGNALSIZE_NOT_SET;
    }
    else
    {
	// No signal is attached. Clear fields (just to be sure).
	data.pSignal = 0;
	data.signalSize = 0;
    }

    // If there is a signal object attached, then copy it
    if (origMsg.data.pSignalObj != 0)
    {
	data.pSignalObj = origMsg.data.pSignalObj->cloneSignalObj();
        MTRACE_P(TRACE_METZMSG, TRACELEVEL_DEBUG, "CMetzMsg::CopyMsg -- signal object copied -- copied obj=%p, this=%p\n",
						  data.pSignalObj, this);
    }
    else
    {
	data.pSignalObj = 0;
    }

    return rc;
}

/*--------------------------------------------------------------------------*/

int CMetzMsg::ReceiveMsg(CMetzMsgQueue *argMsgQueue, int argPrio)
{
    int rc = ERR_NO_ERROR;
    // As this function is sometimes called in while(1)-loops for reading
    // non-blocking queues, the following trace will be printed too often.
    // Therefore it has been disabled.
    // MTRACE_P(TRACE_METZMSG, TRACELEVEL_DEBUG, "CMetzMsg::receive called - msg_queue=%p\n",argMsgQueue);
    rc = argMsgQueue->Receive(this);
    return rc;
}

/*--------------------------------------------------------------------------*/

int CMetzMsg::ReceiveMsgFromNonblockingQueue(CMetzMsgQueue *argMsgQueue, int argPrio)
{
    int rc = ERR_NO_ERROR;
    MTRACE_P(TRACE_METZMSG, TRACELEVEL_DEBUG, "CMetzMsg::receive called - msg_queue=%p\n",argMsgQueue);
    rc = argMsgQueue->ReceiveFromNonblockingQueue(this);    
    return rc;
}

/*--------------------------------------------------------------------------*/

int CMetzMsg::SendMsg(CMetzMsgQueue *argMsgQueue, int argPrio )
{
    int rc = ERR_NO_ERROR;
    MTRACE_P(TRACE_METZMSG, TRACELEVEL_INFO, "CMetzMsg::SendMsg1 called - msg_queue=%p Signal=%ld Signal-Ptr=%p this=%p\n",argMsgQueue,data.SignalName, data.pSignal, this);
    rc = CMetzMsgQueue::Send(argMsgQueue->GetQueueHandle(), this, argPrio);

    if (rc != ERR_NO_ERROR)
    {
	FreeSignal();	// Free the attached memory if sending was unsuccessful
    }
    return rc;
}

/*--------------------------------------------------------------------------*/

int CMetzMsg::SendMsg(mqd_t argMsgQueueHandle, int argPrio)
{
    int rc = ERR_NO_ERROR;
    MTRACE_P(TRACE_METZMSG, TRACELEVEL_INFO, "CMetzMsg::SendMsg2 called - msg_queue=%d Signal=%ld Signal-Ptr=%p this=%p\n",argMsgQueueHandle,data.SignalName, data.pSignal, this);
    // Send to the queue handle given as parameter
    rc = CMetzMsgQueue::Send(argMsgQueueHandle, this, argPrio);

    if (rc != ERR_NO_ERROR)
    {
	FreeSignal();	// Free the attached memory if sending was unsuccessful
    }
    return rc;
}

/*--------------------------------------------------------------------------*/

int CMetzMsg::SendMsgIndirect(int argPrio)
{
    int rc = ERR_NO_ERROR;
    if (data.ResponseQueueHandle != 0)
    {
	MTRACE_P(TRACE_METZMSG, TRACELEVEL_INFO, "CMetzMsg::SendMsgIndirect called - msg_queue=%d Signal=%ld Signal-Ptr=%p this=%p\n",data.ResponseQueueHandle,data.SignalName, data.pSignal, this);
	// Send to the queue handle given in the message
	rc = CMetzMsgQueue::Send(data.ResponseQueueHandle, this, argPrio);
    }
    else
    {
	MTRACE_P(TRACE_METZMSG, TRACELEVEL_DEBUG, "Response queue not set");
	rc = ERR_METZ_MSG_RESPONSE_QUEUE_NOT_SET;
    }

    if (rc != ERR_NO_ERROR)
    {
	FreeSignal();	// Free the attached memory if sending was unsuccessful
    }
    return rc;
}

/*--------------------------------------------------------------------------*/
/**
@brief   FreeSignal
@details Frees all signals attached to the message
@param   none
@return  none
*/
/*--------------------------------------------------------------------------*/

void CMetzMsg::FreeSignal()
{
    if (data.pSignal != 0)
    {
	MTRACE_P(TRACE_METZMSG, TRACELEVEL_INFO, "CMetzMsg::FreeSignal Signal=%ld Signal-Ptr=%p this=%p\n",data.SignalName, data.pSignal, this);

	// There are a few signal structures which have additional pointers to allocated memory.
	// This has to be freed also.
#if 0 //^^^^^^^^^^^Should be avoided in Fusion SW
	if (data.SignalName == SIG_ID_TVEXTCOMMANDSIG)
	{
	    M_TvExtCommandSig *srcPtr  = reinterpret_cast<M_TvExtCommandSig *>(data.pSignal);
	    if (srcPtr->Text != 0)
	    {
		// Delete the text memory
		delete [] srcPtr->Text;
		srcPtr->Text = 0;
	    }
	}
	else if (data.SignalName == SIG_ID_TVPSCOMMANDSIG)
	{
	    M_TvPSCommandSig *srcPtr  = reinterpret_cast<M_TvPSCommandSig *>(data.pSignal);
	    if (srcPtr->Text != 0)
	    {
		// Delete the text memory
		delete [] srcPtr->Text;
		srcPtr->Text = 0;
	    }
	}
#endif
	// Free the signal structure
	char *charPtr = (char *) data.pSignal; // temp pointer
     // delete data.pSignal;		       // deleting void* is not allowed
	delete charPtr;			       // Does this trick really works ? ^^^^
        data.pSignal = 0;		       // Clear pointer
    }

    if (data.pSignalObj != 0)
    {
	MTRACE_P(TRACE_METZMSG, TRACELEVEL_DEBUG, "CMetzMsg::FreeSignal Obj-Ptr=%p this=%p\n", data.pSignalObj, this);

	delete data.pSignalObj;	 // This calls the destructor of the derived class
        data.pSignalObj = 0;	 // Clear pointer
    }
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
/* end of  MetzMsg.cpp */
