TARGET = opt
IF = out.3addr
OPT = none

LDFLAGS =
CXX = g++
CXXFLAGS = -std=c++11 -g -Isrc

all: dirs csc $(TARGET) run

run: $(TARGET) $(IF)
	./$(TARGET) opt=$(OPT) < $(IF)

csc:
	cd csc_src && make

src = $(wildcard src/*.cpp)
obj = $(patsubst src/%,build/%,$(src:.cpp=.o))
dep = $(obj:.o=.dep)

dirs:
	@mkdir -p build

$(TARGET): $(obj)
	$(CXX) -o $(TARGET) $^ $(LDFLAGS)


build/%.dep: src/%.cpp
	$(CXX) $(CXXFLAGS) $< -MM -MT $(@:.dep=.o) >$@

-include $(dep)

build/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build/* csc $(TARGET)

.PHONY: dirs clean