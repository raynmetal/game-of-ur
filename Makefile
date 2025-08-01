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

########################
# DO NOT EDIT
COMPILER_FLAGS_DEBUG := -Wall -DZO_DUMB_DELAY
COMPILER_FLAGS_RELEASE := $(COMPILER_FLAGS_DEBUG) -Wl,-subsystem,windows 
########################
COMPILER_FLAGS := $(COMPILER_FLAGS_RELEASE)

EXTERNAL_LIB_DIR := external_libs
PROJECT_DATA_DIR := data
USER_DIR := user_data 

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

# Per the stack overflow answer here: https://stackoverflow.com/a/18258352
# - `parameter 2` in foreach is a list of files and dirs present in the current dir (or one 
# specified by the user), provided as the `argument 1` of rwildcard or in later iterations
# one of its subdirectories
# - `parameter 3` calls rwildcard on each file and directory on the list.  The second part
# of `parameter 3` essentially just prints the current item, so long as it matches
# the pattern provided in `argument 2` of rwildcard
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d)) 

APP_EXECUTABLE := $(TARGET_BIN_DIR)/$(APP_TARGET).exe
ENGINE_SHARED_LIB_TARGETS := $(TARGET_BIN_DIR)/$(ENGINE_TARGET_BASE).dll $(TARGET_LIB_DIR)/lib$(ENGINE_TARGET_BASE).dll.a
ENGINE_STATIC_LIB_TARGETS := $(TARGET_LIB_DIR)/lib$(ENGINE_TARGET_BASE).a

ENGINE_TARGET := $(ENGINE_STATIC_LIB_TARGETS)
ENGINE_LIB := $(patsubst lib%.a,%,$(notdir $(filter %.a,$(ENGINE_TARGET))))
ENGINE_HEADERS := \
	$(patsubst $(SRC_DIR)/$(ENGINE_SRC_DIR)/%,$(TARGET_INCLUDE_DIR)/$(ENGINE_SRC_DIR)/%, \
		$(call rwildcard,$(SRC_DIR)/$(ENGINE_SRC_DIR),*.hpp) \
	)
ENGINE_SOURCES := $(call rwildcard,$(SRC_DIR)/$(ENGINE_SRC_DIR),*.cpp)
ENGINE_OBJS := $(patsubst $(SRC_DIR)/$(ENGINE_SRC_DIR)/%.cpp,$(OBJ_DIR)/$(ENGINE_SRC_DIR)/%.o,$(ENGINE_SOURCES))

APP_HEADERS := $(filter-out $(ENGINE_HEADERS), $(call rwildcard,$(SRC_DIR),*.hpp))
APP_SOURCES := $(filter-out $(ENGINE_SOURCES), $(call rwildcard,$(SRC_DIR),*.cpp))
APP_OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(APP_SOURCES))

EXTERNAL_LIB_SOURCES := $(call rwildcard,$(EXTERNAL_LIB_DIR),*)
EXTERNAL_LIB_TARGETS := $(patsubst $(EXTERNAL_LIB_DIR)/%,$(TARGET_BIN_DIR)/%,$(EXTERNAL_LIB_SOURCES))

PROJECT_DATA_SOURCES := $(call rwildcard,$(PROJECT_DATA_DIR),*)
PROJECT_DATA_TARGETS := $(patsubst $(PROJECT_DATA_DIR)/%,$(TARGET_BIN_DIR)/$(PROJECT_DATA_DIR)/%,$(PROJECT_DATA_SOURCES))

USER_TARGET_DIR := $(TARGET_BIN_DIR)/$(USER_DIR)

REQUIRED_SUBDIRS := $(sort $(dir $(ENGINE_HEADERS) $(ENGINE_OBJS) $(APP_OBJS) $(EXTERNAL_LIB_TARGETS) $(PROJECT_DATA_TARGETS)) $(USER_TARGET_DIR)/)

.PHONY: all clean

all: $(APP_EXECUTABLE) $(EXTERNAL_LIB_TARGETS) $(PROJECT_DATA_TARGETS) | $(USER_TARGET_DIR)

$(APP_EXECUTABLE): $(ENGINE_TARGET) $(APP_OBJS)
	$(CC) $(APP_OBJS) \
		-o $(APP_EXECUTABLE) \
		$(addprefix -I,$(INCLUDE_PATHS) $(TARGET_INCLUDE_DIR)) \
		$(addprefix -L,$(LIBRARY_PATHS) $(TARGET_LIB_DIR)) \
		$(COMPILER_FLAGS) \
		$(DEBUG_OPTS) \
		$(addprefix -l,$(ENGINE_LIB) $(DYNAMIC_LIBS))

$(ENGINE_TARGET): $(ENGINE_OBJS)
	ar rcs $(filter %.a,$@) $^

.SECONDEXPANSION:

$(APP_OBJS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp $(APP_HEADERS) $(ENGINE_HEADERS) | $$(dir $$@)
	$(CC) -c $< -o $@ \
		$(addprefix -I,$(INCLUDE_PATHS) $(TARGET_INCLUDE_DIR)) \
		$(COMPILER_FLAGS) \
		$(DEBUG_OPTS) \

$(ENGINE_OBJS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp $(ENGINE_HEADERS) | $$(dir $$@)
	$(CC) -r -c $< -o $@ \
		$(addprefix -I,$(INCLUDE_PATHS)) \
		$(COMPILER_FLAGS) \
		$(DEBUG_OPTS) \

$(EXTERNAL_LIB_TARGETS): $(TARGET_BIN_DIR)/% : $(EXTERNAL_LIB_DIR)/% | $$(dir $$@)
ifeq ($(OS), Windows_NT)
	$(CP) "$(subst /,\,$^)" "$(subst /,\,$@)"
else
	$(CP) "$^" "$@"
endif

$(PROJECT_DATA_TARGETS): $(TARGET_BIN_DIR)/$(PROJECT_DATA_DIR)/% : $(PROJECT_DATA_DIR)/% | $$(dir $$@)
ifeq ($(OS), Windows_NT)
	$(CP) "$(subst /,\,$^)" "$(subst /,\,$@)"
else
	$(CP) "$^" "$@"
endif

$(TARGET_INCLUDE_DIR)/%.hpp: $(SRC_DIR)/%.hpp | $$(dir $$@)
	$(CP) "$(subst /,\,$(patsubst $(TARGET_INCLUDE_DIR)/%.hpp,$(SRC_DIR)/%.hpp,$@))" "$(dir $(subst /,\,$@))"

$(REQUIRED_SUBDIRS):
ifeq ($(OS), Windows_NT)
# see answer: https://superuser.com/questions/541534/check-whether-a-file-folder-exists-with-cmd-command-line-not-batch-script
	IF NOT EXIST $(subst /,\,$@)NUL $(MKDIR) $(subst /,\,$@)
else
	$(MKDIR) -p $@
endif

_TO_CLEAN := $(TARGET_BIN_DIR)/* $(TARGET_LIB_DIR)/* $(TARGET_INCLUDE_DIR)/* $(OBJ_DIR)/*

clean:
ifeq ($(OS), Windows_NT)
	del /q $(subst /,\,$(_TO_CLEAN))
	for /D %%f in ($(subst /,\,$(_TO_CLEAN))) do $(RM) "%%f"
else
	$(RM) $(_TO_CLEAN)
endif
