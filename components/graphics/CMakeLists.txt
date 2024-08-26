idf_component_register(SRCS ./conversion.cpp ./cursorman.cpp ./font.cpp ./fontman.cpp ./maccursor.cpp ./managed_surface.cpp ./nine_patch.cpp ./primitives.cpp ./scaler.cpp ./screen.cpp ./sjis.cpp ./surface.cpp ./thumbnail.cpp ./transform_struct.cpp ./transform_tools.cpp ./transparent_surface.cpp ./VectorRenderer.cpp ./VectorRendererSpec.cpp ./wincursor.cpp ./yuv_to_rgb.cpp fonts/bdf.cpp fonts/consolefont.cpp fonts/macfont.cpp fonts/newfont_big.cpp fonts/newfont.cpp fonts/ttf.cpp fonts/winfont.cpp scaler/2xsai.cpp scaler/aspect.cpp scaler/downscaler.cpp scaler/hq2x.cpp scaler/hq3x.cpp scaler/scale2x.cpp scaler/scale3x.cpp scaler/scalebit.cpp scaler/thumbnail_intern.cpp
                       INCLUDE_DIRS ./..
                       REQUIRES
                       PRIV_REQUIRES )

set (CMAKE_CXX_FLAGS "-Wno-reorder -Wno-class-memaccess -Wno-unknown-pragmas -Wno-misleading-indentation -mlongcalls")
set (CMAKE_C_FLAGS "-mlongcalls")
