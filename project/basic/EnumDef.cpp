#include "EnumDef.h"

#ifdef ENUM_OR_STRING
#undef ENUM_OR_STRING
#endif
#define ENUM_OR_STRING(x) #x

static const char *MsgIdName[] =
{
    MSG_ID_MACRO
};

const char *GetMsgIdName(MSG_ID_E enMsgId)
{
    if (enMsgId >= MSG_ID_NULL && enMsgId < MSG_ID_BUTT)
    {
        return MsgIdName[enMsgId];
    }
    else
    {
        return MsgIdName[MSG_ID_NULL];
    }
}

static const char *BugUnitsName[] = 
{
    BUG_UNITS_MACRO
};

const char *GetBugUnitsName(BUG_UNITS_E enBugUnits)
{
    if (enBugUnits >= BUG_NULL && enBugUnits < BUG_BUTT)
    {
        return BugUnitsName[enBugUnits];
    }
    else
    {
        return BugUnitsName[BUG_BUTT];
    }
}

