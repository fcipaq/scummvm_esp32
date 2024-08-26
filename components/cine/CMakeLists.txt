idf_component_register(SRCS ./anim.cpp ./bg.cpp ./bg_list.cpp ./cine.cpp ./console.cpp ./detection.cpp ./gfx.cpp ./main_loop.cpp ./msg.cpp ./object.cpp ./pal.cpp ./part.cpp ./prc.cpp ./rel.cpp ./saveload.cpp ./script_fw.cpp ./script_os.cpp ./sound.cpp ./texte.cpp ./unpack.cpp ./various.cpp
                       INCLUDE_DIRS ./..
                       REQUIRES
                       PRIV_REQUIRES )

set (CMAKE_CXX_FLAGS "-Wno-reorder -Wno-class-memaccess -mlongcalls")
set (CMAKE_C_FLAGS "-mlongcalls")
