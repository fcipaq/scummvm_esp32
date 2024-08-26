idf_component_register(SRCS ./advancedDetector.cpp ./engine.cpp ./game.cpp ./obsolete.cpp ./savestate.cpp
                       INCLUDE_DIRS ./..
                       REQUIRES
                       PRIV_REQUIRES )
                       
set (CMAKE_CXX_FLAGS "-mlongcalls")
set (CMAKE_C_FLAGS "-mlongcalls")                       

