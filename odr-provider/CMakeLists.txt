set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(odr-provider-lib
	"c.cpp"
	"c.h")
add_executable(odr-provider
	"main.cpp"
	"a.cpp"
	"b.cpp"
	"a.h"
	"b.h"
	"common.h"
)
