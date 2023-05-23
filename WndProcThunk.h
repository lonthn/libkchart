/*
 *    thunk with DEP support
 *
 *    author : proguru
 *    July 9,2008
 *
 *    link : http://www.cppblog.com/proguru/archive/2008/08/24/59831.html
 */
#ifndef __KTHUNK_H__
#define __KTHUNK_H__

#ifdef WIN32

#include <cassert>
#include <cstdint>
#include <windows.h>

//#define USE_THISCALL_CONVENTION    //turn it off for c++ builder compatibility

#ifdef USE_THISCALL_CONVENTION
    #define WNDPROC_THUNK_LENGTH    29     // For __thiscall calling convention ONLY, assign hWnd by thunk
    #define GENERAL_THUNK_LENGTH    10
    #define KCALLBACK                      //__thiscall is default
#else
    #define WNDPROC_THUNK_LENGTH    23     //__stdcall calling convention ONLY,assign m_hWnd by thunk
    #define GENERAL_THUNK_LENGTH    16
    #define KCALLBACK __stdcall
#endif

static HANDLE GV_HeapExecutable = NULL;

class ThunkBase {
public:
    ThunkBase(uint32_t size) {
        if(!GV_HeapExecutable) {
            // First thunk, Create the executable heap
			GV_HeapExecutable = ::HeapCreate(
                HEAP_CREATE_ENABLE_EXECUTE, 0, 0
            );
			assert(GV_HeapExecutable);
        }

		machineCodes_ = (uint8_t*)::HeapAlloc(
            GV_HeapExecutable, HEAP_ZERO_MEMORY, size
        );
    }
    
	~ThunkBase() {
        if (GV_HeapExecutable) {
			::HeapFree(GV_HeapExecutable, 0, (void *) machineCodes_);
		}
    }

    inline LONG_PTR GetThunkedCodePtr() {
		return reinterpret_cast<LONG_PTR>(&machineCodes_[0]);
	}

protected:
	uint8_t* machineCodes_;
}; // class ThunkBase


class WndProcThunk : public ThunkBase {
public:
    WndProcThunk()
		: ThunkBase(WNDPROC_THUNK_LENGTH) {}

