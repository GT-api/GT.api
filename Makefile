CXX = g++
CXXFLAGS = -std=c++2b -g -Iinclude -MMD -MP -DDEBUG_TIME

SOURCES := main.cpp enet_impl.cpp \
	include/database/items.cpp \
	include/database/peer.cpp \
	include/database/world.cpp \
	include/network/packet.cpp \
	include/state/punch.cpp
	
OBJECTS := $(SOURCES:.cpp=.o)
DEPS := $(OBJECTS:.o=.d)

TARGET := main.exe

all: .make $(TARGET)

$(TARGET): $(OBJECTS) include/compress.o
	$(CXX) $^ -o $@ -lws2_32 -lwinmm

.make :
	@mkdir -p .make

%.o: %.cpp | .make
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@mv $*.d .make/

-include $(DEPS)

clean:
	rm -rf $(OBJECTS) .make $(TARGET)