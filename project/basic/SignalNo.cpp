#include "SignalNo.h"

#ifdef ENUM_OR_STRING
#undef ENUM_OR_STRING
#endif
#define ENUM_OR_STRING(x) #x

static const char *SignalIdName[] =
{
#include "SignalNo.gen"
};

const char *GetSignalIdName(SIGNAL_ID_E enSigId)
{
    if (enSigId >= SIG_ID_NULL && enSigId < SIG_ID_MAX)
    {
        return SignalIdName[enSigId];
    }
    else
    {
        return SignalIdName[SIG_ID_NULL];
    }
}

