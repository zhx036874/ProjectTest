cmake_minimum_required(VERSION 3.10)
project(ThreadPoolDemo)

# Set C++14 standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt5 package
set(CMAKE_PREFIX_PATH "C:/Qt/Qt5.11.3/5.11.3/msvc2017_64")
find_package(Qt5 COMPONENTS Core REQUIRED)

# Add source files
set(SOURCES
    src/main.cpp
    src/threadpool.cpp
    src/task.cpp
)

# Add header files
set(HEADERS
    include/threadpool.h
    include/task.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)