idf_component_register(SRCS ./main.cpp
                       INCLUDE_DIRS ./.. ./../components/engines
                       REQUIRES
                       PRIV_REQUIRES )

target_compile_options(${COMPONENT_LIB} PRIVATE -DCOMPILEDATE="$(COMPILEDATE)" -DGITREV="$(GITREV)")

set (CMAKE_CXX_FLAGS "-mlongcalls")
set (CMAKE_C_FLAGS "-mlongcalls")
