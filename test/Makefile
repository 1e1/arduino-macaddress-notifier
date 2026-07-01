# Host-side unit tests -- no hardware, no external dependency.
#   make test    build and run
#   make clean   remove the built binary

CXX      ?= c++
CXXFLAGS ?= -std=c++11 -Wall -Wextra -O2

SKETCH   := ../sketch_wiscan
INCLUDES := -Iarduino -I$(SKETCH)

BIN := test_rpnsolver

$(BIN): test_rpnsolver.cpp $(SKETCH)/RpnSolver.cpp $(SKETCH)/RpnSolver.h arduino/ESP8266WiFi.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ test_rpnsolver.cpp $(SKETCH)/RpnSolver.cpp

.PHONY: test clean
test: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN)
