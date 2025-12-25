#include "process.h"
#include <vector>
#include <system_error>
#include <stdexcept>


myLib::Process::Process(const std::wstring& commandLine, bool bInheritHandles, DWORD dwCreationFlags, const std::wstring& currentDirectory) 
{
    if (commandLine.empty()) {
        throw std::invalid_argument("Command line cannot be empty");
    }

    std::vector<wchar_t> cmdBuffer(commandLine.begin(), commandLine.end());
    cmdBuffer.push_back(0);

    STARTUPINFOW si = { sizeof(STARTUPINFOW) };
    pi = {};

    LPCWSTR lpCurrDir = currentDirectory.empty() ? nullptr : currentDirectory.c_str();

    if (!CreateProcessW(NULL, cmdBuffer.data(),
        NULL, NULL, bInheritHandles, dwCreationFlags, NULL, lpCurrDir, &si, &pi))
    {
        throw std::system_error(GetLastError(), std::system_category(), "CreateProcess failed" );
    }
}

myLib::Process::Process(Process&& other) noexcept
{
    pi = other.pi;
    other.pi = { nullptr, nullptr, 0, 0 };
}

myLib::Process& myLib::Process::operator=(Process&& other) noexcept
{
    if (this != &other) {
        close();
        pi = other.pi;
        other.pi = { nullptr, nullptr, 0, 0 };
    }
    return *this;
}

void myLib::Process::wait(DWORD wait)
{
    if (pi.hProcess)
    {
        DWORD res = WaitForSingleObject(pi.hProcess, wait);
        switch (res)
        {
        case WAIT_OBJECT_0: break;
        case WAIT_FAILED: throw std::system_error(static_cast<int>(GetLastError()), std::system_category(), "WaitForSingleObject failed"); break;
        case WAIT_TIMEOUT: throw std::runtime_error("Waiting time expired!"); break;
        default: throw std::runtime_error("Unexpected result from WaitForSingleObject");
            break;
        }
    }   
}

bool myLib::Process::terminate(UINT exitCode)
{
    bool res = TerminateProcess(pi.hProcess, exitCode);
    return res;
}

void myLib::Process::close()
{
    if (pi.hProcess)
    {
        CloseHandle(pi.hProcess);
    }

    if (pi.hThread)
    {
        CloseHandle(pi.hThread);
    }

    pi.hProcess = nullptr;
    pi.hThread = nullptr;
}