    void init(uintptr_t thisPtr, uintptr_t procPtr) {
#ifndef _WIN64
        uint32_t distance = (uint32_t) (procPtr)
                          - (uint32_t) (&machineCodes_[0])
                          - WNDPROC_THUNK_LENGTH;

    #ifdef USE_THISCALL_CONVENTION
        /*
        For __thiscall, the default calling convention used by Microsoft VC++, The thing needed is
        just set ECX with the value of 'this pointer'

        machine code                       assembly instruction        comment
        ---------------------------       -------------------------    ----------
        B9 ?? ?? ?? ??                    mov ecx, pThis                ; Load ecx with this pointer
        50                                PUSH EAX            
        8B 44 24 08                       MOV EAX, DWORD PTR[ESP+8]     ; EAX=hWnd
        89 01                             MOV DWORD PTR [ECX], EAX      ; [pThis]=[ECX]=hWnd
        8B 44 24 04                       mov eax,DWORD PTR [ESP+04H]   ; eax=(return address)
        89 44 24 08                       mov DWORD PTR [ESP+08h],eax   ; hWnd=(return address)
        58                                POP EAX
        83 C4 04                          add ESP,04h
                        
        E9 ?? ?? ?? ??                    jmp ProcPtr                   ; Jump to target message handler
        */
		machineCode_[0]              = 0xB9;
        *((uint32_t*)&machineCode_[ 1]) = (uint32_t)thisPtr;
        *((uint32_t*)&machineCode_[ 5]) = 0x24448B50;
        *((uint32_t*)&machineCode_[ 9]) = 0x8B018908;
        *((uint32_t*)&machineCode_[13]) = 0x89042444;
        *((uint32_t*)&machineCode_[17]) = 0x58082444;
        *((uint32_t*)&machineCode_[21]) = 0xE904C483;
        *((uint32_t*)&machineCode_[25]) = distance;
    #else    
        /*
         * 01/26/2008 modify
        For __stdcall calling convention, replace 'HWND' with 'this pointer'

        Stack frame before modify             Stack frame after modify

        :            :                        :             :
        |---------------|                        |----------------|
        |     lParam    |                        |     lParam     |
        |---------------|                        |----------------|
        |     wParam    |                        |     wParam     |
        |---------------|                        |----------------|
        |     uMsg      |                        |     uMsg       |
        |---------------|                        |----------------|
        |     hWnd      |                        | <this pointer> |
        |---------------|                        |----------------|
        | (Return Addr) | <- ESP                 | (Return Addr)  | <-ESP
        |---------------|                        |----------------|
        :            :                        :             | 

        machine code        assembly instruction            comment    
        ------------------- ----------------------------    --------------
        51                  push ecx
        B8 ?? ?? ?? ??      mov  eax, pThis                  ; eax=this;
        8B 4C 24 08         mov  ecx, dword ptr [esp+08H]    ; ecx=hWnd;
        89 08               mov  dword ptr [eax], ecx        ; [this]=hWnd,if has vftbl should [this+4]=hWnd
        89 44 24 08         mov  dword ptr [esp+08H], eax    ; Overwite the 'hWnd' with 'this pointer'
        59                  pop  ecx
        E9 ?? ?? ?? ??      jmp  ProcPtr                     ; Jump to target message handler
        */

        *((uint32_t*)&machineCodes_[ 0]) = 0xB851;
        *((uint32_t*)&machineCodes_[ 2]) = (uint32_t)thisPtr;
        *((uint32_t*)&machineCodes_[ 6]) = 0x08244C8B;
		*((uint32_t*)&machineCodes_[10]) = 0x4489;//0889;
        *((uint32_t*)&machineCodes_[12]) = 0xE9590824;
        *((uint32_t*)&machineCodes_[16]) = (uint32_t)distance;
    #endif //USE_THISCALL_CONVENTION
#else    //_WIN64
        /* 
        For x64 calling convention, RCX hold the 'HWND',copy the 'HWND' to Window object,
        then insert 'this pointer' into RCX,so perfectly!!!        

        Stack frame before modify                Stack frame after modify

        :               :                        :                :
        |---------------|                        |----------------|
        |               | <-R9(lParam)           |                | <-R9(lParam)
        |---------------|                        |----------------|
        |               | <-R8(wParam)           |                | <-R8(wParam)
        |---------------|                        |----------------|
        |               | <-RDX(msg)             |                | <-RDX(msg)
        |---------------|                        |----------------|
        |               | <-RCX(hWnd)            |                | <-RCX(this)
        |---------------|                        |----------------|
        | (Return Addr) | <-RSP                  | (Return Addr)  | <-RSP
        |---------------|                        |----------------|
        :               :                        :                :

        machine code            assembly instruction     comment
        -------------------       -----------------------    ----
        48B8 ????????????????   mov RAX,pThis
        ; 4808                    mov qword ptr [RAX],RCX    ;m_hWnd=[this]=RCX
        4889C1                  mov RCX,RAX                ;RCX=pThis
        48B8 ????????????????   mov RAX,ProcPtr
        FFE0                    jmp RAX        
        */
        *((uint16_t*)&machineCodes_[ 0]) = 0xB848;
        *((uint64_t*)&machineCodes_[ 2]) = thisPtr;
        // [@author:luo-zeqi]
        // 由于使用该类的 KChartWnd 存在虚函数，其首地址所存的值被虚函数表
        // 占用，因此下面这行汇编会破坏内存，且这目前没有意义，先将其注释。
        //*((uint32_t*)&machineCodes_[10]) = 0x48088948;
        //*((uint32_t*)&machineCodes_[14]) = 0xB848C189;
        *((uint32_t*)&machineCodes_[10]) = 0x48C18948;
        *((uint32_t*)&machineCodes_[14]) = 0xB8;
        *((uint64_t*)&machineCodes_[15]) = procPtr;
        *((uint16_t*)&machineCodes_[23]) = 0xE0FF;
#endif
    }
}; // class WndProcThunk

#endif // WIN32

#endif