#include <node.h>
#include "uodll.h"

using namespace v8;
using namespace node;

OPEN Open;
CLOSE Close;
VERSION Version;
PUSHNIL PushNil;
PUSHBOOLEAN PushBoolean;
PUSHINTEGER PushInteger;
PUSHDOUBLE PushDouble;
PUSHSTRREF PushStrRef;
PUSHSTRVAL PushStrVal;
GETBOOLEAN GetBoolean;
GETINTEGER GetInteger;
GETDOUBLE GetDouble;
GETSTRING GetString;
GETTOP GetTop;
GETTYPE GetType;
INSERT Insert;
PUSHVALUE PushValue;
REMOVE Remove;
SETTOP SetTop;
MARK Mark;
CLEAN Clean;
EXECUTE Execute;

void asyncCall (uv_work_t *req) {
  AsyncCallParams *r = (AsyncCallParams *)req->data;
  DllCallParams *p = (DllCallParams *)r->params;
  
  SetTop(r->handle, 0);
  for(int i = 0;i<r->param_length;i++) {
    switch (p[i].type) {
      case 1:
        PushBoolean(r->handle, p[i].boolean_param);
        break;
      case 3:
        PushInteger(r->handle, p[i].number_param);
        break;
      case 4:
        PushStrRef(r->handle, (char*)p[i].string_param.c_str());
        break;
    }
  }
  Execute(r->handle);
  r->result_length = GetTop(r->handle);
  int result_type, num;
  DllCallParams *res = new DllCallParams[r->result_length];
  for(int i = 0;i<r->result_length;i++) {
    res[i].type = GetType(r->handle, i+1);
    switch (res[i].type) {
      case 1:
        res[i].boolean_param = GetBoolean(r->handle, i+1);
        break;
      case 3:
        num = GetInteger(r->handle, i+1);
        res[i].number_param = num;
        break;
      case 4:
        res[i].char_param = GetString(r->handle, i+1);
        break;
    }
  }
  r->result = res;
}

void afterAsyncCall (uv_work_t *req) {
  HandleScope scope;
  
  AsyncCallParams *r = (AsyncCallParams *)req->data;
  DllCallParams *res = (DllCallParams *)r->result;
  
  Handle<Value> argv[1];
  Local<Array> arr = Array::New(r->result_length);
  for(int i = 0;i<r->result_length;i++) {
    switch (res[i].type) {
      case 0:
        arr->Set(i, Null());
        break;
      case 1:
        arr->Set(i, Boolean::New(res[i].boolean_param));
        break;
      case 3:
        arr->Set(i, Integer::New(res[i].number_param));
        break;
      case 4:
        arr->Set(i, String::New(res[i].char_param));
        break;
    }
  }
  argv[0] = arr;
  
  TryCatch try_catch;
  r->callback->Call(Context::GetCurrent()->Global(), 1, argv);

  // cleanup
  r->callback.Dispose();
  r->callback.Clear();
  delete [] r->params;
  delete [] r->result;
  delete r;
  delete req;

  if (try_catch.HasCaught())
    FatalException(try_catch);
}

Handle<Value> startAsyncCall (const Arguments& args) {
  HandleScope scope;
  
  if (args.Length() < 3) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
  }

  if (!args[0]->IsNumber() || !args[1]->IsArray() || !args[2]->IsFunction()) {
    ThrowException(Exception::TypeError(String::New("Wrong arguments")));
    return scope.Close(Undefined());
  }

  uv_work_t *req = new uv_work_t;
  AsyncCallParams *r = new AsyncCallParams;
  req->data = r;
  Local<Array> a = Local<Array>::Cast(args[1]);
  Local<Value> v;
  DllCallParams *p = new DllCallParams[a->Length()];
  for(int i=0;i<a->Length();i++) {
    v = a->Get(i);
    if(v->IsNumber()) {
      p[i].type = 3;
      p[i].number_param = v->NumberValue();
    } else if (v->IsString()) {
      p[i].type = 4;
      p[i].string_param = std::string(*String::Utf8Value(v->ToString()));
    } else if (v->IsBoolean()) {
      p[i].type = 1;
      p[i].boolean_param = v->BooleanValue();      
    }
  }
  r->handle = args[0]->NumberValue();
  r->param_length = a->Length();
  r->params = p;

  r->callback = Persistent<Function>::New(Local<Function>::Cast(args[2]));

  uv_queue_work(uv_default_loop(), req, asyncCall, afterAsyncCall);

  return Undefined();
}

Handle<Value> GetHandle(const Arguments& args) {
  HandleScope scope;
  return scope.Close(Integer::New(Open()));
}
Handle<Value> CloseHandle(const Arguments& args) {
  HandleScope scope;
  Close(args[0]->NumberValue());
  return scope.Close(Undefined());
}

void Init(Handle<Object> target) {
  HINSTANCE hDLL = LoadLibrary("uo.dll");
  if (hDLL != NULL) {
    Open = (OPEN)GetProcAddress(hDLL, "Open");
    Close = (CLOSE)GetProcAddress(hDLL, "Close");
    Version = (VERSION)GetProcAddress(hDLL, "Version");
    PushNil = (PUSHNIL)GetProcAddress(hDLL, "PushNil");
    PushBoolean = (PUSHBOOLEAN)GetProcAddress(hDLL, "PushBoolean");
    PushInteger = (PUSHINTEGER)GetProcAddress(hDLL, "PushInteger");
    PushDouble = (PUSHDOUBLE)GetProcAddress(hDLL, "PushDouble");
    PushStrRef = (PUSHSTRREF)GetProcAddress(hDLL, "PushStrRef");
    PushStrVal = (PUSHSTRVAL)GetProcAddress(hDLL, "PushStrVal");
    GetBoolean = (GETBOOLEAN)GetProcAddress(hDLL, "GetBoolean");
    GetInteger = (GETINTEGER)GetProcAddress(hDLL, "GetInteger");
    GetDouble = (GETDOUBLE)GetProcAddress(hDLL, "GetDouble");
    GetString = (GETSTRING)GetProcAddress(hDLL, "GetString");
    GetTop = (GETTOP)GetProcAddress(hDLL, "GetTop");
    GetType = (GETTYPE)GetProcAddress(hDLL, "GetType");
    Insert = (INSERT)GetProcAddress(hDLL, "Insert");
    PushValue = (PUSHVALUE)GetProcAddress(hDLL, "PushValue");
    Remove = (REMOVE)GetProcAddress(hDLL, "Remove");
    SetTop = (SETTOP)GetProcAddress(hDLL, "SetTop");
    Mark = (MARK)GetProcAddress(hDLL, "Mark");
    Clean = (CLEAN)GetProcAddress(hDLL, "Clean");
    Execute = (EXECUTE)GetProcAddress(hDLL, "Execute");
    if (!Open) {
      FreeLibrary(hDLL);
    }
  }
  
  target->Set(String::NewSymbol("getHandle"), FunctionTemplate::New(GetHandle)->GetFunction());
  target->Set(String::NewSymbol("closeHandle"), FunctionTemplate::New(CloseHandle)->GetFunction());
  target->Set(String::NewSymbol("call"), FunctionTemplate::New(startAsyncCall)->GetFunction());
}

NODE_MODULE(uodll, Init)