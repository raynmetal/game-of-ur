# delete and copy shell commands in windows
ifeq ($(OS), Windows_NT)
RM := rd /s /q
CP := copy
MKDIR := mkdir
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
MKDIR := mkdir -p
endif


CC := g++


INCLUDE_PATHS := D:\MyDev\MinGW64\Include
LIBRARY_PATHS := D:\MyDev\MinGW64\Lib
DYNAMIC_LIBS := mingw32 SDL2main SDL2 OpenGL32 glew32 SDL2_image SDL2_image.dll assimp.dll SDL2_ttf SDL2_ttf.dll

DEBUG_OPTS := -fdiagnostics-color=always -g
COMPILER_FLAGS_DBG := -Wall -DZO_DUMB_DELAY
COMPILER_FLAGS := $(COMPILER_FLAGS_DBG) -Wl,-subsystem,windows 

TARGET_LIB_DIR := build/lib
TARGET_INCLUDE_DIR := build/include
TARGET_BIN_DIR := build/bin

OBJ_DIR := build/temp

SRC_DIR := src
ENGINE_SRC_DIR := engine
ENGINE_TARGET_BASE := engine
APP_TARGET := application

#====================================
####   READ-ONLY, DO NOT MODIFY  ####
#====================================

APP_EXECUTABLE := $(TARGET_BIN_DIR)/$(APP_TARGET).exe
ENGINE_SHARED_LIB_TARGETS := $(TARGET_BIN_DIR)/$(ENGINE_TARGET_BASE).dll $(TARGET_LIB_DIR)/lib$(ENGINE_TARGET_BASE).dll.a
ENGINE_STATIC_LIB_TARGETS := $(TARGET_LIB_DIR)/lib$(ENGINE_TARGET_BASE).a

ENGINE_TARGET := $(ENGINE_STATIC_LIB_TARGETS)
ENGINE_LIB := $(patsubst lib%.a,%,$(notdir $(filter %.a,$(ENGINE_TARGET))))
ENGINE_HEADERS := \
	$(patsubst $(SRC_DIR)/$(ENGINE_SRC_DIR)/%,$(TARGET_INCLUDE_DIR)/$(ENGINE_SRC_DIR)/%, \
		$(wildcard $(SRC_DIR)/$(ENGINE_SRC_DIR)/*.hpp $(SRC_DIR)/$(ENGINE_SRC_DIR)/**/*.hpp) \
	)
ENGINE_SOURCES := $(wildcard $(SRC_DIR)/$(ENGINE_SRC_DIR)/*.cpp $(SRC_DIR)/$(ENGINE_SRC_DIR)/**/*.cpp)
ENGINE_OBJS := $(patsubst $(SRC_DIR)/$(ENGINE_SRC_DIR)/%.cpp,$(OBJ_DIR)/$(ENGINE_SRC_DIR)/%.o,$(ENGINE_SOURCES))

APP_HEADERS := $(filter-out $(SRC_DIR)/$(ENGINE_SRC_DIR)/%, $(wildcard $(SRC_DIR)/*.hpp $(SRC_DIR)/**/*.hpp))
APP_SOURCES := $(filter-out $(SRC_DIR)/$(ENGINE_SRC_DIR)/%, $(wildcard $(SRC_DIR)/*.cpp $(SRC_DIR)/**/*.cpp))
APP_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(APP_SOURCES))

REQUIRED_SUBDIRS := $(dir $(ENGINE_HEADERS) $(ENGINE_OBJS) $(APP_OBJS))

.PHONY: all clean

all: $(APP_EXECUTABLE)

$(APP_EXECUTABLE): $(ENGINE_TARGET) $(APP_OBJS)
	$(CC) $(APP_OBJS) \
		-o $(APP_EXECUTABLE) \
		$(addprefix -I,$(INCLUDE_PATHS) $(TARGET_INCLUDE_DIR)) \
		$(addprefix -L,$(LIBRARY_PATHS) $(TARGET_LIB_DIR)) \
		$(COMPILER_FLAGS_DBG) \
		$(DEBUG_OPTS) \
		$(addprefix -l,$(ENGINE_LIB) $(DYNAMIC_LIBS))

$(ENGINE_TARGET): $(ENGINE_OBJS)
	ar rcs $(filter %.a,$@) $^

.SECONDEXPANSION:

$(APP_OBJS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp $(APP_HEADERS) $(ENGINE_HEADERS) | $$(dir $$@)
	$(CC) -c $< -o $@ \
		$(addprefix -I,$(INCLUDE_PATHS) $(TARGET_INCLUDE_DIR)) \
		$(COMPILER_FLAGS_DBG) \
		$(DEBUG_OPTS) \

$(ENGINE_OBJS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp $(ENGINE_HEADERS) | $$(dir $$@)
	$(CC) -r -c $< -o $@ \
		$(addprefix -I,$(INCLUDE_PATHS)) \
		$(COMPILER_FLAGS_DBG) \
		$(DEBUG_OPTS) \

$(TARGET_INCLUDE_DIR)/%.hpp: $(SRC_DIR)/%.hpp | $$(dir $$@)
	$(CP) "$(subst /,\,$(patsubst $(TARGET_INCLUDE_DIR)/%.hpp,$(SRC_DIR)/%.hpp,$@))" "$(dir $(subst /,\,$@))"

$(sort $(REQUIRED_SUBDIRS)):
ifeq ($(OS), Windows_NT)
	$(MKDIR) $(subst /,\,$@)
else
	$(MKDIR) $@
endif

_TO_CLEAN := $(TARGET_BIN_DIR)/* $(TARGET_LIB_DIR)/* $(TARGET_INCLUDE_DIR)/* $(OBJ_DIR)/*

clean:
ifeq ($(OS), Windows_NT)
	del /q $(subst /,\,$(_TO_CLEAN))
	for /D %%f in ($(subst /,\,$(_TO_CLEAN))) do $(RM) "%%f"
else
	$(RM) $(_TO_CLEAN)
endif
