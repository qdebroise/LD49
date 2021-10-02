#------------------------------------------------------------------------------
# SDL2 (version 2.0.16)
#------------------------------------------------------------------------------
INCLUDE_DIRS += third-party/sdl2/include/

ifeq ($(TARGET_PLATFORM),linux)
    # From `sdl2-config --static-libs`
    LIBS += -lSDL2main -lSDL2 -lm -ldl -lpthread -lrt
endif

ifeq ($(TARGET_PLATFORM),windows)
    # Warning: order is important!
    LIBS += -lmingw32 -lSDL2main -lSDL2 -mwindows
endif

