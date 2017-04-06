CONTEXT=sdl2
ifeq "$(shell uname)" "Darwin"
    CONTEXT=glfw3
    LDFLAGS += -lobjc -framework Foundation -framework OpenGL -framework Cocoa
endif

CXXFLAGS += -g -W -Wall -Wno-unused-parameter -Wno-deprecated-declarations
CXXFLAGS += $(shell pkg-config --cflags glew)
CXXFLAGS += $(shell pkg-config --cflags $(CONTEXT))

LDFLAGS += -g
LDFLAGS += $(shell pkg-config --libs glew)
LDFLAGS += $(shell pkg-config --libs $(CONTEXT))

TP="tp5"
SRC=main

exe : $(SRC).exe
run : exe
	./$(SRC).exe
$(SRC).exe : $(SRC).cpp *.h
	$(CXX) $(CXXFLAGS) -o$@ $(SRC).cpp $(LDFLAGS)

sol :  ; make SRC=$(SRC)Solution exe
runs : ; make SRC=$(SRC)Solution run

clean :
	rm -rf *.o *.exe *.exe.dSYM

remise zip :
	make clean
	rm -f remise_$(TP).zip
	zip -r remise_$(TP).zip *.cpp *.h *.glsl makefile *.txt textures
