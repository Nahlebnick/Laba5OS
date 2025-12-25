#define NOMINMAX
#include <Windows.h>
#include <iostream>
#include <string>
#include <limits> 
#include <vector>
#include <fstream>
#include "process.h"
#include "common.h"

int main()
{
    setlocale(LC_ALL, "Russian");
    std::string fileName;
    int numRecords;
    std::cout << "Enter file name: "; std::cin >> fileName;
    std::cout << "Enter number of records: "; std::cin >> numRecords;

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

    auto proc = std::make_unique<myLib::Process>(L"client.exe", false, CREATE_NEW_CONSOLE);

    HANDLE hNamedPipe = CreateNamedPipe("\\\\.\\pipe\\emp", PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1, 0, 0, INFINITE, NULL);
    
    if (hNamedPipe == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Create named pipe failed." << std::endl
        << "The last error code: " << GetLastError() << std::endl;
        std::cout << "Press any key to continue...";
        _getche();
        CloseHandle(hFile); CloseHandle(hNamedPipe);
        return 0;
    }
    std::cout << "The server is waiting for connection with a client." << std::endl;
    if (!ConnectNamedPipe(hNamedPipe, NULL))
    {
        std::cerr << "Connection failed." << std::endl
        << "The last error code: " << GetLastError() << std::endl;
        std::cout << "Press any key to continue...";
        _getche();
        return 0;
    }

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
            CloseHandle(hFile); CloseHandle(hNamedPipe);
            return 0;
        }
                    
        if (req.operation == OP_EXIT)
        {
            std::cout << "Client requested exit." << std::endl;
            res.success = true;
            WriteFile(hNamedPipe, &res, sizeof(Response), &bytesWritten, NULL);
            break;
        }

        if (req.operation == OP_READ)
        {
            Employee emp;
            LARGE_INTEGER offset;
            offset.QuadPart = req.employeeID * sizeof(Employee);
            SetFilePointerEx(hFile, offset, NULL, FILE_BEGIN);
            if (!ReadFile(hFile, &emp, sizeof(Employee), &dwBytesRead, NULL))
            {
                std::cerr << "Read file failed." << std::endl
                    << "The last error code: " << GetLastError() << std::endl;
                    std::cout << "Press any key to continue...";
                    _getche();
                CloseHandle(hFile); CloseHandle(hNamedPipe);
                return 0;
            }
            res.success = true;
            res.employee = emp;

            if (!WriteFile(hNamedPipe, &res, sizeof(Response), &bytesWritten, NULL))
            {
                std::cerr << "Failed to write to pipe" << std::endl
                << "The last error code: " << GetLastError() << std::endl;
                std::cout << "Press any key to continue...";
                _getche();
                break;
            }
        }

        else if (req.operation == OP_WRITE)
        {
            Employee emp;
            LARGE_INTEGER offset;
            offset.QuadPart = req.employeeID * sizeof(Employee);
            SetFilePointerEx(hFile, offset, NULL, FILE_BEGIN);
            if (!ReadFile(hFile, &emp, sizeof(Employee), &dwBytesRead, NULL))
            {
                std::cerr << "Read file failed." << std::endl
                    << "The last error code: " << GetLastError() << std::endl;
                    std::cout << "Press any key to continue...";
                    _getche();
                CloseHandle(hFile); CloseHandle(hNamedPipe);
                return 0;
            }
            res.success = true;
            res.employee = emp;

            if (!WriteFile(hNamedPipe, &res, sizeof(Response), &bytesWritten, NULL))
            {
                std::cerr << "Failed to write to pipe" << std::endl
                << "The last error code: " << GetLastError() << std::endl;
                std::cout << "Press any key to continue...";
                _getche();
                break;
            }

            if (!ReadFile(hNamedPipe, &emp, sizeof(Employee), &dwBytesRead, NULL))
            {
                std::cerr << "Read pipe failed." << std::endl
                    << "The last error code: " << GetLastError() << std::endl;
                    std::cout << "Press any key to continue...";
                    _getche();
                CloseHandle(hFile); CloseHandle(hNamedPipe);
                return 0;
            }
            
            LARGE_INTEGER moveBack;
            moveBack.QuadPart = -(LONGLONG)sizeof(Employee);
            SetFilePointerEx(hFile, moveBack, NULL, FILE_CURRENT); 

            if (!WriteFile(hFile, &emp, sizeof(Employee), &bytesWritten, NULL))
            {
                std::cerr << "Read file failed." << std::endl
                    << "The last error code: " << GetLastError() << std::endl;
                    std::cout << "Press any key to continue...";
                    _getche();
                CloseHandle(hFile); CloseHandle(hNamedPipe);
                return 0;
            }

            res.success = true; 
            res.employee = emp;
            if (!WriteFile(hNamedPipe, &res, sizeof(Response), &bytesWritten, NULL))
            {
                std::cerr << "Read file failed." << std::endl
                    << "The last error code: " << GetLastError() << std::endl;
                    std::cout << "Press any key to continue...";
                    _getche();
                CloseHandle(hFile); CloseHandle(hNamedPipe);
                return 0;
            }
        }        
    }

    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
    std::cout << "\n--- File ---\nID\tName\tHours\n";
    for (int i = 0; i < numRecords; ++i)
    {
        ReadFile(hFile, &emp, Employee::SERIALIZED_SIZE, &read, NULL);
        std::cout << emp << std::endl;
    }
    std::cout << "------------------------\n";

    CloseHandle(hFile); CloseHandle(hNamedPipe);

    std::cout << "Press any key to continue...";
    _getche();
    return 0;
}