
#include "def.h"

  

NTSTATUS ReaperEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
    return Driver::Initialize();
    
    return STATUS_SUCCESS;
}