BUILD:=/build
BUFFER:=./buffer
TEST:=./testing
CXX:=g++
CXXFLAGS:=-std=c++11 -g -O2 -Wextra

all:
	
	$(CXX) $(CXXFLAGS) -o $(BUILD)/BufferFrame.o	 -c	$(BUFFER)/BufferFrame.cpp	-lpthread
	$(CXX) $(CXXFLAGS) -o $(BUILD)/BufferManager.o   -c	$(BUFFER)/BufferManager.cpp	-lpthread
	$(CXX) $(CXXFLAGS) -o $(BUILD)/buffertest 	 -I$(BUFFER)   $(BUILD)/BufferManager.o $(BUILD)/BufferFrame.o $(TEST)/buffertest.cpp	-pthread

run:

	./$(BUILD)/buffertest $(disk) $(ram) $(threads)

go:

	make run disk=200 ram=100 threads=10

clean:
	rm -rf $(BUILD)/*
