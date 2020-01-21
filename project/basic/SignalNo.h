#ifndef SIGNALNO_H
#define SIGNALNO_H

#ifdef ENUM_OR_STRING
#undef ENUM_OR_STRING
#endif
#define ENUM_OR_STRING(x) x

enum SignalIds
{
#include "SignalNo.gen"
};

typedef enum SignalIds SIGNAL_ID_E;

const char *GetSignalIdName(SIGNAL_ID_E enSigId);

#endif
