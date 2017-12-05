cmake_minimum_required(VERSION 2.6)

project(bmpToMonochrome)

set(source BMP_to_monochrome.c)

add_library(bmpC STATIC bmp.c)
add_executable(bmpToMonochrome BMP_to_monochrome.c)

target_link_libraries(bmpToMonochrome bmpC)

install(TARGETS bmpC DESTINATION lib)
install(FILES bmp.h DESTINATION include)