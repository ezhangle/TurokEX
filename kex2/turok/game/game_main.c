// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2012 Samuel Villarreal
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
//
// DESCRIPTION: Main game code
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "client.h"
#include "server.h"
#include "kernel.h"
#include "mathlib.h"
#include "level.h"
#include "zone.h"
#include "game.h"

#define MOVE_VELOCITY   2.85f
#define SWIM_VELOCITY   0.05f
#define JUMP_VELOCITY   11.612f
#define NOCLIPMOVE      (MOVE_VELOCITY * 6)

//
// G_Shutdown
//

void G_Shutdown(void)
{
    Z_FreeTags(PU_LEVEL, PU_LEVEL);
    Z_FreeTags(PU_ACTOR, PU_ACTOR);
}

//
// G_Ticker
//

void G_Ticker(void)
{
    if(g_currentmap != NULL)
    {
        g_currentmap->tics++;
        g_currentmap->time = (float)g_currentmap->tics * 0.1f;
    }
}

//
// G_CheckJump
//

static kbool G_CheckJump(actor_t *actor)
{
    if(actor->plane == NULL)
    {
        return false;
    }

    if(actor->terriantype == TT_WATER_SURFACE)
    {
        if(actor->origin[1] >
            (Map_GetArea(actor->plane)->waterplane -
            actor->object.centerheight))
        {
            return true;
        }
    }
    else if(Plane_IsAWall(actor->plane) &&
        actor->origin[1] - Plane_GetDistance(actor->plane, actor->origin) <= 8)
    {
        return false;
    }
    else if(!(actor->flags & AF_CLIENTJUMP))
    {
        if(actor->velocity[1] < 0 && actor->velocity[1] > -16)
        {
            return true;
        }
    }

    if((actor->origin[1] + actor->velocity[1]) -
        Plane_GetDistance(actor->plane, actor->origin) < ONPLANE_EPSILON)
    {
        return true;
    }

    return false;
}

//
// G_ClientWalk
//

static void G_ClientWalk(actor_t *client, ticcmd_t *cmd)
{
    float sy;
    float cy;

    sy = (float)sin(client->yaw);
    cy = (float)cos(client->yaw);

    if(cmd->buttons & BT_FORWARD)
    {
        client->velocity[0] += MOVE_VELOCITY * sy;
        client->velocity[2] += MOVE_VELOCITY * cy;
    }

    if(cmd->buttons & BT_BACKWARD)
    {
        client->velocity[0] -= MOVE_VELOCITY * sy;
        client->velocity[2] -= MOVE_VELOCITY * cy;
    }

    sy = (float)sin(client->yaw + DEG2RAD(90));
    cy = (float)cos(client->yaw + DEG2RAD(90));

    if(cmd->buttons & BT_STRAFELEFT)
    {
        client->velocity[0] += MOVE_VELOCITY * sy;
        client->velocity[2] += MOVE_VELOCITY * cy;
    }

    if(cmd->buttons & BT_STRAFERIGHT)
    {
        client->velocity[0] -= MOVE_VELOCITY * sy;
        client->velocity[2] -= MOVE_VELOCITY * cy;
    }

    if(cmd->buttons & BT_JUMP)
    {
        if(G_CheckJump(client) && !cmd->heldtime[1])
        {
            client->flags |= AF_CLIENTJUMP;
            client->velocity[1] = JUMP_VELOCITY;
        }
    }
}

//
// G_ClientSwim
//

static void G_ClientSwim(actor_t *client, ticcmd_t *cmd)
{
    float sy;
    float cy;
    float vsy;
    float vcy;
    float vel;

    if(cmd->heldtime[0] == 0 &&
        Vec_Unit3(client->velocity) < 3)
    {
        vel = SWIM_VELOCITY * 60;
    }
    else
    {
        vel = SWIM_VELOCITY;
    }

    sy = (float)sin(client->yaw);
    cy = (float)cos(client->yaw);
    vsy = (float)sin(client->pitch);
    vcy = (float)cos(client->pitch);

    if(cmd->buttons & BT_FORWARD)
    {
        client->velocity[0] += (vel * sy) * vcy;
        client->velocity[1] -= (vel * vsy);
        client->velocity[2] += (vel * cy) * vcy;
    }

    if(cmd->buttons & BT_BACKWARD)
    {
        client->velocity[0] -= (SWIM_VELOCITY * sy) * vcy;
        client->velocity[1] += (SWIM_VELOCITY * vsy);
        client->velocity[2] -= (SWIM_VELOCITY * cy) * vcy;
    }

    sy = (float)sin(client->yaw + DEG2RAD(90));
    cy = (float)cos(client->yaw + DEG2RAD(90));

    if(cmd->buttons & BT_STRAFELEFT)
    {
        client->velocity[0] += SWIM_VELOCITY * sy;
        client->velocity[2] += SWIM_VELOCITY * cy;
    }

    if(cmd->buttons & BT_STRAFERIGHT)
    {
        client->velocity[0] -= SWIM_VELOCITY * sy;
        client->velocity[2] -= SWIM_VELOCITY * cy;
    }

    if(cmd->buttons & BT_JUMP)
    {
        client->velocity[1] += SWIM_VELOCITY;
    }
}

//
// G_ClientPaddle
//

