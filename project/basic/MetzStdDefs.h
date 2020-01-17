/**
* @file MetzStdDefs.h
*
* @brief MetzStdDefs  Defines all types which are general and should be used as standard.
* @author Peter Wierzba
*
* $Revision: 1.00 $
* $Date: 2009/01/09 13:52:44GMT $
*
* @note Copyright (c) 2012 Metz-Werke GmbH  CoKG \n
*       All rights reserved
*
* Defines all types which are general and should be used as standard.
* Only things that are really(!) general should be defined here.
*/
/* Archiv: $Source:  $ */
/*==========================================================================*/

#ifndef METZSTDDEFS_H_121214
#define METZSTDDEFS_H_121214

/*--------------------------------------------------------------------------*/
/*	Include Files							    */
/*--------------------------------------------------------------------------*/

#if defined(__cplusplus)
#include <cstddef> // To avoid problems with redefinitions (size_t) done by "thal_cpucomm_api.h / thal_list.h"
#else
#include <stddef.h>
#endif

/*--------------------------------------------------------------------------*/
/*	Basic Constants							    */
/*--------------------------------------------------------------------------*/

#define ERR_NO_ERROR 0

#define DATA16_NOTSET  0xFFFF
#define DATAS32_NOTSET 0x7FFFFFFF

// Task priorities (range from -20 (highest) till +20 (lowest))
#define METZ_THREAD_PRIORITY_TIME_CRITICAL -20 //Indicates 20 points above normal priority.
#define METZ_THREAD_PRIORITY_HIGHEST        -2 //Indicates  2 points above normal priority.
#define METZ_THREAD_PRIORITY_ABOVE_NORMAL   -1 //Indicates  1 point above normal priority.
#define METZ_THREAD_PRIORITY_NORMAL          0 //Indicates normal priority.
#define METZ_THREAD_PRIORITY_BELOW_NORMAL    1 //Indicates  1 point below normal priority.
#define METZ_THREAD_PRIORITY_LOWEST          2 //Indicates  2 points below normal priority.

/* Queue priorities*/
#define M_SQ_PRIO_HIGH  1
#define M_SQ_PRIO_LOW   0

/* More */
#define NIL (( void *)  0 )

#ifndef NULL
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif
/*--------------------------------------------------------------------------*/
/*	Miscellaneous Constants						    */
/*--------------------------------------------------------------------------*/

/* defines for the function sleep(). */

#define SLEEP_1S 1

/* defines for the function usleep(). */

#define USLEEP_MS250 (250 * 1000)
#define USLEEP_MS200 (200 * 1000)
#define USLEEP_MS150 (150 * 1000)
#define USLEEP_MS100 (100 * 1000)
#define USLEEP_MS50  (50 * 1000)
#define USLEEP_MS30  (30 * 1000)
#define USLEEP_MS25  (25 * 1000)
#define USLEEP_MS24  (24 * 1000)
#define USLEEP_MS20  (20 * 1000)
#define USLEEP_MS10  (10 * 1000)

/*--------------------------------------------------------------------------*/
/*	Basic Data Types						    */
/*--------------------------------------------------------------------------*/

//^^ from StdtypedefsNoWizard.h
/* TYPE DEFINITIONS */
#ifndef _M_DEFTYPES_H
#define _M_DEFTYPES_H
typedef unsigned char  M_UChar;    /* 8 bit unsigned integer */
typedef unsigned short M_UShort;   /* 16 bit unsigned integer */
typedef unsigned short M_UInt16;   /* 16 bit unsigned integer */
typedef unsigned long  M_ULong;    /* 32 bit unsigned integer */
typedef unsigned long  M_UInt32;   /* 32 bit!! unsigned integer */

typedef signed char  M_Char;       /* 8 bit signed integer */
typedef signed short M_Short;      /* 16 bit signed integer */
typedef signed short M_Int16;      /* 16 bit signed integer */
typedef signed long  M_Long;       /* 32 bit signed integer */
typedef signed long  M_Int32;      /* 32 bit!! signed integer */

