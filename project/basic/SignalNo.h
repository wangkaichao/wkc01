/**
* @file SignalNo.h        List of all used signal numbers
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
/* Archiv: $Source: defines/SignalNo.h $ */

#ifndef SIGNALNO_H_130122
#define SIGNALNO_H_130122

#ifdef ENUM_OR_STRING
#undef ENUM_OR_STRING
#endif
#define ENUM_OR_STRING( x ) x

enum SignalIds{
#include "SignalNo.gen"
};

typedef enum SignalIds M_ESignalId;

const char * GetSignalIdNamePtr(M_ESignalId aSignalId);

#endif
