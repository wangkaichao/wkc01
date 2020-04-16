#include "EnumMsg.h"

#ifdef ENUM_OR_STRING
#undef ENUM_OR_STRING
#endif
#define ENUM_OR_STRING(x) #x

static const char *MsgIdName[] =
{
#include "EnumMsg.em"
};

const char *GetMsgIdName(MSG_ID_E enMsgId)
{
    if (enMsgId >= MSG_ID_NULL && enMsgId < MSG_ID_MAX)
    {
        return MsgIdName[enMsgId];
    }
    else
    {
        return MsgIdName[MSG_ID_NULL];
    }
}

