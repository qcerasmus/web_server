cmake_minimum_required(VERSION 3.17)

#Download asio
include(FetchContent)
FetchContent_Populate(
  asio
  GIT_REPOSITORY        https://github.com/chriskohlhoff/asio.git
  GIT_TAG   asio-1-18-0
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/third_party/asio
)

add_definitions(-DASIO_STANDALONE)

if (WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00)
endif(WIN32)

project(web_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenSSL)

include_directories(third_party)
include_directories(third_party/asio/asio/include)
include_directories(src)

add_subdirectory(performance_tester)

file(GLOB web_server_headers
    src/*.h)

add_executable(web_server main.cpp ${web_server_headers} ${handler_source})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(web_server Threads::Threads)

if (OPENSSL_FOUND)
    message("FOUND OPENSSL!")
    message("OpenSSL_VERSION: ${OPENSSL_VERSION}")
    message("OpenSSL_INCLUDE_DIR: ${OPENSSL_INCLUDE_DIR}")
    message("OpenSSL_LIBRARIES: ${OPENSSL_LIBRARIES}")
    include_directories(${OPENSSL_INCLUDE_DIR})
    target_link_libraries(web_server ${OPENSSL_LIBRARIES})
  if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/key.pem)
   message("Generating ssl keys for secure sockets")
   execute_process(COMMAND openssl req -x509 -newkey rsa:4096 -keyout ${CMAKE_CURRENT_BINARY_DIR}/key.pem -out ${CMAKE_CURRENT_BINARY_DIR}/cert.pem -sha256 -days 3650 -nodes -subj "/C=XX/ST=StateName/L=CityName/O=CompanyName/OU=CompanySectionName/CN=CommonMNameOrHostName"
     WORKING_DIRECTORY ${OPENSSL_INCLUDE_DIR}/../bin
     RESULT_VARIABLE CREATE_KEYS)
   if (NOT CREATE_KEYS EQUAL "0")
     message (FATAL_ERROR "openssl couldn't create they keys... ${CREATE_KEYS}")
    endif()
  endif()
endif (OPENSSL_FOUND)

#This is to make my life easier... Copy the compile_commands.json file into the main directory for my vim LSP server
execute_process(COMMAND cp compile_commands.json ..)

