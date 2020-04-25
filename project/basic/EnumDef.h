#ifndef ENUM_DEF_H
#define ENUM_DEF_H

#ifdef ENUM_OR_STRING
#undef ENUM_OR_STRING
#endif
#define ENUM_OR_STRING(x) x

#define MSG_ID_MACRO    \
    ENUM_OR_STRING(MSG_ID_NULL),         /* avoid msg id with No.0 */   \
    ENUM_OR_STRING(MSG_ID_STOP_THREAD),                                 \
    ENUM_OR_STRING(MSG_ID_BUTT)

enum MSG_ID_E
{
    MSG_ID_MACRO
};

const char *GetMsgIdName(MSG_ID_E enMsgId);

#define BUG_UNITS_MACRO \
    ENUM_OR_STRING(BUG_NULL),       \
    ENUM_OR_STRING(BUG_THREAD),     \
    ENUM_OR_STRING(BUG_MSGQUEUE),   \
    ENUM_OR_STRING(BUG_LOG),        \
    ENUM_OR_STRING(BUG_BUTT)

enum BUG_UNITS_E
{
    BUG_UNITS_MACRO
};

const char *GetBugUnitsName(BUG_UNITS_E enBugUnits);

#endif