typedef M_UChar*     M_String8;    /*  */
//typedef enum { FALSE = ( 0 == 1 ), TRUE  = ( 1 == 1 ) } M_Boolean;
// FALSE AND TRUE is already defined in
// ".../trident/qx88/Kernel_Driver/Inc/typedef.h" included via "trid_datatype.h"
// Therefore it is NOT even possible to define a enum with the values
// FALSE and TRUE in an easy way
// typedef enum { FALSE, TRUE } M_Boolean;

typedef int M_Boolean; //^^^ pwi Nochmals nachdenken, wie man das geschickter machen kann.

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE !FALSE
#endif

#endif

#ifndef BIT0
/*~T*/
#define BIT0            0x000001U
#define BIT1            0x000002U
#define BIT2            0x000004U
#define BIT3            0x000008U
#define BIT4            0x000010U
#define BIT5            0x000020U
#define BIT6            0x000040U
#define BIT7            0x000080U

#define BIT8            0x000100U
#define BIT9            0x000200U
#define BIT10           0x000400U
#define BIT11           0x000800U
#define BIT12           0x001000U
#define BIT13           0x002000U
#define BIT14           0x004000U
#define BIT15           0x008000U

#define BIT16           0x010000U
#define BIT17           0x020000U
#define BIT18           0x040000U
#define BIT19           0x080000U
#define BIT20           0x100000U
#define BIT21           0x200000U
#define BIT22           0x400000U
#define BIT23           0x800000U

#define BIT24         0x01000000U
#define BIT25         0x02000000U
#define BIT26         0x04000000U
#define BIT27         0x08000000U
#define BIT28         0x10000000U
#define BIT29         0x20000000U
#define BIT30         0x40000000U
#define BIT31         0x80000000U

#endif

#define LOWLOWNIB   0x000fU
#define LOWHIGHNIB  0x00f0U

#define HIGHLOWNIB  0x0f00U
#define HIGHHIGHNIB 0xf000U

/*--------------------------------------------------------------------------*/
/*	Return Values							    */
/*--------------------------------------------------------------------------*/

#define ERR_NO_ERROR 0

// Next should not be used for further development
typedef enum {DONE, BUSY, READY_METZ, FAIL, DONE_NAME_CHANGED, RECORD /* needed for PathFinder*/, REC_TOO_LONG, MAX_RETURNVALUE} M_ReturnValue;

/*--------------------------------------------------------------------------*/
/*	Semaphores / Mutex								    */
/*--------------------------------------------------------------------------*/

/* Semaphores */
#define SEM_PSHARED 1  // non-zero to share semaphore between processes

/* Mutex Type*/
#define Mutex_Type              pthread_mutex_t

/* Mutex Makros */
#define InitializeMutex(cs)	pthread_mutex_init((cs), (NULL))  /* Init Mutex */
#define DeleteMutex(cs)		pthread_mutex_destroy(cs)         /* delete Mutex */
#define RequestMutex(cs)	pthread_mutex_lock(cs)            /* request Mutex */
#define ReleaseMutex(cs)	pthread_mutex_unlock(cs)          /* release Mutex */

/*--------------------------------------------------------------------------*/
/*	Miscellaneous Types						    */
/*--------------------------------------------------------------------------*/

typedef enum
{
   Uninstalled = 0,
   Installed   = 1,
   Operational = 2,
   Error       = 3
} M_OPERATIONALSTATE;

/*--------------------------------------------------------------------------*/
/*	Miscellaneous Macros						    */
/*--------------------------------------------------------------------------*/

#define __PACK_STRUCTURE__	__attribute__((packed))	    //^^^

#define SIZE(a,b) (sizeof(a) / sizeof(b))

