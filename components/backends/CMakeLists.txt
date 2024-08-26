# todo: split esp32 part in seperate CMakeLists.txt file
idf_component_register(SRCS ./base-backend.cpp ./modular-backend.cpp events/default/default-events.cpp audiocd/default/default-audiocd.cpp audiocd/audiocd-stream.cpp fs/abstract-fs.cpp fs/stdiostream.cpp fs/posix/posix-fs.cpp fs/posix/posix-fs-factory.cpp plugins/posix/posix-provider.cpp saves/savefile.cpp saves/posix/posix-saves.cpp saves/default/default-saves.cpp timer/default/default-timer.cpp
./platform/esp32/osystem-audio.cpp ./platform/esp32/osystem-events.cpp ./platform/esp32/osystem-display.cpp ./platform/esp32/osystem-ui.cpp ./platform/esp32/osystem-input.cpp ./platform/esp32/osystem-sdcard.cpp ./platform/esp32/osystem.cpp ./platform/esp32/osystem_gfx.cpp
                       INCLUDE_DIRS ./..
                       REQUIRES main esp_event esp_partition esp_adc nvs_flash fatfs spiffs LovyanGFX driver # anything except for main is only reqired for esp32
                       PRIV_REQUIRES )

set (CMAKE_CXX_FLAGS "-mlongcalls")
set (CMAKE_C_FLAGS "-mlongcalls")

# only required for esp32
set (CMAKE_CXX_FLAGS "-Wno-misleading-indentation -Wno-narrowing -mlongcalls -fpermissive")
