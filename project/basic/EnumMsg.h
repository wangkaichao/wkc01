#ifndef ENUM_MSG_H
#define ENUM_MSG_H

#ifdef ENUM_OR_STRING
#undef ENUM_OR_STRING
#endif
#define ENUM_OR_STRING(x) x

enum MSG_ID_E
{
#include "EnumMsg.em"
};

const char *GetMsgIdName(MSG_ID_E enMsgId);

#endif
