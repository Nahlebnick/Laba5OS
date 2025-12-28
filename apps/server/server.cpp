#define NOMINMAX
#include <Windows.h>
#include <iostream>
#include <string>
#include <limits> 
#include <vector>
#include <fstream>
#include "process.h"
#include "thread.h"
#include "common.h"

std::vector<SRWLOCK> recordLocks; 
CRITICAL_SECTION fileCS;          

struct ThreadInfo
{
    HANDLE hPipe;
    HANDLE hFile;
    int clientID;
};

DWORD WINAPI handleClient(LPVOID lpParam)
{
    ThreadInfo* p = static_cast<ThreadInfo*>(lpParam);
    HANDLE hNamedPipe = p->hPipe;
    HANDLE hFile = p->hFile;
    int id = p->clientID;

    delete p;

    bool running = true;
    while(running)
    {
        DWORD dwBytesRead;
        DWORD bytesWritten;

        Request req;
        Response res;
        if (!ReadFile(hNamedPipe, &req, sizeof(req), &dwBytesRead, NULL))
        {
            std::cerr << "Read file failed." << std::endl
            << "The last error code: " << GetLastError() << std::endl;
            std::cout << "Press any key to continue...";
            _getche();
            break;
        }
                    
        if (req.operation == OP_EXIT)
        {
            std::cout << "Client " << id << " requested exit." << std::endl;
            res.success = true;
            WriteFile(hNamedPipe, &res, sizeof(Response), &bytesWritten, NULL);
            break;
        }

        if (req.operation == OP_READ)
        {
            AcquireSRWLockShared(&recordLocks[req.employeeID]);


            Employee emp;
            LARGE_INTEGER offset;
            offset.QuadPart = req.employeeID * sizeof(Employee);

            EnterCriticalSection(&fileCS);
            SetFilePointerEx(hFile, offset, NULL, FILE_BEGIN);
            if (!ReadFile(hFile, &emp, sizeof(Employee), &dwBytesRead, NULL))
            {
                ReleaseSRWLockShared(&recordLocks[req.employeeID]);
                std::cerr << "Read file failed." << std::endl
                    << "The last error code: " << GetLastError() << std::endl;
                std::cout << "Press any key to continue...";
                _getche();
                break;
            }
            res.success = true;
            res.employee = emp;

            if (!WriteFile(hNamedPipe, &res, sizeof(Response), &bytesWritten, NULL))
            {
                ReleaseSRWLockShared(&recordLocks[req.employeeID]);
                std::cerr << "Failed to write to pipe" << std::endl
                << "The last error code: " << GetLastError() << std::endl;
                std::cout << "Press any key to continue...";
                _getche();
                break;
            }

            ReleaseSRWLockShared(&recordLocks[req.employeeID]);
        }

        else if (req.operation == OP_WRITE)
        {
            AcquireSRWLockExclusive(&recordLocks[req.employeeID]);

            Employee emp;
            LARGE_INTEGER offset;
            offset.QuadPart = req.employeeID * sizeof(Employee);

            EnterCriticalSection(&fileCS);
            SetFilePointerEx(hFile, offset, NULL, FILE_BEGIN);
            BOOL readSuccess = ReadFile(hFile, &emp, sizeof(Employee), &dwBytesRead, NULL);
            LeaveCriticalSection(&fileCS);
            if (!readSuccess)
            {
                ReleaseSRWLockExclusive(&recordLocks[req.employeeID]);
                std::cerr << "Read file failed." << std::endl
                    << "The last error code: " << GetLastError() << std::endl;
                    std::cout << "Press any key to continue...";
                    _getche();
                break;
            }
            res.success = true;
            res.employee = emp;

            if (!WriteFile(hNamedPipe, &res, sizeof(Response), &bytesWritten, NULL))
            {
                ReleaseSRWLockExclusive(&recordLocks[req.employeeID]);
                std::cerr << "Failed to write to pipe" << std::endl
                << "The last error code: " << GetLastError() << std::endl;
                std::cout << "Press any key to continue...";
                _getche();
                break;
            }

            if (!ReadFile(hNamedPipe, &emp, sizeof(Employee), &dwBytesRead, NULL))
            {
                ReleaseSRWLockExclusive(&recordLocks[req.employeeID]);
                std::cerr << "Read pipe failed." << std::endl
                    << "The last error code: " << GetLastError() << std::endl;
                    std::cout << "Press any key to continue...";
                    _getche();
                break;
            }

            EnterCriticalSection(&fileCS);            
            LARGE_INTEGER moveBack;
            moveBack.QuadPart = -(LONGLONG)sizeof(Employee);
            SetFilePointerEx(hFile, moveBack, NULL, FILE_CURRENT); 

            BOOL writeSuccess = WriteFile(hFile, &emp, sizeof(Employee), &bytesWritten, NULL);

            LeaveCriticalSection(&fileCS);
            if (!writeSuccess)
            {
                ReleaseSRWLockExclusive(&recordLocks[req.employeeID]);
                std::cerr << "Read file failed." << std::endl
                    << "The last error code: " << GetLastError() << std::endl;
                    std::cout << "Press any key to continue...";
                    _getche();
                break;
            }

            res.success = true; 
            res.employee = emp;
            if (!WriteFile(hNamedPipe, &res, sizeof(Response), &bytesWritten, NULL))
            {
                ReleaseSRWLockExclusive(&recordLocks[req.employeeID]);
                std::cerr << "Read file failed." << std::endl
                    << "The last error code: " << GetLastError() << std::endl;
                    std::cout << "Press any key to continue...";
                    _getche();
                break;
            }
            ReleaseSRWLockExclusive(&recordLocks[req.employeeID]);
        }        
    }


    FlushFileBuffers(hNamedPipe);
    DisconnectNamedPipe(hNamedPipe);
    CloseHandle(hNamedPipe);
    std::cout << "Client " << id << " disconnected." << std::endl;
    return 0;
}

