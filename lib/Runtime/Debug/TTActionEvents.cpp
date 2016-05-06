//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#include "RuntimeDebugPch.h"

#if ENABLE_TTD

namespace TTD
{
    namespace NSLogEvents
    {
        bool IsJsRTActionExecutedInScriptWrapper(EventKind tag)
        {
            switch(tag)
            {
            case EventKind::GetAndClearExceptionActionTag:
                return false;
            default:
                return true;
            }
        }

        void JsRTActionPassVarToHostInReplay(Js::ScriptContext* ctx, TTDVar origVar, Js::Var replayVar)
        {
#if ENABLE_TTD_INTERNAL_DIAGNOSTICS
            if(replayVar == nullptr || TTD::JsSupport::IsVarTaggedInline(replayVar))
            {
                AssertMsg(origVar == replayVar, "Should be same bit pattern.");
            }
#endif

            if(ctx->ShouldTagForJsRT())
            {
                if(replayVar != nullptr && TTD::JsSupport::IsVarPtrValued(replayVar))
                {
                    ctx->GetThreadContext()->TTDInfo->TrackTagObject(origVar, Js::RecyclableObject::FromVar(replayVar));
                }
            }
        }

        Js::Var JsRTActionInflateVarInReplay(Js::ScriptContext* ctx, TTDVar origVar)
        {
            static_assert(sizeof(TTDVar) == sizeof(Js::Var), "We assume the bit patterns on these types are the same!!!");

            if(origVar == nullptr || TTD::JsSupport::IsVarTaggedInline(origVar))
            {
                return (Js::Var)origVar;
            }
            else
            {
                return ctx->GetThreadContext()->TTDInfo->LookupObjectForTag(origVar);
            }
        }

#if !INT32VAR
        void CreateInt_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTVarsWithIntegralUnionArgumentAction* action = GetInlineEventDataAs<JsRTVarsWithIntegralUnionArgumentAction, EventKind::CreateIntegerActionTag>(evt);

            Js::Var res = nullptr;
            if(!Js::JavascriptNumber::TryToVarFast((int32)action->u_iVal, &res))
            {
                res = Js::JavascriptNumber::ToVar((int32)action->u_iVal, ctx);
            }

            JsRTActionHandleResultForReplay<JsRTVarsWithIntegralUnionArgumentAction, EventKind::CreateIntegerActionTag>(ctx, evt, res);
        }
#endif

        void CreateNumber_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTDoubleArgumentAction* action = GetInlineEventDataAs<JsRTDoubleArgumentAction, EventKind::CreateNumberActionTag>(evt);

            Js::Var res = Js::JavascriptNumber::ToVarNoCheck(action->DoubleValue, ctx);

