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
// DESCRIPTION: Javascript API
//
//-----------------------------------------------------------------------------

#include "js.h"
#include "js_shared.h"
#include "common.h"
#include "zone.h"

CVAR_EXTERNAL(kf_basepath);

#define JS_RUNTIME_HEAP_SIZE 64L * 1024L * 1024L
#define JS_STACK_CHUNK_SIZE  8192

static JSRuntime    *js_runtime;
static JSContext    *js_context;
static JSObject     *js_gobject;

//
// J_GlobalEnumerate
//
// Lazy enumeration for the ECMA standard classes.
// Doing this is said to lower memory usage.
//

static JSBool J_GlobalEnumerate(JSContext *cx, JSObject *obj)
{
    return JS_EnumerateStandardClasses(cx, obj);
}

//
// J_GlobalResolve
//
// Lazy resolution for the ECMA standard classes.
//

static JSBool J_GlobalResolve(JSContext *cx, JSObject *obj, jsval id,
                              uintN flags, JSObject **objp)
{
    if((flags & JSRESOLVE_ASSIGNING) == 0)
    {
        JSBool resolved;

        if(!JS_ResolveStandardClass(cx, obj, id, &resolved))
        {
            return JS_FALSE;
        }

        if(resolved)
        {
            *objp = obj;
            return JS_TRUE;
        }
    }

    return JS_TRUE;
}

//
// global_class
//
static JSClass global_class =
{
    "global",                                   // name
    JSCLASS_NEW_RESOLVE | JSCLASS_GLOBAL_FLAGS, // flags
    JS_PropertyStub,                            // addProperty
    JS_PropertyStub,                            // delProperty
    JS_PropertyStub,                            // getProperty
    JS_PropertyStub,                            // setProperty
    J_GlobalEnumerate,                          // enumerate
    (JSResolveOp)J_GlobalResolve,               // resolve
    JS_ConvertStub,                             // convert
    JS_FinalizeStub,                            // finalize
    JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
};

//
// J_Error
//

static void J_Error(JSContext *cx, const char *message, JSErrorReport *report)
{
    char *buf;
    char *f = NULL;
    char *l = NULL;
    char *m = NULL;
    int len = 0;

    if(!report)
    {
        Com_CPrintf(COLOR_RED, "%s\n", message);
        return;
    }

    if(report->filename)
    {
        f = Z_Strdupa(kva("%s: ", report->filename));
        len += strlen(f);
    }

    if(report->lineno)
    {
        l = Z_Strdupa(kva("%i ", report->lineno));
        len += strlen(l);
    }

    m = Z_Strdupa(message);
    len += strlen(m);

    buf = Z_Alloca(len+1);
    if(f) strcat(buf, f);
    if(l) strcat(buf, l);
    if(m) strcat(buf, m);

    if(JSREPORT_IS_WARNING(report->flags))
    {
        Com_Warning(buf);
        return;
    }
    else
    {
        Com_CPrintf(COLOR_RED, "%s\n", buf);
    }
}

//
// J_ContextCallback
//

static JSBool J_ContextCallback(JSContext *cx, uintN contextOp)
{
    if(contextOp == JSCONTEXT_NEW)
    {
        JS_SetErrorReporter(cx, J_Error);
        JS_SetVersion(cx, JSVERSION_LATEST);
    }
    
    return JS_TRUE;
}

//
// J_AddObject
//

JSObject *J_AddObject(JSClass *class, JSFunctionSpec *func, JSPropertySpec *prop,
                     const char *name, JSContext *cx, JSObject *obj)
{
    JSObject *newobj;

    if(!(newobj = JS_DefineObject(cx, obj, name, class, NULL, 0)))
        Com_Error("J_AddObject: Failed to create a new class for %s", name);

    if(prop)
    {
        if(!JS_DefineProperties(cx, newobj, prop))
             Com_Error("J_AddObject: Failed to define properties for class %s", name);
    }

    if(func)
    {
        if(!JS_DefineFunctions(cx, newobj, func))
            Com_Error("J_AddObject: Failed to define functions for class %s", name);
    }

    return newobj;
}

//
// J_Shutdown
//

void J_Shutdown(void)
{
    JS_DestroyContext(js_context);
    JS_DestroyRuntime(js_runtime);
    JS_ShutDown();
}

//
// FCmd_JS
//

static void FCmd_JS(void)
{
    JSContext *cx = js_context;
    JSObject *obj = js_gobject;
    JSBool ok;
    JSString *str;
    jsval result;
    char *buffer;

    if(Cmd_GetArgc() < 2)
    {
        Com_Printf("Usage: js <code>\n");
        return;
    }

    buffer = Cmd_GetArgv(1);

    if(JS_BufferIsCompilableUnit(cx, obj, buffer, strlen(buffer)))
    {
        JSScript *script;

        JS_ClearPendingException(cx);
        if(script = JS_CompileScript(cx, obj, buffer,
            strlen(buffer), "console", 1))
        {
            ok = JS_ExecuteScript(cx, obj, script, &result);

            if(ok && result != JSVAL_VOID)
            {
                if(str = JS_ValueToString(cx, result))
                {
                    Com_Printf("%s\n", JS_GetStringBytes(str));
                }
            }

            JS_DestroyScript(cx, script);
        }
    }
}

//
// FCmd_JSFile
//

static void FCmd_JSFile(void)
{
    JSContext *cx = js_context;
    JSObject *obj = js_gobject;
    JSScript *script;
    jsval result;
    uint32 oldopts;

    if(Cmd_GetArgc() < 2)
    {
        Com_Printf("Usage: jsfile <filename>\n");
        return;
    }

    oldopts = JS_GetOptions(cx);
    JS_SetOptions(cx, oldopts | JSOPTION_COMPILE_N_GO);
    if(script = JS_CompileFile(cx, obj, kva("%s\\%s",
        kf_basepath.string, Cmd_GetArgv(1))))
    {
        JS_ExecuteScript(cx, obj, script, &result);
        JS_DestroyScript(cx, script);
    }

    JS_SetOptions(cx, oldopts);
}

//
// J_Init
//

void J_Init(void)
{
    if(!(js_runtime = JS_NewRuntime(JS_RUNTIME_HEAP_SIZE)))
        Com_Error("JS_Init: Failed to initialize JSAPI runtime");

    JS_SetContextCallback(js_runtime, J_ContextCallback);

    if(!(js_context = JS_NewContext(js_runtime, JS_STACK_CHUNK_SIZE)))
        Com_Error("JS_Init: Failed to create a JSAPI context");

    if(!(js_gobject = JS_NewObject(js_context, &global_class, NULL, NULL)))
        Com_Error("JS_Init: Failed to create a global class object");

    JS_SetGlobalObject(js_context, js_gobject);

    JS_DEFINEOBJECT(sys);
    JS_INITCLASS(vector3, 3);

    Cmd_AddCommand("js", FCmd_JS);
    Cmd_AddCommand("jsfile", FCmd_JSFile);
}
