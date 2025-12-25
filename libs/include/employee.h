#include <iostream>
#include <cstring>
#include <conio.h>

struct Employee
{
    int num;
    char name[10];
    double hours;
    static constexpr size_t  SERIALIZED_SIZE = sizeof(int) + sizeof(double) + 10;


    //Employee(int num_) : num(num_) {}
    /*Employee() noexcept;
    Employee(int num_, const char* name_, double hours_) : num(num_), hours(hours_)
    {
        std::strcpy(name, name_);
    }

    Employee(const Employee& _other) noexcept;
    Employee(Employee&& _other) noexcept;
    ~Employee() noexcept = default;*/

    friend std::istream& operator>>(std::istream& is, Employee& e);
    friend std::ostream& operator<<(std::ostream& out, const Employee& e);


    //void serialize(std::ofstream& out) const noexcept;
    
};