CXX = g++
CXXFLAGS = -std=c++23 -g -Iinclude -MMD -MP -DDEBUG_TIME

SOURCES := main.cpp \
	include/database/items.cpp \
	include/database/peer.cpp \
	include/database/world.cpp \
	include/network/packet.cpp \
	include/network/compress.cpp \
	include/network/enet_impl.cpp \
	include/state/punch.cpp \
	include/state/pickup.cpp \
	include/state/movement.cpp \
	include/state/equip.cpp
	
OBJECTS := $(SOURCES:.cpp=.o)
DEPS := $(OBJECTS:.o=.d)
PCH := .make/pch.gch

TARGET := main.exe

all: .make $(PCH) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $^ -o $@ -lws2_32 -lwinmm

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