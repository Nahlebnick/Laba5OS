#ifndef PROCESS_H
#define PROCESS_H

#include <iostream>
#include <functional>
#include <windows.h>
#include <format>

namespace myLib {
class Process
{
	PROCESS_INFORMATION pi{};
public:
	Process(const std::wstring& commandLine,
			bool bInheritHandles = false,
			DWORD dwCreationGlags = 0,
			const std::wstring& currentDirectory = L""
		);              

	Process(const Process&) = delete;
	Process& operator=(const Process&) = delete;

	Process(Process&& other) noexcept;
	Process& operator=(Process&& other) noexcept;
	~Process() noexcept { close(); }
	
	void wait(DWORD wait=INFINITE);

	bool valid() {return pi.hProcess != nullptr;}

	bool terminate(UINT exitCode = 1);

	DWORD GetThreadID() const noexcept { return pi.dwThreadId; }
	DWORD GetProcessID() const noexcept { return pi.dwProcessId; }
	HANDLE GetThreadHandle() const noexcept { return pi.hThread; }
	HANDLE GetProcessHandle() const noexcept { return pi.hProcess; }

private:
	void close();
};

}
#endif // PROCESS_H