            JsRTActionHandleResultForReplay<JsRTDoubleArgumentAction, EventKind::CreateNumberActionTag>(ctx, evt, res);
        }

        void CreateBoolean_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTVarsWithIntegralUnionArgumentAction* action = GetInlineEventDataAs<JsRTVarsWithIntegralUnionArgumentAction, EventKind::CreateBooleanActionTag>(evt);

            Js::Var res = action->u_bVal ? ctx->GetLibrary()->GetTrue() : ctx->GetLibrary()->GetFalse();

            JsRTActionHandleResultForReplay<JsRTVarsWithIntegralUnionArgumentAction, EventKind::CreateBooleanActionTag>(ctx, evt, res);
        }

        void CreateString_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTStringArgumentAction* action = GetInlineEventDataAs<JsRTStringArgumentAction, EventKind::CreateStringActionTag>(evt);

            Js::Var res = Js::JavascriptString::NewCopyBuffer(action->StringValue.Contents, action->StringValue.Length, ctx);

            JsRTActionHandleResultForReplay<JsRTStringArgumentAction, EventKind::CreateStringActionTag>(ctx, evt, res);
        }

        void CreateSymbol_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTVarsArgumentAction* action = GetInlineEventDataAs<JsRTVarsArgumentAction, EventKind::CreateSymbolActionTag>(evt);
            Js::Var description = JsRTActionInflateVarInReplay(ctx, action->Var1);

            Js::JavascriptString* descriptionString;
            if(description != nullptr)
            {
                descriptionString = Js::JavascriptConversion::ToString(description, ctx);
            }
            else
            {
                descriptionString = ctx->GetLibrary()->GetEmptyString();
            }
            Js::Var res = ctx->GetLibrary()->CreateSymbol(descriptionString);

            JsRTActionHandleResultForReplay<JsRTVarsArgumentAction, EventKind::CreateSymbolActionTag>(ctx, evt, res);
        }

        void VarConvertToNumber_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTVarsArgumentAction* action = GetInlineEventDataAs<JsRTVarsArgumentAction, EventKind::VarConvertToNumberActionTag>(evt);
            Js::Var var = JsRTActionInflateVarInReplay(ctx, action->Var1);

            Js::Var res = Js::JavascriptOperators::ToNumber(var, ctx);

            JsRTActionHandleResultForReplay<JsRTVarsArgumentAction, EventKind::VarConvertToNumberActionTag>(ctx, evt, res);
        }

        void VarConvertToBoolean_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTVarsArgumentAction* action = GetInlineEventDataAs<JsRTVarsArgumentAction, EventKind::VarConvertToBooleanActionTag>(evt);
            Js::Var var = JsRTActionInflateVarInReplay(ctx, action->Var1);

            Js::Var res = Js::JavascriptConversion::ToBool(var, ctx) ? ctx->GetLibrary()->GetTrue() : ctx->GetLibrary()->GetFalse();

            JsRTActionHandleResultForReplay<JsRTVarsArgumentAction, EventKind::VarConvertToBooleanActionTag>(ctx, evt, res);
        }

        void VarConvertToString_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTVarsArgumentAction* action = GetInlineEventDataAs<JsRTVarsArgumentAction, EventKind::VarConvertToStringActionTag>(evt);
            Js::Var var = JsRTActionInflateVarInReplay(ctx, action->Var1);

            Js::Var res = Js::JavascriptConversion::ToString(var, ctx);

            JsRTActionHandleResultForReplay<JsRTVarsArgumentAction, EventKind::VarConvertToStringActionTag>(ctx, evt, res);
        }

        void VarConvertToObject_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTVarsArgumentAction* action = GetInlineEventDataAs<JsRTVarsArgumentAction, EventKind::VarConvertToObjectActionTag>(evt);
            Js::Var var = JsRTActionInflateVarInReplay(ctx, action->Var1);

            Js::Var res = Js::JavascriptOperators::ToObject(var, ctx);

            JsRTActionHandleResultForReplay<JsRTVarsArgumentAction, EventKind::VarConvertToObjectActionTag>(ctx, evt, res);
        }

        void AddRootRef_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            asdf;
        }

        void RemoveRootRef_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            asdf;
        }

        void LocalRootClear_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            asdf;
        }

        void AllocateObject_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            Js::RecyclableObject* res = ctx->GetLibrary()->CreateObject();

            JsRTActionHandleResultForReplay<JsRTVarsArgumentAction, EventKind::AllocateObjectActionTag>(ctx, evt, res);
        }

        void AllocateExternalObject_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            Js::RecyclableObject* res = ctx->GetLibrary()->CreateObject();

            JsRTActionHandleResultForReplay<JsRTVarsArgumentAction, EventKind::AllocateExternalObjectActionTag>(ctx, evt, res);
        }

        void AllocateArrayAction_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTVarsWithIntegralUnionArgumentAction* action = GetInlineEventDataAs<JsRTVarsWithIntegralUnionArgumentAction, EventKind::AllocateArrayActionTag>(evt);

            Js::Var res = ctx->GetLibrary()->CreateArray((uint32)action->u_iVal);

            JsRTActionHandleResultForReplay<JsRTVarsWithIntegralUnionArgumentAction, EventKind::AllocateArrayActionTag>(ctx, evt, res);
        }

        void AllocateArrayBufferAction_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTByteBufferAction* action = GetInlineEventDataAs<JsRTByteBufferAction, EventKind::AllocateArrayBufferActionTag>(evt);

            Js::ArrayBuffer* abuff = ctx->GetLibrary()->CreateArrayBuffer(action->Length);
            AssertMsg(abuff->GetByteLength() == action->Length, "Something is wrong with our sizes.");

            js_memcpy_s(abuff->GetBuffer(), abuff->GetByteLength(), action->Buffer, action->Length);

            JsRTActionHandleResultForReplay<JsRTByteBufferAction, EventKind::AllocateArrayBufferActionTag>(ctx, evt, (Js::Var)abuff);
        }

        void AllocateFunctionAction_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTVarsWithIntegralUnionArgumentAction* action = GetInlineEventDataAs<JsRTVarsWithIntegralUnionArgumentAction, EventKind::AllocateFunctionActionTag>(evt);

            Js::Var res = nullptr;
            if(action->u_bVal)
            {
                res = ctx->GetLibrary()->CreateStdCallExternalFunction(nullptr, 0, nullptr);
            }
            else
            {
                Js::Var nameVar = JsRTActionInflateVarInReplay(ctx, action->Var1);

                Js::JavascriptString* name = nullptr;
                if(nameVar != nullptr)
                {
                    name = Js::JavascriptConversion::ToString(nameVar, ctx);
                }
                else
                {
                    name = ctx->GetLibrary()->GetEmptyString();
                }

                res = ctx->GetLibrary()->CreateStdCallExternalFunction(nullptr, name, nullptr);
            }

            JsRTActionHandleResultForReplay<JsRTVarsWithIntegralUnionArgumentAction, EventKind::AllocateFunctionActionTag>(ctx, evt, res);
        }

        void GetAndClearExceptionAction_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            HRESULT hr = S_OK;
            Js::JavascriptExceptionObject *recordedException = nullptr;

            BEGIN_TRANSLATE_OOM_TO_HRESULT
            recordedException = ctx->GetAndClearRecordedException();
            END_TRANSLATE_OOM_TO_HRESULT(hr)

            if(hr == E_OUTOFMEMORY)
            {
                //
                //TODO: we don't have support for OOM yet (and adding support will require a non-trivial amount of work)
                //
                AssertMsg(false, "OOM is not supported");
            }

            Js::Var exception = nullptr;
            if(recordedException != nullptr)
            {
                exception = recordedException->GetThrownObject(nullptr);
            }

            JsRTActionHandleResultForReplay<JsRTVarsArgumentAction, EventKind::GetAndClearExceptionActionTag>(ctx, evt, exception);
        }

        void GetPropertyAction_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTVarsWithIntegralUnionArgumentAction* action = GetInlineEventDataAs<JsRTVarsWithIntegralUnionArgumentAction, EventKind::GetPropertyActionTag>(evt);
            Js::Var var = JsRTActionInflateVarInReplay(ctx, action->Var1);

            Js::Var res = Js::JavascriptOperators::OP_GetProperty(var, action->u_pid, ctx);

            JsRTActionHandleResultForReplay<JsRTVarsWithIntegralUnionArgumentAction, EventKind::GetPropertyActionTag>(ctx, evt, res);
        }

        void GetIndexAction_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTVarsArgumentAction* action = GetInlineEventDataAs<JsRTVarsArgumentAction, EventKind::GetIndexActionTag>(evt);
            Js::Var var = JsRTActionInflateVarInReplay(ctx, action->Var1);
            Js::Var index = JsRTActionInflateVarInReplay(ctx, action->Var2);

            Js::Var res = Js::JavascriptOperators::OP_GetElementI(var, index, ctx);

            JsRTActionHandleResultForReplay<JsRTVarsArgumentAction, EventKind::GetIndexActionTag>(ctx, evt, res);
        }

        void GetOwnPropertyInfoAction_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTVarsWithIntegralUnionArgumentAction* action = GetInlineEventDataAs<JsRTVarsWithIntegralUnionArgumentAction, EventKind::GetOwnPropertyInfoActionTag>(evt);
            Js::Var var = JsRTActionInflateVarInReplay(ctx, action->Var1);

            Js::Var res = nullptr;
            Js::PropertyDescriptor propertyDescriptorValue;
            if(Js::JavascriptOperators::GetOwnPropertyDescriptor(Js::RecyclableObject::FromVar(var), action->u_pid, ctx, &propertyDescriptorValue))
            {
                res = Js::JavascriptOperators::FromPropertyDescriptor(propertyDescriptorValue, ctx);
            }

            JsRTActionHandleResultForReplay<JsRTVarsWithIntegralUnionArgumentAction, EventKind::GetOwnPropertyInfoActionTag>(ctx, evt, res);
        }

        void GetOwnPropertiesInfoAction_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTVarsWithIntegralUnionArgumentAction* action = GetInlineEventDataAs<JsRTVarsWithIntegralUnionArgumentAction, EventKind::GetOwnPropertiesInfoActionTag>(evt);
            Js::Var var = JsRTActionInflateVarInReplay(ctx, action->Var1);

            Js::Var res = nullptr;
            if(action->u_bVal)
            {
                res = Js::JavascriptOperators::GetOwnPropertyNames(var, ctx);
            }
            else
            {
                res = Js::JavascriptOperators::GetOwnPropertySymbols(var, ctx);
            }

            JsRTActionHandleResultForReplay<JsRTVarsWithIntegralUnionArgumentAction, EventKind::GetOwnPropertiesInfoActionTag>(ctx, evt, res);
        }

        void DefinePropertyAction_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTVarsWithIntegralUnionArgumentAction* action = GetInlineEventDataAs<JsRTVarsWithIntegralUnionArgumentAction, EventKind::DefinePropertyActionTag>(evt);
            Js::Var object = JsRTActionInflateVarInReplay(ctx, action->Var1);
            Js::Var propertyDescriptor = JsRTActionInflateVarInReplay(ctx, action->Var2);

            Js::PropertyDescriptor propertyDescriptorValue;
            Js::JavascriptOperators::ToPropertyDescriptor(propertyDescriptor, &propertyDescriptorValue, ctx);

            Js::JavascriptOperators::DefineOwnPropertyDescriptor(Js::RecyclableObject::FromVar(object), action->u_pid, propertyDescriptorValue, true, ctx);
        }

        void DeletePropertyAction_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTVarsWithBoolAndPIDArgumentAction* action = GetInlineEventDataAs<JsRTVarsWithBoolAndPIDArgumentAction, EventKind::DeletePropertyActionTag>(evt);
            Js::Var var = JsRTActionInflateVarInReplay(ctx, action->Var1);

            Js::Var res = Js::JavascriptOperators::OP_DeleteProperty(var, action->Pid, ctx, action->BoolVal ? Js::PropertyOperation_StrictMode : Js::PropertyOperation_None);

            JsRTActionHandleResultForReplay<JsRTVarsWithBoolAndPIDArgumentAction, EventKind::DeletePropertyActionTag>(ctx, evt, res);
        }

        void SetPrototypeAction_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTVarsArgumentAction* action = GetInlineEventDataAs<JsRTVarsArgumentAction, EventKind::SetPrototypeActionTag>(evt);
            Js::Var var = JsRTActionInflateVarInReplay(ctx, action->Var1);
            Js::Var proto = JsRTActionInflateVarInReplay(ctx, action->Var2);

            Js::JavascriptObject::ChangePrototype(Js::RecyclableObject::FromVar(var), Js::RecyclableObject::FromVar(proto), true, ctx);
        }

        void SetPropertyAction_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTVarsWithBoolAndPIDArgumentAction* action = GetInlineEventDataAs<JsRTVarsWithBoolAndPIDArgumentAction, EventKind::SetPropertyActionTag>(evt);
            Js::Var var = JsRTActionInflateVarInReplay(ctx, action->Var1);
            Js::Var value = JsRTActionInflateVarInReplay(ctx, action->Var2);

            Js::JavascriptOperators::OP_SetProperty(var, action->Pid, value, ctx, nullptr, action->BoolVal ? Js::PropertyOperation_StrictMode : Js::PropertyOperation_None);
        }

        void SetIndexAction_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTVarsArgumentAction* action = GetInlineEventDataAs<JsRTVarsArgumentAction, EventKind::SetIndexActionTag>(evt);
            Js::Var var = JsRTActionInflateVarInReplay(ctx, action->Var1);
            Js::Var index = JsRTActionInflateVarInReplay(ctx, action->Var2);
            Js::Var value = JsRTActionInflateVarInReplay(ctx, action->Var3);

            Js::JavascriptOperators::OP_SetElementI(var, index, value, ctx);
        }

        void GetTypedArrayInfoAction_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTVarsWithIntegralUnionArgumentAction* action = GetInlineEventDataAs<JsRTVarsWithIntegralUnionArgumentAction, EventKind::GetTypedArrayInfoActionTag>(evt);
            Js::Var var = JsRTActionInflateVarInReplay(ctx, action->Var1);

            Js::Var res = nullptr;
            if(action->u_bVal)
            {
                Js::TypedArrayBase* typedArrayBase = Js::TypedArrayBase::FromVar(var);
                res = typedArrayBase->GetArrayBuffer();
            }

            JsRTActionHandleResultForReplay<JsRTVarsWithIntegralUnionArgumentAction, EventKind::GetTypedArrayInfoActionTag>(ctx, evt, res);
        }
        
        //////////////////

        void JsRTConstructCallAction_ProcessArgs(EventLogEntry* evt, int32 rootDepth, Js::JavascriptFunction* function, uint32 argc, Js::Var* argv, UnlinkableSlabAllocator& alloc)
        {
            JsRTConstructCallAction* ccAction = GetInlineEventDataAs<JsRTConstructCallAction, EventKind::ConstructCallActionTag>(evt);

            ccAction->ArgCount = argc + 1;

            static_assert(sizeof(TTDVar) == sizeof(Js::Var), "These need to be the same size (and have same bit layout) for this to work!");

            ccAction->ArgArray = alloc.SlabAllocateArray<TTDVar>(ccAction->ArgCount);
            ccAction->ArgArray[0] = static_cast<TTDVar>(function);
            js_memcpy_s(ccAction->ArgArray + 1, (ccAction->ArgCount - 1) * sizeof(TTDVar), argv, argc * sizeof(Js::Var));
        }

        void JsRTConstructCallAction_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTConstructCallAction* ccAction = GetInlineEventDataAs<JsRTConstructCallAction, EventKind::ConstructCallActionTag>(evt);

            Js::Var jsFunctionVar = JsRTActionInflateVarInReplay(ctx, ccAction->ArgArray[0]);
            Js::JavascriptFunction* jsFunction = Js::JavascriptFunction::FromVar(jsFunctionVar);

            //remove implicit constructor function as first arg in callInfo and argument loop below
            Js::CallInfo callInfo(Js::CallFlags::CallFlags_New, (ushort)(ccAction->ArgCount - 1)); 
            for(uint32 i = 1; i < ccAction->ArgCount; ++i)
            {
                ccAction->ExecArgs[i] = JsRTActionInflateVarInReplay(ctx, ccAction->ArgArray[i]);
            }
            Js::Arguments jsArgs(callInfo, ccAction->ExecArgs);

            //
            //TODO: we will want to look at this at some point -- either treat as "top-level" call or maybe constructors are fast so we can just jump back to previous "real" code
            //AssertMsg(!Js::ScriptFunction::Is(jsFunction) || execContext->TTDRootNestingCount != 0, "This will cause user code to execute and we need to add support for that as a top-level call source!!!!");
            //

            Js::Var res = Js::JavascriptFunction::CallAsConstructor(jsFunction, /* overridingNewTarget = */nullptr, jsArgs, ctx);

            JsRTActionHandleResultForReplay<JsRTConstructCallAction, EventKind::ConstructCallActionTag>(ctx, evt, res);
        }

        void JsRTConstructCallAction_UnloadEventMemory(EventLogEntry* evt, UnlinkableSlabAllocator& alloc)
        {
            JsRTConstructCallAction* ccAction = GetInlineEventDataAs<JsRTConstructCallAction, EventKind::ConstructCallActionTag>(evt);

            if(ccAction->ArgArray != nullptr)
            {
                alloc.UnlinkAllocation(ccAction->ArgArray);
            }

#if ENABLE_TTD_DEBUGGING
            if(ccAction->ExecArgs != nullptr)
            {
                alloc.UnlinkAllocation(ccAction->ExecArgs);
            }
#endif
        }

        void JsRTConstructCallAction_Emit(const EventLogEntry* evt, LPCWSTR uri, FileWriter* writer, ThreadContext* threadContext)
        {
            const JsRTConstructCallAction* ccAction = GetInlineEventDataAs<JsRTConstructCallAction, EventKind::ConstructCallActionTag>(evt);

            writer->WriteKey(NSTokens::Key::argRetVal, NSTokens::Separator::CommaSeparator);
            NSSnapValues::EmitTTDVar(ccAction->Result, writer, NSTokens::Separator::NoSeparator);

            writer->WriteLengthValue(ccAction->ArgCount, NSTokens::Separator::CommaSeparator);
            writer->WriteSequenceStart_DefaultKey(NSTokens::Separator::CommaSeparator);
            for(uint32 i = 0; i < ccAction->ArgCount; ++i)
            {
                NSTokens::Separator sep = (i != 0) ? NSTokens::Separator::CommaSeparator : NSTokens::Separator::NoSeparator;
                NSSnapValues::EmitTTDVar(ccAction->ArgArray[i], writer, sep);
            }
            writer->WriteSequenceEnd();
        }

        void JsRTConstructCallAction_Parse(EventLogEntry* evt, ThreadContext* threadContext, FileReader* reader, UnlinkableSlabAllocator& alloc)
        {
            JsRTConstructCallAction* ccAction = GetInlineEventDataAs<JsRTConstructCallAction, EventKind::ConstructCallActionTag>(evt);

            reader->ReadKey(NSTokens::Key::argRetVal, true);
            ccAction->Result = NSSnapValues::ParseTTDVar(false, reader);

            ccAction->ArgCount = reader->ReadLengthValue(true);
            ccAction->ArgArray = alloc.SlabAllocateArray<TTDVar>(ccAction->ArgCount);

            reader->ReadSequenceStart_WDefaultKey(true);
            for(uint32 i = 0; i < ccAction->ArgCount; ++i)
            {
                ccAction->ArgArray[i] = NSSnapValues::ParseTTDVar(i != 0, reader);
            }
            reader->ReadSequenceEnd();

#if ENABLE_TTD_DEBUGGING
            Js::Var* execArgs = alloc.SlabAllocateArray<Js::Var>(ccAction->ArgCount - 1); //we use an extra space for the function
#endif
        }

        void JsRTCallbackAction_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTCallbackAction* cbAction = GetInlineEventDataAs<JsRTCallbackAction, EventKind::CallbackOpActionTag>(evt);

