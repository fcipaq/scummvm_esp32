idf_component_register(SRCS ./commandLine.cpp ./main.cpp ./plugins.cpp ./version.cpp
                       INCLUDE_DIRS ./..
                       REQUIRES
                       PRIV_REQUIRES )

set (CMAKE_CXX_FLAGS "-mlongcalls")
set (CMAKE_C_FLAGS "-mlongcalls")
