# delete and copy shell commands in windows
ifeq ($(OS), Windows_NT)
RM := del /Q /F
CP := copy
ifdef ComSpec
SHELL :=$(ComSpec)
endif
ifdef COMSPEC
SHELL := $(COMSPEC)
endif
# delete and copy shell commands in posix shells
else
RM := rm -rf
CP := cp -f
endif

SRCS := src/main.cpp src/engine/util.cpp src/engine/simple_ecs.cpp src/engine/shader_program.cpp src/engine/light.cpp src/engine/window_context_manager.cpp src/engine/fly_camera.cpp src/engine/texture.cpp src/engine/mesh.cpp src/engine/material.cpp src/engine/model.cpp src/engine/shapegen.cpp src/engine/framebuffer.cpp src/engine/instance.cpp src/engine/render_stage.cpp src/engine/render_system.cpp src/engine/scene_system.cpp src/engine/input_system/input_manager.cpp src/engine/input_system/action_context.cpp

CC := g++

INCLUDE_PATHS := -ID:\MyDev\MinGW64\Include

LIBRARY_PATHS := -LD:\MyDev\MinGW64\Lib

LINKER_FLAGS := -lmingw32 -lSDL2main -lSDL2 -lOpenGL32 -lglew32 -lSDL2_image -lSDL2_image.dll -lassimp.dll

COMPILER_FLAGS_DBG := -Wall

EXTERNAL_LIBS := build\glew32.dll build\glewinfo.exe build\libassimp-5.dll build\SDL2_image.dll build\SDL2.dll

DEBUG_OPTS := -fdiagnostics-color=always -g

COMPILER_FLAGS := $(COMPILER_FLAGS_DBG) -Wl,-subsystem,windows

OBJS := build/my_opengl_demo

all : $(SRCS) $(EXTERNAL_LIBS)
	$(CC) $(SRCS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJS)

debug : $(SRCS) $(EXTERNAL_LIBS)
	$(CC) $(SRCS) $(INCLUDE_PATHS)  $(LIBRARY_PATHS) $(COMPILER_FLAGS_DBG) $(LINKER_FLAGS) $(DEBUG_OPTS) -o $(OBJS)

$(EXTERNAL_LIBS): build\\% : external_libs\\%
	$(CP) "$^" "$@"

clean :
	$(RM) build\*