#define MAXVAL(_x_,_y_) ((_x_) >= (_y_) ? (_x_) : (_y_))
#define MINVAL(_x_,_y_) ((_x_) <  (_y_) ? (_x_) : (_y_))

/* A macro to limit the value y to a maximum of x in which case defaults to z */
#define mLIMITED_TO_(x,y,z) (( y <= x ) ? y : z )

#ifdef __cplusplus
// A macro to disallow the copy constructor and operator= functions for a class.
// Useful to prevent unexpected use of default functions, provided by the compiler.
// This has be used in the "private" declarations for a class to work.
//
// Example:
//
//   private:
//	DISALLOW_COPY_AND_ASSIGN(MyClass);

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)
#endif

//-------------------------------------------------------------------------------------
//attention!!!: if you add or remove a country also change the marcos below, neccessary for rigth lto correction (Jan, 04.03.2005)

#define COUNTRY_STRINGS_MACRO \
ENUM_OR_STRING(GERMANY), \
ENUM_OR_STRING(AUSTRIA), \
ENUM_OR_STRING(SWITZERLAND_GERMAN), \
ENUM_OR_STRING(SWEDEN), \
ENUM_OR_STRING(NORWAY), \
ENUM_OR_STRING(FINLAND), \
ENUM_OR_STRING(DANMARK), \
ENUM_OR_STRING(BELGIUM_DUTCH), \
ENUM_OR_STRING(NETHERLANDS), \
ENUM_OR_STRING(ITALY), \
ENUM_OR_STRING(PORTUGAL), \
ENUM_OR_STRING(SPAIN), \
ENUM_OR_STRING(FRANCE), \
ENUM_OR_STRING(OTHER_COUNTRY), \
ENUM_OR_STRING(GREAT_BRITAIN), \
ENUM_OR_STRING(BELGIUM_FRENCH), \
ENUM_OR_STRING(BELGIUM_GERMAN), \
ENUM_OR_STRING(SWITZERLAND_FRENCH), \
ENUM_OR_STRING(SWITZERLAND_ITALY), \
ENUM_OR_STRING(POLAND), \
ENUM_OR_STRING(SWITZERLAND), \
ENUM_OR_STRING(BELGIUM), \
ENUM_OR_STRING(IRELAND), \
ENUM_OR_STRING(EAST_EUROPE), \
ENUM_OR_STRING(GREECE), \
ENUM_OR_STRING(M_CZECH), \
ENUM_OR_STRING(HUNGARY), \
ENUM_OR_STRING(AUSTRALIA), \
ENUM_OR_STRING(BALTIKUM), \
ENUM_OR_STRING(NEWZEALAND), \
ENUM_OR_STRING(SLOWAKEI), \
ENUM_OR_STRING(TURKEY), \
ENUM_OR_STRING(LUXEMBOURG), \
ENUM_OR_STRING(CROATIA), \
ENUM_OR_STRING(ICELAND), \
ENUM_OR_STRING(SLOVENIA), \
ENUM_OR_STRING(BUNDESTAG), \
ENUM_OR_STRING(UKRAINE), \
ENUM_OR_STRING(RUSSIA), \
ENUM_OR_STRING(SINGAPORE), \
ENUM_OR_STRING(MALAYSIA), \
ENUM_OR_STRING(HONG_KONG), \
ENUM_OR_STRING(CHINA), \
ENUM_OR_STRING(MAX_COUNTRY)

typedef enum {                     // Don't change order, major for DataLogic
#undef ENUM_OR_STRING
#define ENUM_OR_STRING(a) a
   COUNTRY_STRINGS_MACRO
} M_TVCountry;        // OTHER_COUNTRY = 13

