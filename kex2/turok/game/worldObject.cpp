// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2013 Samuel Villarreal
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
// DESCRIPTION: World Object
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "worldObject.h"
#include "world.h"

DECLARE_ABSTRACT_CLASS(kexWorldObject, kexDisplayObject)

//
// kexWorldObject::kexWorldObject
//

kexWorldObject::kexWorldObject(void) {
    this->bStatic       = true;
    this->bCollision    = false;
    this->bTouch        = false;
    this->bCanPickup    = false;
    this->radius        = 10.24f;
    this->baseHeight    = 10.24f;
    this->viewHeight    = 8.192f;
    this->centerHeight  = 5.12f;
    this->areaNode      = NULL;
    
    this->physics.SetOwner(this);
    this->areaLink.SetData(this);
}

//
// kexWorldObject::~kexWorldObject
//

kexWorldObject::~kexWorldObject(void) {
}

//
// kexWorldObject::SetBoundingBox
//

void kexWorldObject::SetBoundingBox(const kexVec3 &min, const kexVec3 &max) {
    baseBBox.min = min;
    baseBBox.max = max;
}

//
// kexWorldObject::OnTouch
//

void kexWorldObject::OnTouch(kexWorldObject *instigator) {
}

//
// kexWorldObject::LinkArea
//

void kexWorldObject::LinkArea(void) {
    areaNode_t *node;
    kexBBox box;

    if(IsStale()) {
        return;
    }

    UnlinkArea();

    node = localWorld.areaNodes;
    box.min.Set(-radius, 0, -radius);
    box.max.Set(radius, 0, radius);
    box.min *= 0.5f;
    box.max *= 0.5f;
    box.min += origin;
    box.max += origin;
    box += 8.0f;

    while(1) {
        if(node->axis == -1) {
            break;
        }
        if(box.min[node->axis] > node->dist) {
            node = node->children[0];
        }
        else if(box.max[node->axis] < node->dist) {
            node = node->children[1];
        }
        else {
            break;
        }
    }

    areaLink.AddBefore(node->objects);
    areaNode = node;
}

//
// kexWorldObject::UnlinkArea
//

void kexWorldObject::UnlinkArea(void) {
    areaLink.Remove();
    areaNode = NULL;
}

//
// kexWorldObject::Trace
//

bool kexWorldObject::Trace(traceInfo_t *trace) {
    kexVec2 org;
    kexVec2 dir;
    kexVec2 nDir;
    kexVec2 cDist;
    float cp;
    float rd;
    float r;

    org = (origin - trace->start);
    dir = trace->dir;

    if(dir.Dot(org) <= 0) {
        return false;
    }

    float len = dir.Unit();

    if(len == 0) {
        return false;
    }

    nDir    = (dir * (1.0f / len));
    cp      = nDir.Dot(org);
    cDist   = (org - (nDir * cp));
    r       = radius + 8.192f;
    rd      = r * r - cDist.UnitSq();

    if(rd <= 0) {
        return false;
    }

    float frac = (cp - kexMath::Sqrt(rd)) * (1.0f / len);

    if(frac <= 1.0f && frac < trace->fraction) {
        kexVec3 hit;

        if(frac < 0) {
            frac = 0;
        }

        hit = (trace->start + ((trace->end - trace->start) * frac));
        if(hit[1] > origin[1] + baseHeight) {
            return false;
        }

        trace->hitActor = this;
        trace->fraction = frac;
        trace->hitVector = hit + (trace->dir * 0.09375f);
        trace->hitNormal = (trace->start - origin);
        trace->hitNormal[1] = 0;
        trace->hitVector[1] = trace->start[1];
        trace->hitNormal.Normalize();
        return true;
    }

    return false;
}

//
// kexWorldObject::AlignToSurface
//

bool kexWorldObject::AlignToSurface(void) {
    return false;
}
