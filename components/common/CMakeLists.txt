idf_component_register(SRCS ./debug.cpp ./archive.cpp ./config-manager.cpp ./coroutines.cpp ./cosinetables.cpp ./dcl.cpp ./encoding.cpp ./error.cpp ./EventDispatcher.cpp ./EventMapper.cpp ./file.cpp ./fs.cpp ./gui_options.cpp ./hashmap.cpp ./iff_container.cpp ./ini-file.cpp ./installshield_cab.cpp ./json.cpp ./language.cpp ./localization.cpp ./macresman.cpp ./md5.cpp ./memorypool.cpp ./mutex.cpp ./osd_message_queue.cpp ./platform.cpp ./quicktime.cpp ./random.cpp ./rational.cpp ./rendermode.cpp ./sinetables.cpp ./str.cpp ./stream.cpp ./system.cpp ./textconsole.cpp ./text-to-speech.cpp ./tokenizer.cpp ./translation.cpp ./unarj.cpp ./unzip.cpp ./ustr.cpp ./util.cpp ./winexe.cpp ./winexe_ne.cpp ./winexe_pe.cpp ./xmlparser.cpp ./zlib.cpp
                       INCLUDE_DIRS ./..
                       REQUIRES
                       PRIV_REQUIRES )


set (CMAKE_CXX_FLAGS "-Wno-reorder -Wno-class-memaccess -Wno-unknown-pragmas -mlongcalls")
set (CMAKE_C_FLAGS "-mlongcalls")
