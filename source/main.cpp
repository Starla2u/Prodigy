#include <Windows.h>
#include <stdio.h>

/* GUI currently not implemented (1/3/2023) */

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: injectdll.exe <process id> <dll path>\n");
        return 1;
    }

    DWORD processId = atoi(argv[1]);
    char *dllPath = argv[2];

    HANDLE hDevice = CreateFile("\\\\.\\Prodigy", GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) {
        printf("Error: Could not open device.\n");
        return 2;
    }

    DWORD inBuffer[2];
    inBuffer[0] = processId;
    inBuffer[1] = (DWORD)strlen(dllPath) + 1;
    char outBuffer[256];

    DWORD bytesReturned;
    BOOL success = DeviceIoControl(hDevice, IOCTL_INJECT_DLL, inBuffer, sizeof(inBuffer),
        outBuffer, sizeof(outBuffer), &bytesReturned, NULL);

    if (!success) {
        printf("Error: IOCTL failed.\n");
        CloseHandle(hDevice);
        return 3;
    }

    DWORD bytesWritten;
    success = WriteFile(hDevice, dllPath, inBuffer[1], &bytesWritten, NULL);
    if (!success || bytesWritten != inBuffer[1]) {
        printf("Error: Could not send DLL path to driver.\n");
        CloseHandle(hDevice);
        return 4;
    }

    success = ReadFile(hDevice, outBuffer, sizeof(outBuffer), &bytesReturned, NULL);
    if (!success || bytesReturned == 0) {
        printf("Error: Could not read result from driver.\n");
        CloseHandle(hDevice);
        return 5;
    }

    printf("%s\n", outBuffer);
    CloseHandle(hDevice);
    return 0;
}
