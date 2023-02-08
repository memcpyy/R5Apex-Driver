#pragma once
#include "sharedmem.h"

#define RVA(addr, size) (BYTE*)addr + *(INT*)((BYTE*)addr + ((size) - 4)) + size

namespace Driver {

	/*
	NOT MY FUNCTION BUT I THOUGHT IT WOULD SUFFICE IN DISPLAYING 
	HOOKING AND INITILIZING THE SHARED MEMORY
	*/
	INT64 NTAPI EnumerateDebuggingDevicesHook(PVOID A1, PINT64 A2) {
		if (ExGetPreviousMode() != UserMode
			|| A1 == nullptr
			|| !Utils::ProbeUserAddress(A1, sizeof(gData), sizeof(DWORD))
			|| !Memory::Copy(&gData, A1, sizeof(CommunicationData))
			|| gData.Magic != 0x999) {

			

			return EnumerateDebuggingDevicesOriginal(A1, A2);
		}

		
		InterlockedExchangePointer((PVOID*)gFunc, (PVOID)EnumerateDebuggingDevicesOriginal);
		
		SharedMemory::Loop();
	}

	NTSTATUS Initialize() {
		auto OSInfo{ System::GetOSVersion() };

		if (OSInfo.dwBuildNumber < 19041) {
			ActiveThreadsOffset = OSInfo.dwBuildNumber == 10240 ? 0x490 : 0x498;
		}

		if (gKernelBase = System::GetModuleInfo<char*>("ntoskrnl.exe")) {
			if (auto Func = Utils::FindPatternImage(gKernelBase,
				"\x48\x8B\x05\x00\x00\x00\x00\x75\x07\x48\x8B\x05\x00\x00\x00\x00\xE8\x00\x00\x00\x00", //finding the pattern of the function we are hooking
				"xxx????xxxxx????x????")) {

				gFunc = (DWORD64)(Func = RVA(Func, 7));
				*(PVOID*)&EnumerateDebuggingDevicesOriginal = InterlockedExchangePointer((PVOID*)Func, (PVOID)EnumerateDebuggingDevicesHook); // Hook EnumerateDebuggingDevices()
			

				return STATUS_SUCCESS; //MOVE INSIDE OF ELSE
			}
		}

		return STATUS_UNSUCCESSFUL;
	}
}