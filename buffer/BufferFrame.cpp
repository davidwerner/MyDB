
#include "BufferFrame.hpp"
#include <iostream>
#include <unistd.h>
#include <string>
#include <cstring>

using namespace std;


/*
* Constructor - latch (rwlock) gets initialized
*/
BufferFrame::BufferFrame(int fd, uint64_t pageId) : _fd(fd), _pageId(pageId), state(CREATED), prev(NULL), next(NULL), _data(NULL), _pageNo(pageId & 0x0000FFFFFFFFFFFF), _userNumber(0){
	pthread_rwlock_init(&_latch, NULL);
}

/*
* Destructor - latch (rwlock) gets destroyed + frame content is stored on diks itf its dirty
*/
BufferFrame::~BufferFrame(){
    pthread_rwlock_destroy(&_latch);

    // Write data to file if it was modified
    if(state == frame_state_t::DIRTY){
    	store();
    }

    // free memory
    if (_data != NULL) {
        free(_data);
        _data = NULL;
    }
}
	
uint64_t BufferFrame::getPageId(){
	return this->_pageId;
}

/*
* Acquires read or write lock of the frame based on exclusive flag
*/
void BufferFrame::lock(bool exclusive){
	if (exclusive) {
        pthread_rwlock_wrlock(&_latch);
    } else {
        pthread_rwlock_rdlock(&_latch);
    }
}

/*
* Unlocks the frame
*/
void BufferFrame::unlock(){
	pthread_rwlock_unlock(&_latch);
}


/*
* Returns the data content of the frame - fresh created frames need to load the data first
*/
void* BufferFrame::getData(){
	if (this->state == frame_state_t::CREATED) {
		load();
	}

	return _data;
}

/*
* Loads the content of the frame by using the file descriptor 
*/
void BufferFrame::load(){
	// Allocate memory for data
	_data = malloc(PAGE_SIZE);

    assert(_data != NULL);

    // Read data
    if(pread(_fd, _data, PAGE_SIZE, _pageNo * PAGE_SIZE) < 0) {
    	std::cerr << "cannot read from segment " << strerror(errno) << std::endl;
    	return;
    }

    // If data is just read, the frame cannot be dirty
    state = frame_state_t::CLEAN;
}

/*
* Writes the content of a frame back to the segment file
*/
void BufferFrame::store(){
	if(pwrite(_fd, _data, PAGE_SIZE, _pageNo * PAGE_SIZE) < 0){
		std::cerr << "cannot write to segment " << strerror(errno) << std::endl;
		return;
	}

    state = frame_state_t::CLEAN;
}

