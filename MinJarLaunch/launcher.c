#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#else
#include <unistd.h>
#endif

#ifdef _WIN32
static void showErrorAndExit(const char* message) {
    int len = MultiByteToWideChar(CP_ACP, 0, message, -1, NULL, 0);
    wchar_t* wMessage = (wchar_t*)malloc(len * sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP, 0, message, -1, wMessage, len);

    MessageBoxW(NULL, wMessage, L"Error!", MB_ICONERROR | MB_OK);
    free(wMessage);
    exit(EXIT_FAILURE);
}
#else
static void showErrorAndExit(const char* message) {
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
}
#endif

static int is_java_in_path() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    DWORD result = SearchPath(NULL, "java.exe", NULL, MAX_PATH, buffer, NULL);
    return result > 0;
#else
    const char* path = getenv("PATH");
    if (path == NULL) {
        return 0;
    }

    char* path_copy = strdup(path);
    char* token = strtok(path_copy, ":");
    while (token != NULL) {
        char java_path[4096];
        snprintf(java_path, sizeof(java_path), "%s/java", token);
        if (access(java_path, X_OK) == 0) {
            free(path_copy);
            return 1;
        }
        token = strtok(NULL, ":");
    }
    free(path_copy);
    return 0;
#endif
}

#ifdef _WIN32
int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ PWSTR szCmdLine,
    _In_ int cmdShow) {

    if (is_java_in_path()) {
        int argc;
        LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

        if (argc > 1) {
            char command[4096];
            int ret = snprintf(command, sizeof(command), "javaw -jar \"%S\"", argv[1]);
            if (ret < 0 || ret >= sizeof(command)) {
                showErrorAndExit("Command buffer overflow!");
            }

            for (int i = 2; i < argc; i++) {
                ret = snprintf(command + strlen(command), sizeof(command) - strlen(command), " \"%S\"", argv[i]);
                if (ret < 0 || ret >= sizeof(command) - strlen(command)) {
                    showErrorAndExit("Command buffer overflow!");
                }
            }

            STARTUPINFO si;
            PROCESS_INFORMATION pi;
            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            if (!CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                showErrorAndExit("javaw.exe not found!");
            }

            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else {
            MessageBoxW(NULL, L"No file specified!", L"Info!", MB_ICONEXCLAMATION | MB_OK);
            LocalFree(argv);
            return EXIT_FAILURE;
        }

        LocalFree(argv);
    }
    else {
        MessageBoxW(NULL, L"JAVA not found!\n\nDownload and install a JRE.", L"Info!", MB_ICONEXCLAMATION | MB_OK);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
#else
int main(int argc, char* argv[]) {
    if (is_java_in_path()) {
        if (argc > 1) {
            char* javaCmd[argc + 2];
            javaCmd[0] = "java";
            javaCmd[1] = "-jar";
            for (int i = 1; i < argc; i++) {
                javaCmd[i + 1] = argv[i];
            }
            javaCmd[argc + 1] = NULL;

            execvp("java", javaCmd);
            perror("Error: java not found or error executing command");
            return EXIT_FAILURE;
        }
        else {
            fprintf(stderr, "No file specified!\n");
            return EXIT_FAILURE;
        }
    }
    else {
        fprintf(stderr, "JAVA not found!\n\nDownload and install a JRE.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
#endif
