MAIN_SOURCE = main.cpp
SOURCES = $(wildcard engine/*.cpp) $(wildcard engine/ECS/*.cpp) $(wildcard engine/ECS/Colliders/*.cpp)

OBJECTS = $(MAIN_SOURCE:.cpp=.o) $(SOURCES:.cpp=.o)

COMPILER = g++
C_FLAGS = -std=c++17

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
    NET_INCLUDE_PATHS = -I/usr/include -Iengine/networking
	LIBRARY_PATHS = -L/usr/lib/x86_64-linux-gnu
    
    MIXER = -lSDL2_mixer
    IMAGE = -lSDL2_image
    TTF = -lSDL2_ttf
    MAIN_SDL = -lSDL2
    
    LINKER_FLAGS = $(MAIN_SDL) $(MIXER) $(IMAGE) $(TTF) -lpng -ljpeg -lfreetype -lz -lm
    C_FLAGS += -pthread

endif


ifeq ($(tree),y)
	C_FLAGS += -H
else ifeq ($(debug),y)
	C_FLAGS += -g
endif




#This is the target that compiles our executable. ALSO MAKE SURE THE FIRST CHARACTER IS A TAB AND NOT SPACES
all: main

main: $(OBJECTS)
	$(COMPILER) $(OBJECTS) $(LIBRARY_PATHS) $(LINKER_FLAGS) -o main

%.o: %.cpp 
	$(COMPILER) $(C_FLAGS) $(INCLUDE_PATHS) $(NET_INCLUDE_PATHS) -c $< -o $@

clean:
	rm -f $(OBJECTS) main
	
.PHONY: all clean