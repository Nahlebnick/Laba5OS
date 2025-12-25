add_executable(Server)
target_sources(Server PRIVATE server/server.cpp)

add_executable(Client)
target_sources(Client PRIVATE client/client.cpp)

target_link_libraries(Server PRIVATE MyLib)
target_link_libraries(Client PRIVATE MyLib)