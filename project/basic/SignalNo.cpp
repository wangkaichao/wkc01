/**
* @file SignalNo.cpp        List of all used signal numbers
*
* @author Peter Wierzba
*
* $Revision: 1.0 $
* $Date: 2010/05/26 07:11:38GMT $
*
* @note Copyright (c) 2013 Metz-Werke GmbH  CoKG \n
*       All rights reserved
*
*/
/* Archiv: $Source: defines/SignalNo.cpp $ */

#include "SignalNo.h"

#ifdef ENUM_OR_STRING
#undef ENUM_OR_STRING
#endif
#define ENUM_OR_STRING( x ) #x

static const char * SignalIdStringArray[] =
{
#include "SignalNo.gen"
};

#define mCheckRangeSignalId(x) ((x >= SIG_ID_NULL) && (x < SIG_ID_MAX))

const char * GetSignalIdNamePtr(M_ESignalId aSignalId)
{
   if(!mCheckRangeSignalId(aSignalId))
   {
      return SignalIdStringArray[SIG_ID_NULL];
   }
   else
   {
      return SignalIdStringArray[aSignalId];
   }
}