#if !ENABLE_TTD_DEBUGGING
            ; //we don't need to do anything
#else
            if(cbAction->RegisterLocation == nullptr)
            {
                const_cast<JsRTCallbackAction*>(cbAction)->RegisterLocation = HeapNew(TTDebuggerSourceLocation);
            }

            if(!cbAction->RegisterLocation->HasValue())
            {
                ctx->GetThreadContext()->TTDLog->GetTimeAndPositionForDebugger(*(cbAction->RegisterLocation));
            }
#endif
        }

        void JsRTCallbackAction_UnloadEventMemory(EventLogEntry* evt, UnlinkableSlabAllocator& alloc)
        {
            JsRTCallbackAction* cbAction = GetInlineEventDataAs<JsRTCallbackAction, EventKind::CallbackOpActionTag>(evt);

            if(cbAction->RegisterLocation != nullptr)
            {
                cbAction->RegisterLocation->Clear();
                HeapDelete(cbAction->RegisterLocation);
            }
        }

        void JsRTCallbackAction_Emit(const EventLogEntry* evt, LPCWSTR uri, FileWriter* writer, ThreadContext* threadContext)
        {
            const JsRTCallbackAction* cbAction = GetInlineEventDataAs<JsRTCallbackAction, EventKind::CallbackOpActionTag>(evt);

            writer->WriteBool(NSTokens::Key::boolVal, cbAction->IsCreate ? true : false, NSTokens::Separator::CommaSeparator);
            writer->WriteBool(NSTokens::Key::boolVal, cbAction->IsCancel ? true : false, NSTokens::Separator::CommaSeparator);
            writer->WriteBool(NSTokens::Key::boolVal, cbAction->IsRepeating ? true : false, NSTokens::Separator::CommaSeparator);

            writer->WriteInt64(NSTokens::Key::hostCallbackId, cbAction->CurrentCallbackId, NSTokens::Separator::CommaSeparator);
            writer->WriteInt64(NSTokens::Key::newCallbackId, cbAction->NewCallbackId, NSTokens::Separator::CommaSeparator);
        }

        void JsRTCallbackAction_Parse(EventLogEntry* evt, ThreadContext* threadContext, FileReader* reader, UnlinkableSlabAllocator& alloc)
        {
            JsRTCallbackAction* cbAction = GetInlineEventDataAs<JsRTCallbackAction, EventKind::CallbackOpActionTag>(evt);

            cbAction->IsCreate = reader->ReadBool(NSTokens::Key::boolVal, true) ? TRUE : FALSE;
            cbAction->IsCancel = reader->ReadBool(NSTokens::Key::boolVal, true) ? TRUE : FALSE;
            cbAction->IsRepeating = reader->ReadBool(NSTokens::Key::boolVal, true) ? TRUE : FALSE;

            cbAction->CurrentCallbackId = reader->ReadInt64(NSTokens::Key::hostCallbackId, true);
            cbAction->NewCallbackId = reader->ReadInt64(NSTokens::Key::newCallbackId, true);
        }

        bool JsRTCallbackAction_GetActionTimeInfoForDebugger(const EventLogEntry* evt, TTDebuggerSourceLocation& sourceLocation)
        {
#if !ENABLE_TTD_DEBUGGING
            return false;
#else
            const JsRTCallbackAction* cbAction = GetInlineEventDataAs<JsRTCallbackAction, EventKind::CallbackOpActionTag>(evt);

            if(cbAction->RegisterLocation != nullptr && cbAction->RegisterLocation->HasValue())
            {
                sourceLocation.SetLocation(*(cbAction->RegisterLocation));
                return true;
            }
            else
            {
                sourceLocation.Clear();
                return false; //we haven't been re-executed in replay so we don't have our info yet
            }
#endif
        }

        void JsRTCodeParseAction_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTCodeParseAction* cpAction = GetInlineEventDataAs<JsRTCodeParseAction, EventKind::CodeParseActionTag>(evt);
            JsRTCodeParseAction_AdditionalInfo* cpInfo = cpAction->AdditionalInfo;

            Js::JavascriptFunction* function = nullptr;

            LPCWSTR script = cpInfo->SourceCode.Contents;
            uint32 scriptLength = cpInfo->SourceCode.Length;
            DWORD_PTR sourceContext = cpInfo->DocumentID;

            SourceContextInfo * sourceContextInfo = ctx->GetSourceContextInfo(sourceContext, nullptr);

            if(sourceContextInfo == nullptr)
            {
                sourceContextInfo = ctx->CreateSourceContextInfo(sourceContext, cpInfo->SourceUri.Contents, cpInfo->SourceUri.Length, nullptr);
            }

            SRCINFO si = {
                /* sourceContextInfo   */ sourceContextInfo,
                /* dlnHost             */ 0,
                /* ulColumnHost        */ 0,
                /* lnMinHost           */ 0,
                /* ichMinHost          */ 0,
                /* ichLimHost          */ static_cast<ULONG>(scriptLength), // OK to truncate since this is used to limit sourceText in debugDocument/compilation errors.
                /* ulCharOffset        */ 0,
                /* mod                 */ kmodGlobal,
                /* grfsi               */ 0
            };

            Js::Utf8SourceInfo* utf8SourceInfo;
            CompileScriptException se;
            BEGIN_LEAVE_SCRIPT_WITH_EXCEPTION(ctx)
            {
                function = ctx->LoadScript((const byte*)script, scriptLength * sizeof(char16), &si, &se, &utf8SourceInfo, Js::Constants::GlobalCode, cpInfo->LoadFlag);
            }
            END_LEAVE_SCRIPT_WITH_EXCEPTION(ctx);
            AssertMsg(function != nullptr, "Something went wrong");

            Js::FunctionBody* fb = TTD::JsSupport::ForceAndGetFunctionBody(function->GetParseableFunctionInfo());

            ////
            //We don't do this automatically in the eval helper so do it here
            ctx->ProcessFunctionBodyOnLoad(fb, nullptr);
            ctx->RegisterLoadedScript(fb, cpAction->BodyCtrId);

            const HostScriptContextCallbackFunctor& hostFunctor = ctx->GetCallbackFunctor_TTD();
            if(hostFunctor.pfOnScriptLoadCallback != nullptr)
            {
                hostFunctor.pfOnScriptLoadCallback(hostFunctor.HostData, function, utf8SourceInfo, &se);
            }
            ////

            JsRTActionHandleResultForReplay<JsRTCodeParseAction, EventKind::CodeParseActionTag>(ctx, evt, (Js::Var)function);
        }

        void JsRTCodeParseAction_UnloadEventMemory(EventLogEntry* evt, UnlinkableSlabAllocator& alloc)
        {
            JsRTCodeParseAction* cpAction = GetInlineEventDataAs<JsRTCodeParseAction, EventKind::CodeParseActionTag>(evt);
            JsRTCodeParseAction_AdditionalInfo* cpInfo = cpAction->AdditionalInfo;

            alloc.UnlinkString(cpInfo->SourceCode);

            if(!IsNullPtrTTString(cpInfo->SourceUri))
            {
                alloc.UnlinkString(cpInfo->SourceUri);
            }

            if(!IsNullPtrTTString(cpInfo->SourceFile))
            {
                alloc.UnlinkString(cpInfo->SourceFile);
            }

            if(!IsNullPtrTTString(cpInfo->SrcDir))
            {
                alloc.UnlinkString(cpInfo->SrcDir);
            }

            alloc.UnlinkAllocation(cpAction->AdditionalInfo);
        }

        void JsRTCodeParseAction_Emit(const EventLogEntry* evt, LPCWSTR uri, FileWriter* writer, ThreadContext* threadContext)
        {
            const JsRTCodeParseAction* cpAction = GetInlineEventDataAs<JsRTCodeParseAction, EventKind::CodeParseActionTag>(evt);
            JsRTCodeParseAction_AdditionalInfo* cpInfo = cpAction->AdditionalInfo;

            writer->WriteKey(NSTokens::Key::argRetVal, NSTokens::Separator::CommaSeparator);
            NSSnapValues::EmitTTDVar(cpAction->Result, writer, NSTokens::Separator::NoSeparator);

            writer->WriteUInt64(NSTokens::Key::documentId, (uint64)cpInfo->DocumentID, NSTokens::Separator::CommaSeparator);
            writer->WriteTag<LoadScriptFlag>(NSTokens::Key::loadFlag, cpInfo->LoadFlag, NSTokens::Separator::CommaSeparator);

            writer->WriteUInt64(NSTokens::Key::bodyCounterId, cpAction->BodyCtrId, NSTokens::Separator::CommaSeparator);

            writer->WriteString(NSTokens::Key::logDir, cpInfo->SrcDir, NSTokens::Separator::CommaSeparator);
            writer->WriteString(NSTokens::Key::src, cpInfo->SourceFile, NSTokens::Separator::CommaSeparator);
            writer->WriteString(NSTokens::Key::uri, cpInfo->SourceUri, NSTokens::Separator::CommaSeparator);

            writer->WriteLengthValue(cpInfo->SourceCode.Length, NSTokens::Separator::CommaSeparator);

            UtilSupport::TTAutoString docId;
            docId.Append(cpInfo->DocumentID);
            docId.Append(_u("ld"));

            JsSupport::WriteCodeToFile(threadContext->TTDStreamFunctions, cpInfo->SrcDir.Contents, docId.GetStrValue(), cpInfo->SourceUri.Contents, cpInfo->SourceCode.Contents, cpInfo->SourceCode.Length);
        }

        void JsRTCodeParseAction_Parse(EventLogEntry* evt, ThreadContext* threadContext, FileReader* reader, UnlinkableSlabAllocator& alloc)
        {
            JsRTCodeParseAction* cpAction = GetInlineEventDataAs<JsRTCodeParseAction, EventKind::CodeParseActionTag>(evt);
            cpAction->AdditionalInfo = alloc.SlabAllocateStruct<JsRTCodeParseAction_AdditionalInfo>();

            JsRTCodeParseAction_AdditionalInfo* cpInfo = cpAction->AdditionalInfo;

            reader->ReadKey(NSTokens::Key::argRetVal, true);
            cpAction->Result = NSSnapValues::ParseTTDVar(false, reader);

            cpInfo->DocumentID = (DWORD_PTR)reader->ReadUInt64(NSTokens::Key::documentId, true);
            cpInfo->LoadFlag = reader->ReadTag<LoadScriptFlag>(NSTokens::Key::loadFlag, true);

            cpAction->BodyCtrId = reader->ReadUInt64(NSTokens::Key::bodyCounterId, true);

            reader->ReadString(NSTokens::Key::logDir, alloc, cpInfo->SrcDir, true);
            reader->ReadString(NSTokens::Key::src, alloc, cpInfo->SourceFile, true);
            reader->ReadString(NSTokens::Key::uri, alloc, cpInfo->SourceUri, true);

            cpInfo->SourceCode.Length = reader->ReadLengthValue(true);
            cpInfo->SourceCode.Contents = alloc.SlabAllocateArray<wchar>(cpInfo->SourceCode.Length + 1);

            UtilSupport::TTAutoString docId;
            docId.Append(cpInfo->DocumentID);
            docId.Append(_u("ld"));

            JsSupport::ReadCodeFromFile(threadContext->TTDStreamFunctions, cpInfo->SrcDir.Contents, docId.GetStrValue(), cpInfo->SourceUri.Contents, cpInfo->SourceCode.Contents, cpInfo->SourceCode.Length);
        }

