CXX = g++
CXXFLAGS = -std=c++2b -g -Iinclude -MMD -MP -DDEBUG_TIME

SOURCES = main.cpp include/database/items.cpp include/database/peer.cpp enet_impl.cpp
OBJECTS = $(SOURCES:.cpp=.o) include/compress.o
DEPS = $(DEPS_DIR)/$(OBJECTS:.o=.d)

TARGET = main.exe

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ -lws2_32 -lwinmm

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@mkdir -p DEPS
	@mv $*.d DEPS/

-include $(DEPS)

clean:
	rm -rf $(OBJECTS) DEPS $(TARGET)