static void G_ClientPaddle(actor_t *client, ticcmd_t *cmd)
{
    float sy;
    float cy;
    float vsy;
    float vcy;
    float ang;
    float vel;

    sy = (float)sin(client->yaw);
    cy = (float)cos(client->yaw);
    vsy = (float)sin(client->pitch);
    vcy = (float)cos(client->pitch);

    ang = client->pitch;

    Ang_Clamp(&ang);
            
    if(ang < DEG2RAD(45))
    {
        vsy = 0;
    }
    else
    {
        vsy *= MOVE_VELOCITY;
    }

    if(cmd->heldtime[0] == 0 &&
        Vec_Unit2(client->velocity) < 2)
    {
        vel = SWIM_VELOCITY * 80;
    }
    else
    {
        vel = SWIM_VELOCITY;
    }

    if(cmd->buttons & BT_FORWARD)
    {
        client->velocity[0] += (vel * sy) * vcy;
        client->velocity[1] -= vsy;
        client->velocity[2] += (vel * cy) * vcy;
    }

    if(cmd->buttons & BT_BACKWARD)
    {
        client->velocity[0] -= (SWIM_VELOCITY * sy) * vcy;
        client->velocity[1] += vsy;
        client->velocity[2] -= (SWIM_VELOCITY * cy) * vcy;
    }

    sy = (float)sin(client->yaw + DEG2RAD(90));
    cy = (float)cos(client->yaw + DEG2RAD(90));

    if(cmd->buttons & BT_STRAFELEFT)
    {
        client->velocity[0] += SWIM_VELOCITY * sy;
        client->velocity[2] += SWIM_VELOCITY * cy;
    }

    if(cmd->buttons & BT_STRAFERIGHT)
    {
        client->velocity[0] -= SWIM_VELOCITY * sy;
        client->velocity[2] -= SWIM_VELOCITY * cy;
    }

    if(cmd->buttons & BT_JUMP)
    {
        if(G_CheckJump(client) && !cmd->heldtime[1])
        {
            client->flags |= AF_CLIENTJUMP;
            client->velocity[1] = JUMP_VELOCITY;
        }
    }
}

//
// G_ClientNoClipMove
//

static void G_ClientNoClipMove(actor_t *client, ticcmd_t *cmd)
{
    float sy;
    float cy;
    float vsy;
    float vcy;
    float x1;
    float y1;
    float z1;
    float x2;
    float y2;
    float z2;

    sy = (float)sin(client->yaw);
    cy = (float)cos(client->yaw);
    vsy = (float)sin(client->pitch);
    vcy = (float)cos(client->pitch);

    x1 = y1 = z1 = x2 = y2 = z2 = 0;

    if(cmd->buttons & BT_FORWARD)
    {
        x1 = (NOCLIPMOVE * sy) * vcy;
        y1 = NOCLIPMOVE * -vsy;
        z1 = (NOCLIPMOVE * cy) * vcy;
    }

    if(cmd->buttons & BT_BACKWARD)
    {
        x1 = -(NOCLIPMOVE * sy) * vcy;
        y1 = NOCLIPMOVE * vsy;
        z1 = -(NOCLIPMOVE * cy) * vcy;
    }

    sy = (float)sin(client->yaw + DEG2RAD(90));
    cy = (float)cos(client->yaw + DEG2RAD(90));

    if(cmd->buttons & BT_STRAFELEFT)
    {
        x2 = NOCLIPMOVE * sy;
        z2 = NOCLIPMOVE * cy;
    }

    if(cmd->buttons & BT_STRAFERIGHT)
    {
        x2 = -NOCLIPMOVE * sy;
        z2 = -NOCLIPMOVE * cy;
    }

    if(cmd->buttons & BT_JUMP)
    {
        y2 = NOCLIPMOVE;
    }

    client->velocity[0] = x1 + x2;
    client->velocity[1] = y1 + y2;
    client->velocity[2] = z1 + z2;
}

//
// G_ClientReborn
//

void G_ClientReborn(gclient_t *client)
{
    memset(client, 0, sizeof(gclient_t));

    client->hasbackpack                 = false;
    client->maxammo[am_clip]            = 100;
    client->maxammo[am_shells]          = 20;
    client->maxammo[am_expshells]       = 10;
    client->maxammo[am_cell]            = 100;
    client->maxammo[am_tekarrows]       = 15;
    client->maxammo[am_arrows]          = 30;
    client->maxammo[am_mini]            = 125;
    client->maxammo[am_grenade]         = 20;
    client->maxammo[am_rocket]          = 6;
    client->maxammo[am_fusion]          = 2;
    client->maxammo[am_chrono]          = 3;
    client->weaponowned[wp_knife]       = true;
    client->weaponowned[wp_crossbow]    = true;
    client->activeweapon                = wp_knife;
    client->actor->health               = 100;
}

//
// G_ClientThink
//

void G_ClientThink(actor_t *client, ticcmd_t *cmd)
{
    switch(client->terriantype)
    {
    case TT_WATER_SHALLOW:
        G_ClientWalk(client, cmd);
        break;

    case TT_WATER_SURFACE:
        G_ClientPaddle(client, cmd);
        break;

    case TT_WATER_UNDER:
        G_ClientSwim(client, cmd);
        break;

    case TT_LAVA:
        G_ClientWalk(client, cmd);
        break;

    case TT_NOCLIP:
        G_ClientNoClipMove(client, cmd);
        break;

    default:
        G_ClientWalk(client, cmd);
        break;
    }

    // TEMP
    G_ActorMovement(client);
}

//
// FCmd_NoClip
//

static void FCmd_NoClip(void)
{
    actor_t *actor;

    // TODO: TEMP
    actor = &client.localactor;
    if(actor->terriantype == TT_NOCLIP)
    {
        actor->terriantype = TT_NORMAL;
        actor->plane = G_FindClosestPlane(actor->origin);
    }
    else
    {
        actor->terriantype = TT_NOCLIP;
    }
}

//
// G_Init
//

void G_Init(void)
{
    Map_Init();

    // TODO: TEMP
    Cmd_AddCommand("noclip", FCmd_NoClip);
}

