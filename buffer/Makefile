BUILD:=../build
CXX:=g++
CXXFLAGS:=-std=c++11 -g -O2 -Wextra

all:
	
	$(CXX) $(CXXFLAGS) -o $(BUILD)/BufferFrame.o	 -c	BufferFrame.cpp 	-lpthread
	$(CXX) $(CXXFLAGS) -o $(BUILD)/BufferManager.o   -c	BufferManager.cpp 	-lpthread

clean:
	rm -rf $(BUILD)/BufferManager.o
	rm -rf $(BUILD)/BufferFrame.o	
