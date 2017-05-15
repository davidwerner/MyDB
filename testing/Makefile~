BUFFER_MNGR = ../buffer

BUILD = ../build

INCLUDES := -I$(BUFFER_MNGR) -I$(PAGES) -I$(INDEX) -I$(PARSER) -I$(OPERATOR)
OBJECTS := $(BUILD)/*.o -pthread
CXX:=g++
CXXFLAGS:=-std=c++11 -g -O2 -Werror

all:
	#$(CXX) $(CXXFLAGS) -o $(BUILD)/BufferManagerTest 	-I$(BUFFER_MNGR)   $(BUILD)/BufferManager.o $(BUILD)/BufferFrame.o BufferManagerTest.cpp  -pthread
	#$(CXX) $(CXXFLAGS) -o $(BUILD)/BufferManagerTest2 	-I$(BUFFER_MNGR)   $(BUILD)/BufferManager.o $(BUILD)/BufferFrame.o BufferManagerTest2.cpp -pthread
	$(CXX) $(CXXFLAGS) -o $(BUILD)/buffertest 			-I$(BUFFER_MNGR)   $(BUILD)/BufferManager.o $(BUILD)/BufferFrame.o buffertest.cpp 		  -pthread	  
clean:
	rm -rf $(BUILD)/BufferManagerTest
