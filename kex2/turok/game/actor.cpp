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
// DESCRIPTION: GameActor system
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "actor.h"

DECLARE_ABSTRACT_CLASS(kexActor, kexObject)

//
// kexActor::kexActor
//

kexActor::kexActor(void) {
    this->mass          = 1200;
    this->friction      = 1.0f;
    this->airFriction   = 1.0f;
    this->refCount      = 0;
    this->bStatic       = true;
    this->bCollision    = false;
    this->bTouch        = false;
    this->bClientOnly   = false;
    this->bHidden       = false;
}

//
// kexActor::~kexActor
//

kexActor::~kexActor(void) {
}

//
// kexActor::LocalTick
//

void kexActor::LocalTick(void) {
}

//
// kexActor::Tick
//

void kexActor::Tick(void) {
}

//
// kexActor::Remove
//

void kexActor::Remove(void) {
}

//
// kexActor::AddRef
//

int kexActor::AddRef(void) {
    return ++refCount;
}

//
// kexActor::RemoveRef
//

int kexActor::RemoveRef(void) {
    return --refCount;
}

//
// kexActor::SetTarget
//

void kexActor::SetTarget(kexActor *targ) {
    // If there was a target already, decrease its refcount
    if(target)
        target->RemoveRef();

    // Set new target and if non-NULL, increase its counter
    if((target = targ))
        target->AddRef();
}

DECLARE_CLASS(kexWorldActor, kexActor)

unsigned int kexWorldActor::id = 0;

//
// kexWorldActor::kexWorldActor
//

kexWorldActor::kexWorldActor(void) {
    this->radius        = 30.72f;
    this->baseHeight    = 30.72f;
    this->viewHeight    = 16.384f;
    this->centerHeight  = 10.24f;
    this->bRotor        = false;
}

//
// kexWorldActor::~kexWorldActor
//

kexWorldActor::~kexWorldActor(void) {
}

//
// kexWorldActor::LocalTick
//

void kexWorldActor::LocalTick(void) {
}

//
// kexWorldActor::Tick
//

void kexWorldActor::Tick(void) {
}

//
// kexWorldActor::Remove
//

void kexWorldActor::Remove(void) {
}

//
// kexWorldActor::Parse
//

void kexWorldActor::Parse(kexLexer *lexer) {
}

//
// kexWorldActor::UpdateTransform
//

void kexWorldActor::UpdateTransform(void) {
}

//
// kexWorldActor::Event
//

bool kexWorldActor::Event(const char *function, long *args, unsigned int nargs) {
    return false;
}