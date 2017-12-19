@echo off
if "%1_" == "debug_" (
	cmake -G "MinGW Makefiles" -DDEBUG=1 CMakeLists.txt
	echo Project is being compiled in debug mode
) else (
	if "%1_" == "help" (
		echo Help:
		echo 	%0 help - this help
		echo 	%0 debug - compile project with debug option
	) else (
		cmake -G "MinGW Makefiles" -DDEBUG=0 CMakeLists.txt
	)
)
mingw32-make