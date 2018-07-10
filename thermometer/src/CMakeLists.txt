project(thermometer)
cmake_minimum_required(VERSION 2.8.11)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

message("LIBS: ${CONAN_LIBS}")

add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})
