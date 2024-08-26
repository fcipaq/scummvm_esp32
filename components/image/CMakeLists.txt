idf_component_register(SRCS ./bmp.cpp ./iff.cpp ./jpeg.cpp ./pcx.cpp ./pict.cpp ./png.cpp ./tga.cpp codecs/bmp_raw.cpp codecs/cdtoons.cpp codecs/cinepak.cpp codecs/codec.cpp codecs/msrle4.cpp codecs/msrle.cpp codecs/msvideo1.cpp codecs/qtrle.cpp codecs/rpza.cpp codecs/smc.cpp codecs/svq1.cpp codecs/truemotion1.cpp
                       INCLUDE_DIRS ./..
                       REQUIRES
                       PRIV_REQUIRES )

set (CMAKE_CXX_FLAGS "-mlongcalls")
set (CMAKE_C_FLAGS "-mlongcalls")
