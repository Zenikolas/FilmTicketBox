cmake_minimum_required(VERSION 3.17)
project(filmTicketBox)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenSSL REQUIRED)
find_package(Poco COMPONENTS Net JSON Util REQUIRED)
IF(Poco_FOUND)
    MESSAGE("Poco found!")
ENDIF()

include_directories(Poco_INCLUDE_DIRS)
add_executable(filmTicketBox main.cpp cinema.cpp)
target_link_libraries(filmTicketBox Poco::Net Poco::JSON Poco::Util)