int main()
{
    InitializeCriticalSection(&fileCS);
    std::string fileName;
    int numRecords;
    std::cout << "Enter file name: "; std::cin >> fileName;
    std::cout << "Enter number of records: "; std::cin >> numRecords;

    recordLocks.resize(numRecords);
    for (int i = 0; i < numRecords; ++i)
    {
        InitializeSRWLock(&recordLocks[i]);
    }

    HANDLE hFile = CreateFileA(fileName.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    
    for (int i = 0; i < numRecords; ++i)
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        Employee emp;
        std::cout << "Employee " << i + 1 << ": ";
        emp.num = i;
        std::cin >> emp;
        DWORD written;
        WriteFile(hFile, &emp, sizeof(Employee), &written, NULL);
    }

    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    Employee emp;
    DWORD read;
    std::cout << "\n--- File ---\nID   \tName\t\tHours\n";
    for (int i = 0; i < numRecords; ++i)
    {
        ReadFile(hFile, &emp, Employee::SERIALIZED_SIZE, &read, NULL);
        std::cout << emp << std::endl;
    }
    std::cout << "------------------------\n";

    std::cout << "Enter number of clients to spawn: ";
    int num_clients;
    std::cin >> num_clients;

    std::vector<std::unique_ptr<myLib::Thread>> threads;
    std::vector<std::unique_ptr<myLib::Process>> processes;

    for (int i = 0; i < num_clients; i++)
    {
        auto proc = std::make_unique<myLib::Process>(L"client.exe", false, CREATE_NEW_CONSOLE);
        processes.push_back(std::move(proc));
    }
    
    HANDLE hNamedPipe = CreateNamedPipe("\\\\.\\pipe\\emp", PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
    PIPE_UNLIMITED_INSTANCES, 0, 0, INFINITE, NULL);
    
    if (hNamedPipe == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Create named pipe failed." << std::endl
        << "The last error code: " << GetLastError() << std::endl;
        std::cout << "Press any key to continue...";
        _getche();
        CloseHandle(hFile); CloseHandle(hNamedPipe);
        return 0;
    }   
        
    for (int i = 0; i < num_clients; i++)
    {
        std::cout << "The server is waiting for connection with a client number " << i+1 << std::endl;
        if (!ConnectNamedPipe(hNamedPipe, NULL))
        {
            std::cerr << "Connection failed." << std::endl
            << "The last error code: " << GetLastError() << std::endl;
            CloseHandle(hNamedPipe);
            hNamedPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\emp"), PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 4096, 4096, 0, NULL);
            continue;
        }
        else
        {
            std::cout << "Successfully connected client number " << i+1<< " to the server!" << std::endl;
        
            ThreadInfo *param = new ThreadInfo();
            param->hFile = hFile;
            param->hPipe = hNamedPipe;
            param->clientID = i;
            
            try
            {
                auto th = std::make_unique<myLib::Thread>(handleClient, param);
                threads.push_back(std::move(th));
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }

            hNamedPipe = CreateNamedPipe(
                TEXT("\\\\.\\pipe\\emp"),
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                4096,
                4096,
                0,
                NULL
            );
        }
        
    }  

    CloseHandle(hNamedPipe);

    std::cout << "All clients connected. Waiting for them to finish work..." << std::endl;

    for (auto& th : threads)
    {
        if (th) th->join();
    }


    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    std::cout << "\n--- File ---\nID\tName\tHours\n";
    for (int i = 0; i < numRecords; ++i)
    {
        ReadFile(hFile, &emp, Employee::SERIALIZED_SIZE, &read, NULL);
        std::cout << emp << std::endl;
    }
    std::cout << "------------------------\n";

    CloseHandle(hFile);
    DeleteCriticalSection(&fileCS);
    std::cout << "Press any key to continue...";
    _getche();
    return 0;
}