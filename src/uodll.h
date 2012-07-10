#define BUILDING_NODE_EXTENSION
#include <node.h>
#include <string>

using namespace v8;
using namespace node;

typedef int (__stdcall *OPEN)();
typedef void (__stdcall *CLOSE)(int);
typedef int (__stdcall *VERSION)();
typedef void (__stdcall *PUSHNIL)(int);
typedef void (__stdcall *PUSHBOOLEAN)(int, bool);
typedef void (__stdcall *PUSHINTEGER)(int, int);
typedef void (__stdcall *PUSHDOUBLE)(int, double);
typedef void (__stdcall *PUSHSTRREF)(int, char*);
typedef void (__stdcall *PUSHSTRVAL)(int, char*);
typedef bool (__stdcall *GETBOOLEAN)(int, int);
typedef int (__stdcall *GETINTEGER)(int, int);
typedef double (__stdcall *GETDOUBLE)(int, int);
typedef char* (__stdcall *GETSTRING)(int, int);
typedef int (__stdcall *GETTOP)(int);
typedef int (__stdcall *GETTYPE)(int, int);
typedef void (__stdcall *INSERT)(int, int);
typedef void (__stdcall *PUSHVALUE)(int, int);
typedef void (__stdcall *REMOVE)(int, int);
typedef void (__stdcall *SETTOP)(int, int);
typedef void (__stdcall *MARK)();
typedef void (__stdcall *CLEAN)();
typedef int (__stdcall *EXECUTE)(int);

struct DllCallParams {
  int type;
  std::string string_param;
  char* char_param;
  int number_param;
  bool boolean_param;
};

struct AsyncCallParams {
  int handle;
  int param_length;
  int result_length;
  DllCallParams* params;
  DllCallParams* result;
  Persistent<Function> callback;
};