//-------------------------------------------------------------------------------------
typedef enum {
               AUSTRALIAN_STATE_NOT_SET,
               WESTERN_AUSTRALIA,
               NORTHERN_TERRITORY,
               SOUTH_AUSTRALIA,
               BROKEN_HILL,
               QUEENSLAND,
               NEW_SOUTH_WALES,
               VICTORIA,
               TASMANIA,
               CAPITAL_TERRITORY,
               MAX_AUSTRALIAN_STATE,
} M_AustralianState;
//-------------------------------------------------------------------------------------
typedef enum {
#ifdef CC_TBD
               NO_TVMODE   = OsdAppTvModesNO_MODE,
               TV_MODE     = OsdAppTvModesTV_MODE,
               RADIO_MODE  = OsdAppTvModesRADIO_MODE,
               AV_MODE     = OsdAppTvModesAV_MODE,
               MAX_TV_MODE = OsdAppTvModesMAX_TV_MODE
#else
               NO_TVMODE   = 0,
               TV_MODE     = 1,
               RADIO_MODE  = 2,
               AV_MODE     = 3,
               MAX_TV_MODE
#endif
} M_TvMode;
//-------------------------------------------------------------------------------------
typedef enum{
#ifdef CC_TBD
               NO_PROG          = 0,
               TERR_PROG =        OsdAppProgTypesTERR_PROG,
               SAT_PROG =         OsdAppProgTypesSAT_PROG,
               DVB_S_PROG =       OsdAppProgTypesDVB_S_PROG,
               DVB_T_PROG =       OsdAppProgTypesDVB_T_PROG,
               DVB_C_PROG =       OsdAppProgTypesDVB_C_PROG,
               RADIO_PROG =       OsdAppProgTypesRADIO_PROG,
               ADR_PROG =         OsdAppProgTypesADR_PROG,
               DVB_S_RADIO_PROG = OsdAppProgTypesDVB_S_RADIO_PROG,
               DVB_T_RADIO_PROG = OsdAppProgTypesDVB_T_RADIO_PROG,
               DVB_C_RADIO_PROG = OsdAppProgTypesDVB_C_RADIO_PROG,
               AV_PROG =          OsdAppProgTypesAV_PROG,
               MAX_PROGTYPE =     OsdAppProgTypesMAX_PROG_TYPE
#else
               NO_PROG          = 0,
               TERR_PROG =        1,
               SAT_PROG =         2,
               DVB_S_PROG =       3,
               DVB_T_PROG =       4,
               DVB_C_PROG =       5,
               RADIO_PROG =       6,
               ADR_PROG =         7,
               DVB_S_RADIO_PROG = 8,
               DVB_T_RADIO_PROG = 9,
               DVB_C_RADIO_PROG = 10,
               AV_PROG =          11,
               MAX_PROGTYPE
#endif
} M_ProgType;
//-------------------------------------------------------------------------------------

#define BUF_GET_UINT32(b) ((b)[0]<<24|(b)[1]<<16|(b)[2]<<8|(b)[3])
#define BUF_GET_UINT16(b) (                      (b)[0]<<8|(b)[1])
#define BUF_GET_UINT8(b)  (                                (b)[0])

#define BUF_PUT_UINT32(b,v) do { (b)[0]=(unsigned char)(((v)>>24) & 0xFF);\
                                 (b)[1]=(unsigned char)(((v)>>16) & 0xFF);\
                                 (b)[2]=(unsigned char)(((v)>>8)  & 0xFF);\
                                 (b)[3]=(unsigned char)( (v)      & 0xFF); } while(0)

#define BUF_PUT_UINT16(b,v) do { (b)[0]=(unsigned char)(((v)>>8) & 0xFF);\
                                 (b)[1]=(unsigned char)( (v)     & 0xFF); } while(0)

#define BUF_PUT_UINT8(b,v)  do { (b)[0]=(unsigned char)(v); } while(0)

#define HIGHBYTE16(i) (unsigned char)(((i)>>8) & 0xFF)
#define  LOWBYTE16(i) (unsigned char)((i) & 0xFF)

//-------------------------------------------------------------------------------------


#endif // METZSTDDEFS_H_121214