#if ENABLE_TTD_INTERNAL_DIAGNOSTICS
        void JsRTCallFunctionAction_ProcessDiagInfoPre(EventLogEntry* evt, Js::JavascriptFunction* function, UnlinkableSlabAllocator& alloc)
        {
            JsRTCallFunctionAction* cfAction = GetInlineEventDataAs<JsRTCallFunctionAction, EventKind::CallExistingFunctionActionTag>(evt);

            Js::JavascriptString* displayName = function->GetDisplayName();
            alloc.CopyStringIntoWLength(displayName->GetSz(), displayName->GetLength(), cfAction->AdditionalInfo->FunctionName);
        }

        void JsRTCallFunctionAction_ProcessDiagInfoPost(EventLogEntry* evt, double wallTime, int64 lastNestedEvent)
        {
            JsRTCallFunctionAction* cfAction = GetInlineEventDataAs<JsRTCallFunctionAction, EventKind::CallExistingFunctionActionTag>(evt);

            cfAction->AdditionalInfo->EndTime = wallTime;
            cfAction->AdditionalInfo->LastNestedEvent = lastNestedEvent;
        }
#endif

        void JsRTCallFunctionAction_ProcessArgs(EventLogEntry* evt, int32 rootDepth, Js::JavascriptFunction* function, uint32 argc, Js::Var* argv, double wallTime, int64 hostCallbackId, int64 topLevelCallbackEventTime, UnlinkableSlabAllocator& alloc)
        {
            JsRTCallFunctionAction* cfAction = GetInlineEventDataAs<JsRTCallFunctionAction, EventKind::CallExistingFunctionActionTag>(evt);
            cfAction->AdditionalInfo = alloc.SlabAllocateStruct<JsRTCallFunctionAction_AdditionalInfo>();

            cfAction->CallbackDepth = rootDepth;
            cfAction->ArgCount = argc + 1;

            static_assert(sizeof(TTDVar) == sizeof(Js::Var), "These need to be the same size (and have same bit layout) for this to work!");

            cfAction->ArgArray = alloc.SlabAllocateArray<TTDVar>(cfAction->ArgCount);
            cfAction->ArgArray[0] = static_cast<TTDVar>(function);
            js_memcpy_s(cfAction->ArgArray + 1, (cfAction->ArgCount -1) * sizeof(TTDVar), argv, argc * sizeof(Js::Var));

            cfAction->AdditionalInfo->BeginTime = wallTime;
            cfAction->AdditionalInfo->EndTime = -1.0;

            cfAction->AdditionalInfo->HostCallbackId = hostCallbackId;
            cfAction->AdditionalInfo->TopLevelCallbackEventTime = topLevelCallbackEventTime;
        }

        void JsRTCallFunctionAction_ProcessReturn(EventLogEntry* evt, Js::Var res, bool hasScriptException, bool hasTerminiatingException)
        {
            JsRTCallFunctionAction* cfAction = GetInlineEventDataAs<JsRTCallFunctionAction, EventKind::CallExistingFunctionActionTag>(evt);
            JsRTCallFunctionAction_AdditionalInfo* cfInfo = cfAction->AdditionalInfo;

            JsRTActionHandleResultForRecord<JsRTCallFunctionAction, EventKind::CallExistingFunctionActionTag>(evt, res);

            cfInfo->HasScriptException = hasScriptException;
            cfInfo->HasTerminiatingException = hasTerminiatingException;
        }

        void JsRTCallFunctionAction_Execute(const EventLogEntry* evt, Js::ScriptContext* ctx)
        {
            const JsRTCallFunctionAction* cfAction = GetInlineEventDataAs<JsRTCallFunctionAction, EventKind::CallExistingFunctionActionTag>(evt);
            JsRTCallFunctionAction_AdditionalInfo* cfInfo = cfAction->AdditionalInfo;

            ThreadContext* threadContext = ctx->GetThreadContext();

            Js::Var jsFunctionVar = JsRTActionInflateVarInReplay(ctx, cfAction->ArgArray[0]);
            Js::JavascriptFunction *jsFunction = Js::JavascriptFunction::FromVar(jsFunctionVar);

            //remove implicit constructor function as first arg in callInfo and argument loop below
            Js::CallInfo callInfo((ushort)(cfAction->ArgCount - 1));
            for(uint32 i = 1; i < cfAction->ArgCount; ++i)
            {
                cfAction->AdditionalInfo->ExecArgs[i] = JsRTActionInflateVarInReplay(ctx, cfAction->ArgArray[i]);
            }
            Js::Arguments jsArgs(callInfo, cfAction->AdditionalInfo->ExecArgs);

            if(cfAction->CallbackDepth == 0)
            {
                threadContext->TTDLog->ResetCallStackForTopLevelCall(cfInfo->TopLevelCallbackEventTime, cfInfo->HostCallbackId);
            }

            Js::Var result = nullptr;
            try
            {
                result = jsFunction->CallRootFunction(jsArgs, ctx, true);
            }
            catch(Js::JavascriptExceptionObject*  exceptionObject)
            {
                AssertMsg(threadContext->GetRecordedException() == nullptr, "Not sure if this is true or not but seems like a reasonable requirement.");

                threadContext->SetRecordedException(exceptionObject);
            }
            catch(Js::ScriptAbortException)
            {
                AssertMsg(threadContext->GetRecordedException() == nullptr, "Not sure if this is true or not but seems like a reasonable requirement.");

                threadContext->SetRecordedException(threadContext->GetPendingTerminatedErrorObject());
            }
            catch(TTDebuggerAbortException)
            {
                throw; //re-throw my abort exception up to the top-level.
            }
            catch(...)
            {
                AssertMsg(false, "What else if dying here?");

                //not sure of our best strategy so just run for now
            }

            //since we tag in JsRT we need to tag here too
            JsRTActionHandleResultForReplay<JsRTCallFunctionAction, EventKind::CallExistingFunctionActionTag>(ctx, evt, result);

#if ENABLE_TTD_DEBUGGING
            if(cfAction->CallbackDepth == 0)
            {
                if(threadContext->TTDLog->HasImmediateReturnFrame())
                {
                    JsRTCallFunctionAction_SetLastExecutedStatementAndFrameInfo(const_cast<EventLogEntry*>(evt), threadContext->TTDLog->GetImmediateReturnFrame());
                }
                else
                {
                    JsRTCallFunctionAction_SetLastExecutedStatementAndFrameInfo(const_cast<EventLogEntry*>(evt), threadContext->TTDLog->GetImmediateExceptionFrame());
                }

                if(cfInfo->HasScriptException || cfInfo->HasTerminiatingException)
                {
                    throw TTDebuggerAbortException::CreateUncaughtExceptionAbortRequest(threadContext->TTDLog->GetCurrentTopLevelEventTime(), _u("Uncaught exception -- Propagate to top-level."));
                }
            }
#endif
        }

        void JsRTCallFunctionAction_UnloadEventMemory(EventLogEntry* evt, UnlinkableSlabAllocator& alloc)
        {
            JsRTCallFunctionAction* cfAction = GetInlineEventDataAs<JsRTCallFunctionAction, EventKind::CallExistingFunctionActionTag>(evt);
            JsRTCallFunctionAction_AdditionalInfo* cfInfo = cfAction->AdditionalInfo;

            alloc.UnlinkAllocation(cfAction->ArgArray);

#if ENABLE_TTD_DEBUGGING
            if(cfInfo->ExecArgs != nullptr)
            {
                alloc.UnlinkAllocation(cfInfo->ExecArgs);
            }

            JsRTCallFunctionAction_UnloadSnapshot(evt);
#endif


#if ENABLE_TTD_INTERNAL_DIAGNOSTICS
            alloc.UnlinkString(cfInfo->FunctionName);
#endif

            alloc.UnlinkAllocation(cfInfo);
        }

        void JsRTCallFunctionAction_Emit(const EventLogEntry* evt, LPCWSTR uri, FileWriter* writer, ThreadContext* threadContext)
        {
            const JsRTCallFunctionAction* cfAction = GetInlineEventDataAs<JsRTCallFunctionAction, EventKind::CallExistingFunctionActionTag>(evt);
            const JsRTCallFunctionAction_AdditionalInfo* cfInfo = cfAction->AdditionalInfo;

            writer->WriteKey(NSTokens::Key::argRetVal, NSTokens::Separator::CommaSeparator);
            NSSnapValues::EmitTTDVar(cfAction->Result, writer, NSTokens::Separator::NoSeparator);

            writer->WriteInt32(NSTokens::Key::rootNestingDepth, cfAction->CallbackDepth, NSTokens::Separator::CommaSeparator);

            writer->WriteLengthValue(cfAction->ArgCount, NSTokens::Separator::CommaSeparator);

            writer->WriteSequenceStart_DefaultKey(NSTokens::Separator::CommaSeparator);
            for(uint32 i = 0; i < cfAction->ArgCount; ++i)
            {
                NSTokens::Separator sep = (i != 0) ? NSTokens::Separator::CommaSeparator : NSTokens::Separator::NoSeparator;
                NSSnapValues::EmitTTDVar(cfAction->ArgArray[i], writer, sep);
            }
            writer->WriteSequenceEnd();

            writer->WriteDouble(NSTokens::Key::beginTime, cfInfo->BeginTime, NSTokens::Separator::CommaSeparator);
            writer->WriteDouble(NSTokens::Key::endTime, cfInfo->EndTime, NSTokens::Separator::CommaSeparator);

            writer->WriteInt64(NSTokens::Key::hostCallbackId, cfInfo->HostCallbackId, NSTokens::Separator::CommaSeparator);
            writer->WriteInt64(NSTokens::Key::eventTime, cfInfo->TopLevelCallbackEventTime, NSTokens::Separator::CommaSeparator);

            writer->WriteBool(NSTokens::Key::boolVal, cfInfo->HasScriptException, NSTokens::Separator::CommaSeparator);
            writer->WriteBool(NSTokens::Key::boolVal, cfInfo->HasTerminiatingException, NSTokens::Separator::CommaSeparator);

#if ENABLE_TTD_INTERNAL_DIAGNOSTICS
            writer->WriteInt64(NSTokens::Key::i64Val, cfInfo->LastNestedEvent, NSTokens::Separator::CommaSeparator);
            writer->WriteString(NSTokens::Key::name, cfInfo->FunctionName, NSTokens::Separator::CommaSeparator);
#endif
        }

        void JsRTCallFunctionAction_Parse(EventLogEntry* evt, ThreadContext* threadContext, FileReader* reader, UnlinkableSlabAllocator& alloc)
        {
            JsRTCallFunctionAction* cfAction = GetInlineEventDataAs<JsRTCallFunctionAction, EventKind::CallExistingFunctionActionTag>(evt);
            cfAction->AdditionalInfo = alloc.SlabAllocateStruct<JsRTCallFunctionAction_AdditionalInfo>();

            reader->ReadKey(NSTokens::Key::argRetVal, true);
            cfAction->Result = NSSnapValues::ParseTTDVar(false, reader);

            cfAction->CallbackDepth = reader->ReadInt32(NSTokens::Key::rootNestingDepth, true);

            cfAction->ArgCount = reader->ReadLengthValue(true);

            reader->ReadSequenceStart_WDefaultKey(true);
            for(uint32 i = 0; i < cfAction->ArgCount; ++i)
            {
                cfAction->ArgArray[i] = NSSnapValues::ParseTTDVar(i != 0, reader);
            }
            reader->ReadSequenceEnd();

            JsRTCallFunctionAction_AdditionalInfo* cfInfo = cfAction->AdditionalInfo;
            cfInfo->BeginTime = reader->ReadDouble(NSTokens::Key::beginTime, true);
            cfInfo->EndTime = reader->ReadDouble(NSTokens::Key::endTime, true);

            cfInfo->HostCallbackId = reader->ReadInt64(NSTokens::Key::hostCallbackId, true);
            cfInfo->TopLevelCallbackEventTime = reader->ReadInt64(NSTokens::Key::eventTime, true);

            cfInfo->HasScriptException = reader->ReadBool(NSTokens::Key::boolVal, true);
            cfInfo->HasTerminiatingException = reader->ReadBool(NSTokens::Key::boolVal, true);

#if ENABLE_TTD_DEBUGGING
            cfInfo->RtRSnap = nullptr;
            cfInfo->ExecArgs = alloc.SlabAllocateArray<Js::Var>(cfAction->ArgCount - 1); //we use an extra space for the function

            cfInfo->LastExecutedLocation.Clear();
#endif

#if ENABLE_TTD_INTERNAL_DIAGNOSTICS
            cfInfo->LastNestedEvent = reader->ReadInt64(NSTokens::Key::i64Val, true);
            reader->ReadString(NSTokens::Key::name, alloc, cfInfo->FunctionName, true);
#endif
        }

        void JsRTCallFunctionAction_UnloadSnapshot(EventLogEntry* evt)
        {
#if ENABLE_TTD_DEBUGGING
            JsRTCallFunctionAction* cfAction = GetInlineEventDataAs<JsRTCallFunctionAction, EventKind::CallExistingFunctionActionTag>(evt);
            JsRTCallFunctionAction_AdditionalInfo* cfInfo = cfAction->AdditionalInfo;

            if(cfInfo->RtRSnap != nullptr)
            {
                HeapDelete(cfInfo->RtRSnap);
                cfInfo->RtRSnap = nullptr;
            }
#endif
        }

        void JsRTCallFunctionAction_SetLastExecutedStatementAndFrameInfo(EventLogEntry* evt, const SingleCallCounter& lastSourceLocation)
        {
#if ENABLE_TTD_DEBUGGING
            JsRTCallFunctionAction* cfAction = GetInlineEventDataAs<JsRTCallFunctionAction, EventKind::CallExistingFunctionActionTag>(evt);
            JsRTCallFunctionAction_AdditionalInfo* cfInfo = cfAction->AdditionalInfo;

            cfInfo->LastExecutedLocation.SetLocation(lastSourceLocation);
#endif
        }

        bool JsRTCallFunctionAction_GetLastExecutedStatementAndFrameInfoForDebugger(const EventLogEntry* evt, TTDebuggerSourceLocation& lastSourceInfo)
        {
#if !ENABLE_TTD_DEBUGGING
            lastSourceInfo.Clear();
            return false;
#else
            const JsRTCallFunctionAction* cfAction = GetInlineEventDataAs<JsRTCallFunctionAction, EventKind::CallExistingFunctionActionTag>(evt);
            JsRTCallFunctionAction_AdditionalInfo* cfInfo = cfAction->AdditionalInfo;
            if(cfInfo->LastExecutedLocation.HasValue())
            {
                lastSourceInfo.SetLocation(cfInfo->LastExecutedLocation);
                return true;
            }
            else
            {
                lastSourceInfo.Clear();
                return false;
            }
#endif
        }
    }
}

#endif