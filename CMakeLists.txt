cmake_minimum_required(VERSION 2.6)

project(bmpToMonochrome)

set(source BMP_to_monochrome.c)

add_executable(bmpToMonochrome BMP_to_monochrome.c bmp.c)

SET(DEBUG False CACHE BOOLEAN "1 if debug-mode is activated, 0 if not")

if (DEBUG)
	add_definitions(-DDEBUG -DECHO_ON -Wall -Wextra)
else(DEBUG)
	if (ECHO_ON)
		add_definitions(-DECHO_ON)
	endif(ECHO_ON)
endif(DEBUG)

#install(TARGETS bmpC DESTINATION lib)
install(FILES bmp.h DESTINATION include)