CXX = g++
CXXFLAGS = -std=c++23 -g -Iinclude -MMD -MP -DDEBUG_TIME

LIBS :=
ifeq ($(OS),Windows_NT)
    LIBS += -lws2_32 -lwinmm
endif

SOURCES := main.cpp \
	include/database/items.cpp \
	include/database/peer.cpp \
	include/database/world.cpp \
	include/network/packet.cpp \
	include/network/compress.cpp \
	include/network/enet_impl.cpp \
	include/event_type/type_receive.cpp \
	include/state/punch.cpp \
	include/state/pickup.cpp \
	include/state/movement.cpp \
	include/state/equip.cpp \
	include/action/logging_in.cpp \
	include/action/enter_game.cpp \
	include/action/dialog_return.cpp \
	include/action/join_request.cpp \
	include/action/quit_to_exit.cpp \
	include/action/quit.cpp \
	include/action/input.cpp \
	include/action/drop.cpp \
	include/action/respawn.cpp \
	include/action/friends.cpp \
	include/on/EmoticonDataChanged.cpp \
	include/on/Action.cpp \
	include/on/RequestWorldSelectMenu.cpp \
	include/commands/commands.cpp \
	include/commands/warp.cpp \
	include/commands/find.cpp \
	include/tools/randomizer.cpp
	
OBJECTS := $(SOURCES:.cpp=.o)
DEPS := $(OBJECTS:.o=.d)
PCH := .make/pch.gch

TARGET := main.exe

all: .make $(PCH) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $^ -o $@ $(LIBS)

.make :
	@mkdir -p .make

$(PCH): include/pch.hpp | .make
	$(CXX) $(CXXFLAGS) -x c++-header $< -o $@

%.o: %.cpp $(PCH) | .make
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@mv $*.d .make/

-include $(DEPS)

clean:
	rm -rf $(OBJECTS) .make $(TARGET) $(PCH)