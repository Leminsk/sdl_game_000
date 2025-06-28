# specifies which files to compile as part of the project
MAIN_SOURCE = main.cpp
SOURCES = engine/*.cpp engine/ECS/*.cpp engine/ECS/Colliders/*.cpp

# specifies which compiler we're using
COMPILER = g++

ifeq ($(OS),Windows_NT) # For Windows
    # INCLUDE_PATHS specifies the additional include paths we'll need
    INCLUDE_PATHS = -IC:/msys64/mingw64/include/SDL2
    NET_INCLUDE_PATHS = -IC:/msys64/mingw64/include/asio -Iengine/networking
    
    # LIBRARY_PATHS specifies the additional library paths we'll need
    LIBRARY_PATHS = -LC:/msys64/mingw64/lib
    
    # COMPILER_FLAGS specifies the additional compilation options we're using
    # -w suppresses all warnings
    # -Wl,-subsystem,windows gets rid of the console window
    # COMPILER_FLAGS = -w -Wl,-subsystem,windows
    # COMPILER_FLAGS = -Wno-deprecated-declarations
    
    # LINKER_FLAGS specifies the libraries we're linking against
    # NET = -lssl -lcrypto # I don't think I'll need this for Linux
    MIXER = -lSDL2_mixer
    IMAGE = -lSDL2_image
    TTF = -lSDL2_ttf
    MAIN_SDL = -lSDL2main -lSDL2
    
    WIN_NET = -lws2_32 -lwsock32
    MINGW = -lmingw32
    
    LINKER_FLAGS = $(MINGW) $(MAIN_SDL) $(WIN_NET) $(MIXER) $(IMAGE) $(TTF)

else # For Linux
    INCLUDE_PATHS = -I/usr/include/SDL2
    NET_INCLUDE_PATHS = -I/usr/include/asio -Iengine/networking
    
    MIXER = -lSDL2_mixer
    IMAGE = -lSDL2_image
    TTF = -lSDL2_ttf
    MAIN_SDL = -lSDL2
    
    LINKER_FLAGS = $(MAIN_SDL) $(MIXER) $(IMAGE) $(TTF)
    COMPILER_FLAGS = -pthread

endif




# OBJ_NAME specifies the name of our executable
OBJ_NAME = main

#This is the target that compiles our executable. ALSO MAKE SURE THE FIRST CHARACTER IS A TAB AND NOT SPACES
all : $(MAIN_SOURCE)
	$(COMPILER) $(MAIN_SOURCE) $(SOURCES) $(INCLUDE_PATHS) $(NET_INCLUDE_PATHS) $(LIBRARY_PATHS) $(LINKER_FLAGS) $(COMPILER_FLAGS) -o $(OBJ_NAME)