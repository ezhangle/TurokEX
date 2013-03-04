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
// DESCRIPTION: Object Pooling System
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "js_objPool.h"
#include "common.h"

//
// JPool_Initialize
//

void JPool_Initialize(jsObjectPool_t *jsPool, jsint length, JSClass *class)
{
    int i;

    jsPool->pool = JS_NewArrayObject(js_context, length, NULL);
    jsPool->objects = JS_NewArrayObject(js_context, 0, NULL);

    JS_AddRoot(js_context, &jsPool->pool);
    JS_AddRoot(js_context, &jsPool->objects);

    for(i = 0; i < length; i++)
    {
        JSObject *obj = JS_NewObject(js_context, class, NULL, NULL);

        if(obj == NULL)
            Com_Error("JPool_Initialize: Unable to create object");

        JS_DefineElement(js_context, jsPool->pool, i,
            OBJECT_TO_JSVAL(obj), NULL, NULL, JSPROP_ENUMERATE);
    }
}

//
// JPool_GetFree
//

JSObject *JPool_GetFree(jsObjectPool_t *jsPool, JSClass *class)
{
    jsuint length;
    JSObject *object;
    jsval argv = JSVAL_VOID;
    jsval rval;

    if(JS_GetArrayLength(js_context, jsPool->pool, &length) && length > 0)
    {
        if(!JS_CallFunctionName(js_context, jsPool->pool, "pop", 0, &argv, &rval))
            return NULL;
        if(!JS_ValueToObject(js_context, rval, &object))
            return NULL;
        argv = OBJECT_TO_JSVAL(object);
        if(!JS_CallFunctionName(js_context, jsPool->objects, "push", 1, &argv, &rval))
            return NULL;

        return object;
    }

    if(!(object = JS_NewObject(js_context, class, NULL, NULL)))
        return NULL;

    return object;
}

//
// JPool_ReleaseObjects
//

void JPool_ReleaseObjects(jsObjectPool_t *jsPool)
{
    jsuint length;

    if(JS_GetArrayLength(js_context, jsPool->objects, &length) && length > 0)
    {
        jsuint i;

        for(i = 0; i < length; i++)
        {
            jsval argv;
            jsval rval;

            if(!JS_CallFunctionName(js_context, jsPool->objects,
                "pop", 0, &argv, &rval))
                continue;
            argv = rval;
            if(!JS_CallFunctionName(js_context, jsPool->pool,
                "push", 1, &argv, &rval))
                continue;
        }
    }
}