idf_component_register(SRCS ./about.cpp ./Actions.cpp ./browser.cpp ./chooser.cpp ./debugger.cpp ./dialog.cpp ./editgamedialog.cpp ./error.cpp ./EventRecorder.cpp ./fluidsynth-dialog.cpp ./gui-manager.cpp ./Key.cpp ./massadd.cpp ./message.cpp ./object.cpp ./options.cpp ./predictivedialog.cpp ./themebrowser.cpp ./ThemeEngine.cpp ./ThemeEval.cpp ./ThemeLayout.cpp ./ThemeParser.cpp ./Tooltip.cpp ./unknown-game-dialog.cpp ./widget.cpp
                       INCLUDE_DIRS ./..
                       REQUIRES
                       PRIV_REQUIRES )
                       
set (CMAKE_CXX_FLAGS "-mlongcalls")
set (CMAKE_C_FLAGS "-mlongcalls")

