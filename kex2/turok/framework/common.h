// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2007-2012 Samuel Villarreal
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
//-----------------------------------------------------------------------------

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <new>
#include "SDL_endian.h"
#include "shared.h"

#ifdef _WIN32
#include "opndir.h"
#endif

#include "mathlib.h"
#include "system.h"
#include "object.h"
#include "memHeap.h"

#define RGBA(r,g,b,a) ((rcolor)((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))

#define COLOR_WHITE         RGBA(0xFF, 0xFF, 0xFF, 0xFF)
#define COLOR_WHITE_A(a)    RGBA(0xFF, 0xFF, 0xFF, a)
#define COLOR_RED           RGBA(0xFF, 0, 0, 0xFF)
#define COLOR_GREEN         RGBA(0, 0xFF, 0, 0xFF)
#define COLOR_YELLOW        RGBA(0xFF, 0xFF, 0, 0xFF)
#define COLOR_CYAN          RGBA(0, 0xFF, 0xFF, 0xFF)

//
// ENDIAN SWAPS
//
#define Com_SwapLE16(x)   SDL_SwapLE16(x)
#define Com_SwapLE32(x)   SDL_SwapLE32(x)
#define Com_SwapBE16(x)   SDL_SwapBE16(x)
#define Com_SwapBE32(x)   SDL_SwapBE32(x)

// Defines for checking the endianness of the system.

#if SDL_BYTEORDER == SYS_LIL_ENDIAN
#define SYS_LITTLE_ENDIAN
#elif SDL_BYTEORDER == SYS_BIG_ENDIAN
#define SYS_BIG_ENDIAN
#endif

//
// SYSTEM
//
#ifdef _WIN32
void Sys_ShowConsole(kbool show);
void Sys_SpawnConsole(void);
void Sys_DestroyConsole(void);
void Sys_UpdateConsole(void);
void Sys_Printf(const char *string);
void Sys_Error(const char *string);
#endif

//
// COMMON
//

extern  int	myargc;
extern  char** myargv;

char *kva(char *str, ...);
kbool fcmp(float f1, float f2);

class kexCommon {
public:
    void            Printf(const char *string, ...);
    void            Printf(const kexStr &str);
    void            CPrintf(rcolor color, const char *string, ...);
    void            Warning(const char *string, ...);
    void            Warning(const kexStr &str);
    void            DPrintf(const char *string, ...);
    void            Error(const char *string, ...);
    int             CheckParam(const char *check);
    void            Shutdown(void);
    void            WriteConfigFile(void);
    void            ReadConfigFile(const char *file);
    unsigned int    HashFileName(const char *name);
    void            AddCvar(const kexStr &name, const kexStr &value, const kexStr &desc, const int flags);
    float           GetCvarFloat(const kexStr &name);
    bool            GetCvarBool(const kexStr &name);

    static void     InitObject(void);

private:
    char            buffer[4096];
};

extern kexCommon common;

#include "cmd.h"
#include "cvar.h"

extern kexCvar cvarDeveloper;

#define NETBACKUPS      64
#define MAX_PLAYERS     8

typedef struct
{
    int ingoing;
    int outgoing;
    int acks;
} netsequence_t;

typedef struct
{
    fint_t  angle[2];
    fint_t  mouse[2];
    fint_t  msec;
    fint_t  timestamp;
    fint_t  frametime;
    byte    buttons[MAXACTIONS];
    byte    heldtime[MAXACTIONS];
} ticcmd_t;

//
// CLIENT PACKET TYPES
//
typedef enum
{
    cp_ping         = 1,
    cp_say,
    cp_cmd,
    cp_msgserver,
    cp_changeweapon,
    NUMCLIENTPACKETS
} cl_packets_t;

//
// SERVER PACKET TYPES
//
typedef enum
{
    sp_ping         = 1,
    sp_clientinfo,
    sp_msg,
    sp_pmove,
    sp_weaponinfo,
    sp_changemap,
    NUMSERVERPACKETS
} sv_packets_t;

//
// SERVER MESSAGES
//
typedef enum
{
    sm_full         = 1,
    NUMSERVERMESSAGES
} sv_messages_t;

#endif