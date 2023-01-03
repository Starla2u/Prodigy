#include <ntifs.h>
#include <wdm.h>

#define IOCTL_INJECT_DLL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)

NTSTATUS DispatchIoctl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    ULONG inputLength = stack->Parameters.DeviceIoControl.InputBufferLength;
    ULONG outputLength = stack->Parameters.DeviceIoControl.OutputBufferLength;
    PVOID inputBuffer = Irp->AssociatedIrp.SystemBuffer;
    PVOID outputBuffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;

    switch (code)
    {
        case IOCTL_INJECT_DLL:
        {
            if (inputLength < sizeof(UNICODE_STRING))
            {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            UNICODE_STRING dllPath;
            RtlCopyMemory(&dllPath, inputBuffer, sizeof(UNICODE_STRING));

            HANDLE processId = (HANDLE)outputBuffer;
            status = InjectDll(processId, &dllPath);
            break;
        }
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS InjectDll(HANDLE processId, PUNICODE_STRING dllPath)
{
    NTSTATUS status = STATUS_SUCCESS;
    HANDLE processHandle = NULL;
    PVOID remoteDllPath = NULL;

    OBJECT_ATTRIBUTES attributes;
    InitializeObjectAttributes(&attributes, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);
    CLIENT_ID clientId;
    clientId.UniqueProcess = processId;
    clientId.UniqueThread = NULL;
    status = ZwOpenProcess(&processHandle, PROCESS_ALL_ACCESS, &attributes, &clientId);
    if (!NT_SUCCESS(status))
    {
        goto cleanup;
    }

    SIZE_T pathSize = dllPath->Length;
    remoteDllPath = ExAllocatePoolWithTag(NonPagedPool, pathSize, 'DLLP');
    if (remoteDllPath == NULL)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    SIZE_T bytesWritten = 0;
    status = ZwWriteVirtualMemory(processHandle, remoteDllPath, dllPath->Buffer, pathSize, &bytesWritten);
    if (!NT_SUCCESS(status) || bytesWritten != pathSize)
    {
        goto cleanup;
    }

    PTHREAD_START_ROUTINE loadLibraryA = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
    if (loadLibraryA == NULL)
    {
        status = STATUS_PROCEDURE_NOT_FOUND;
        goto cleanup;
    }

    HANDLE threadHandle = NULL;
    CLIENT_ID threadClientId;
    status = RtlCreateUserThread(processHandle, NULL, TRUE, NULL, 0, 0, loadLibraryA, remoteDllPath, &threadHandle, &threadClientId);
    if (!NT_SUCCESS(status))
    {
        goto cleanup;
    }

    status = ZwWaitForSingleObject(threadHandle, TRUE, NULL);

cleanup:
    if (remoteDllPath != NULL)
    {
        ExFreePoolWithTag(remoteDllPath, 'DLLP');
    }

    if (threadHandle != NULL)
    {
        ZwClose(threadHandle);
    }

    if (processHandle != NULL)
    {
        ZwClose(processHandle);
    }

    return status;
}

NTSTATUS DispatchRead(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    PVOID buffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG length = stack->Parameters.Read.Length;
    ULONG offset = stack->Parameters.Read.ByteOffset.QuadPart;

    // TODO: Implement read logic here

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = length;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS DispatchWrite(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    PVOID buffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG length = stack->Parameters.Write.Length;
    ULONG offset = stack->Parameters.Write.ByteOffset.QuadPart;

    // TODO: Implement write logic here

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = length;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS DispatchControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;
    PVOID inputBuffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG inputLength = stack->Parameters.DeviceIoControl.InputBufferLength;
    PVOID outputBuffer = Irp->AssociatedIrp.SystemBuffer;
    ULONG outputLength = stack->Parameters.DeviceIoControl.OutputBufferLength;

    // TODO: Implement control logic here

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    NTSTATUS status = STATUS_SUCCESS;

    status = CreateDevice(DriverObject);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE] = DispatchCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DispatchClose;
    DriverObject->MajorFunction[IRP_MJ_READ] = DispatchRead;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = DispatchWrite;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchControl;
    DriverObject->DriverUnload = DriverUnload;

    return status;
}

VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
    DeleteDevice(DriverObject);
}

NTSTATUS DispatchCreate(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS DispatchClose(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}
