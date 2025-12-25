add_library(MyLib STATIC)

target_sources(MyLib
    PRIVATE 
    src/employee.cpp

    PUBLIC 
    FILE_SET HEADERS 
    BASE_DIRS include 
    FILES 
        include/employee.h
)