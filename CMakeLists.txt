cmake_minimum_required(VERSION 2.6)

project(bmpToMonochrome)

set(source BMP_to_monochrome.c)

#add_library(bmpC STATIC bmp.c)
add_executable(bmpToMonochrome BMP_to_monochrome.c bmp.c)

#target_link_libraries(bmpToMonochrome bmpC)
if (DEBUG)
	add_definitions(-DDEBUG -DECHO_ON -Wall -Wextra)
else(DEBUG)
	if (ECHO_ON)
		add_definitions(-DECHO_ON)
	endif(ECHO_ON)
endif(DEBUG)

#install(TARGETS bmpC DESTINATION lib)
install(FILES bmp.h DESTINATION include)