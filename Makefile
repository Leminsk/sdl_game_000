# specifies which files to compile as part of the project
# MAIN_SOURCE = main.cpp
MAIN_SOURCE = ClientServerHybrid.cpp
SOURCES = networking/*.cpp
# SOURCES = engine/*.cpp engine/ECS/*.cpp engine/ECS/Colliders/*.cpp

# specifies which compiler we're using
COMPILER = g++

# INCLUDE_PATHS specifies the additional include paths we'll need
# INCLUDE_PATHS = -IC:/msys64/mingw64/include/SDL2
NET_INCLUDE_PATHS = -IC:/msys64/mingw64/include/asio -IN:/game_dev/game_000/networking

# LIBRARY_PATHS specifies the additional library paths we'll need
LIBRARY_PATHS = -LC:/msys64/mingw64/lib

# COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
# -Wl,-subsystem,windows gets rid of the console window
# COMPILER_FLAGS = -w -Wl,-subsystem,windows
# COMPILER_FLAGS = -Wno-deprecated-declarations

# LINKER_FLAGS specifies the libraries we're linking against
NET = -lssl -lcrypto
# MIXER = -lSDL2_mixer
# IMAGE = -lSDL2_image
# TTF = -lSDL2_ttf
# MAIN_SDL = -lSDL2main -lSDL2

WIN_NET = -lws2_32 -lwsock32
MINGW = -lmingw32

LINKER_FLAGS = $(MINGW) $(MAIN_SDL) $(WIN_NET) $(MIXER) $(IMAGE) $(TTF)

#OBJ_NAME specifies the name of our executable
OBJ_NAME = main

#This is the target that compiles our executable. ALSO MAKE SURE THE FIRST CHARACTER IS A TAB AND NOT SPACES
all : $(MAIN_SOURCE)
	$(COMPILER) $(MAIN_SOURCE) $(SOURCES) $(INCLUDE_PATHS) $(NET_INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)