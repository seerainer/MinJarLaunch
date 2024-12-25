#include <process.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>

int
WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ PWSTR szCmdLine,
    _In_ int cmdShow) {

    if (wcslen(szCmdLine) > 0) {
        intptr_t x = _wexeclp(L"javaw.exe", L"javaw.exe", L"-jar", szCmdLine, NULL);

        if (x < 0) {
            MessageBoxW(NULL, L"javaw.exe not found!", L"Error!", MB_ICONERROR | MB_OK);

            return EXIT_FAILURE;
        }
    } else {
        MessageBoxW(NULL, L"No file specified!", L"Info!", MB_ICONEXCLAMATION | MB_OK);

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
