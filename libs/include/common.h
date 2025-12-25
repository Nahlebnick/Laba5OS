#ifndef COMMON_H
#define COMMON_H

#include <windows.h>
#include <string>
#include <iostream>
#include "employee.h"

#pragma pack(push, 1)

enum Operation {
    OP_READ = 1,
    OP_WRITE = 2,
    OP_EXIT = 3
};

struct Request
 {
    Operation operation;    
    int employeeID;         
};

struct Response
 {
    bool success;           
    Employee employee;      
};

#pragma pack(pop)
#endif 