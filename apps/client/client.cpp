#define NOMINMAX
#include <iostream>
#include <Windows.h>
#include "common.h"


bool sendRequest(Operation operation, int employeeID, HANDLE hPipe)
 {
    Request request;
    request.operation = operation;
    request.employeeID = employeeID;

    DWORD bytesWritten;
    if (!WriteFile(hPipe, &request, sizeof(Request), &bytesWritten, NULL))
    {
        std::cerr << "Failed to send request." << std::endl
        << "The last error code: " << GetLastError() << std::endl;
        std::cout << "Press any key to continue...";
        _getche();
        return false;
    }

    return true;
}
Response receiveResponse(HANDLE hPipe)
{
    Response response;
    DWORD bytesRead;
        
    if (!ReadFile(hPipe, &response, sizeof(Response), &bytesRead, NULL))
    {
        response.success = false;
    }

    return response;
}

void performReadOperation(int employeeID, HANDLE hPipe)
{
    if(!sendRequest(OP_READ, employeeID, hPipe))
    {
        return;
    }

    Response response = receiveResponse(hPipe);
        
    if (response.success)
    {
        std::cout << "\nEmployee record:" << std::endl;
        std::cout << response.employee << std::endl;
    }
    else
    {
        std::cout << "Failed to receive response" << std::endl;
        return;
    }
}

void performWriteOperation(int employeeID, HANDLE hPipe) {
        
        if (!sendRequest(OP_WRITE, employeeID, hPipe))
        {
            return;
        }

        Response response = receiveResponse(hPipe);
        
        if (!response.success)
        {
            std::cout << "Failed to receive response" << std::endl;
            return;
        }

        std::cout << "\nCurrent record:" << std::endl;
        std::cout << response.employee << std::endl;

        Employee modifiedEmployee = response.employee;
        
        std::cout << "\nEnter new data:" << std::endl;
        
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin >> modifiedEmployee;

        DWORD bytesWritten;
        if (!WriteFile(hPipe, &modifiedEmployee, sizeof(Employee), &bytesWritten, NULL))
        {
            std::cerr << "Failed to send modified data" << std::endl
            << "The last error code: " << GetLastError() << std::endl;
            std::cout << "Press any key to continue...";
            _getche();
            return;
        }

        response = receiveResponse(hPipe);
        
        if (response.success)
        {
            std::cout << "\nRecord updated successfully:" << std::endl;
            std::cout << response.employee << std::endl;
        }
        else
        {
            std::cout << "Failed to receive response" << std::endl;
            return;
        }
    }

int main(int argc, char* argv[])
{
    if (argc < 1) return 1;

    HANDLE hPipe = CreateFile("\\\\.\\pipe\\emp", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (hPipe == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Connection with named pipe failed." << std::endl
        << "The last error code: " << GetLastError() << std::endl;
        std::cout << "Press any key to continue...";
        _getche();
        return 0;
    }

    bool running = true;
    while(running)
    {
        std::cout << "\n==== Operation Menu ====" << std::endl;
        std::cout << "1. Read employee record" << std::endl;
        std::cout << "2. Modify employee record" << std::endl;
        std::cout << "3. Exit" << std::endl;
        std::cout << "Choose operation: ";

        int i;
        std:: cin >> i;
        Operation op;
        switch(i)
        {
            case 1: op = OP_READ; break;
            case 2: op = OP_WRITE; break;
            case 3: op = OP_EXIT; break;
            default: std::cerr << "Incorrect choice." << std::endl; continue;
        }

        if (op == OP_EXIT)
         {
            sendRequest(op, 0, hPipe);
            running = false;
            continue;
        }

        int employeeID;
        std::cout << "Enter employee ID: ";
        std::cin >> employeeID;

        if (op == OP_READ)
        {
            performReadOperation(employeeID, hPipe);
        }

        else if (op == OP_WRITE)
        {
            performWriteOperation(employeeID, hPipe);
        }       
    }

    CloseHandle(hPipe);
    return 0;
}