cmake_minimum_required(VERSION 3.0.0)

project(fakng-shell)

set(MAIN_PROJECT_SRC_LIST main)
add_compile_options()
add_executable(main ${MAIN_PROJECT_SRC_LIST})

add_subdirectory(lib)
target_link_libraries(main lib)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
