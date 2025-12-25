#include <iostream>
#include <cstring>
#include <conio.h>

#pragma pack(push, 1)
struct Employee
{
    int num;
    char name[10];
    double hours;
    static constexpr size_t  SERIALIZED_SIZE = sizeof(int) + sizeof(double) + 10;

    Employee() = default;

    friend std::istream& operator>>(std::istream& is, Employee& e);
    friend std::ostream& operator<<(std::ostream& out, const Employee& e);
    
};
#pragma pack(pop) 