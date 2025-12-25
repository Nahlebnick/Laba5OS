#include "employee.h"

#include <iostream>
#include <iomanip> 
#include <vector>
#include <limits>  
#include <fstream>

std::istream& operator>>(std::istream& is, Employee& e)
{
    std::cout << "Enter name (max 9 chars): ";
    is.get(e.name, 10); 
    if (is.fail() && !is.eof()) is.clear(); 
    is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Enter hours: ";
    while (!(is >> e.hours) || e.hours < 0) {
        std::cout << "Invalid hours. Enter a positive number: ";
        is.clear();
        is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    return is;
}

std::ostream& operator<<(std::ostream& out, const Employee& e)
{
    out << "Id: " << e.num << '\t';
    out << "Name: " << e.name << '\t';
    out << "Hours: " << e.hours << '\t';
    return out;
}

/*void Employee::serialize(std::ofstream& out) const noexcept
{
    std::vector<char> buffer(SERIALIZED_SIZE);
    int i = 0;
    memcpy(buffer.data(), this, SERIALIZED_SIZE);
    out.write(buffer.data(), buffer.size());
}*/