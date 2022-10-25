project(fsh-lib)

set(SOURCES parser.c fsh.c)
set(HEADERS parser.h fsh.h)

add_library(lib ${SOURCES} ${HEADERS})
target_include_directories(lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
