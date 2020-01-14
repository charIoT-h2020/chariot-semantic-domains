cmake_minimum_required(VERSION 3.0)

project(domsec)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(libutils)
add_subdirectory(libalgorithms)
add_subdirectory(libapplications)

include_directories(utils)
include_directories(algorithms)
include_directories(applications)
set(SOURCES
   tests/TestDomain/Token.cpp
   tests/TestDomain/Lexer.cpp
   tests/TestDomain/Parser.cpp
   tests/TestDomain.cpp
   )
    
set(HEADERS
   tests/TestDomain/Token.h
   tests/TestDomain/Lexer.h
   tests/TestDomain/Parser.h
   tests/TestDomain.h
   )

add_executable(domsec ${SOURCES} ${HEADERS})
target_link_libraries(domsec utils numerics ScalarInterface)

enable_testing()
add_subdirectory(tests)
add_test(NAME TestDomain COMMAND domsec ${CMAKE_SOURCE_DIR}/tests/file_general.txt)

install(TARGETS domsec
  RUNTIME DESTINATION bin)
