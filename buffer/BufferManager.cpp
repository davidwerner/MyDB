#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fstream>
#include <exception>
#include <string>
#include <cstring>
#include <iostream>

#include "BufferFrame.hpp"
#include "BufferManager.hpp"

using namespace std;


/*
* Constructor - pageCount limits the amount of pages that can be in the memory simultaneously
*/
BufferManager::BufferManager(unsigned pageCount) : _pageCount(pageCount), to_delete(NULL), just_unfixed(NULL) {

}

/*
* Fixes frame - exclusive flag indictates if a lock for a writing access is needed
*
* If a Frame is requested which is not already part of the frame map a new frame ist created.
* The data of such a frame is loaded within its 'getData'-method. This has 2 major advantages for 'fixPage()':

* 1. The Constructor fo a BufferFrame is simple. It just needs a pageId and a file descriptor and can therefore easily be used with
*    the 'emplace'-method of unordered map.
*
* 2. No read or write operation has to be performed within 'fixPage'. We just need to open the corresponding file (sometimes).
*
*
*
* Another importan oint is the order in which th locks are acquired. 
* 'fixPage' always tries to acquire the global lock for the map first and searches for the desired BufferFrame.
* After it either found or deleted the frame it releases the global lock BEFORE it tries to acquire the lock for the latch!
*
*/
BufferFrame &BufferManager::fixPage(uint64_t pageId, bool exclusive) {

	this->m.lock();

	BufferFrame *f;

	try {

		// Looking for page in frame map
		f = &frames.at(pageId);
		
		removeFromEvictionQueue(f);

		this->m.unlock();
		f->lock(exclusive);

		return *f;

	} catch (const std::out_of_range &e) {


		// Page was not found in frame map
		if (frames.size() >= _pageCount) {

			// not enough memory space -> least recently used page gets evicted
			BufferFrame *evictim = lru(); // no typo just a pun :)

			if (evictim == NULL) {
				this->m.unlock();
				throw invalid_argument("too many used pages");
			}

			frames.erase(evictim->getPageId());
		}

		// Retrieve file descriptor of desired segment
		uint64_t segNumber = pageId >> 48;
		int fd;

		try {

			fd = segments.at(segNumber);

		} catch (const std::out_of_range &e) {

			fd = open_segment(segNumber);
		}
		
		//create new frame in frame map
		auto result = frames.emplace(std::piecewise_construct,
                	std::forward_as_tuple(pageId),
                	std::forward_as_tuple(fd, pageId)
      				);

		f = &result.first->second;
	
		f->_userNumber++;

		// Unlock global lock
		this->m.unlock();

		// Try to acquire lock for newly created frame
		f->lock(exclusive);

		return *f;
	}


}

/*
* Unfixes a frame and marks it dirty if it was modified
*/
void  BufferManager::unfixPage(BufferFrame &frame, bool isDirty) {

	//set dirty flag
	if (isDirty) frame.setDirty();

	frame.unlock();

	// insert into lru eviction queue
	insertInEvictionQueue(&frame);
}

/*
* Opens the file corresponding to the segment number
*/
int BufferManager::open_segment(uint64_t seg_no) {
	int fd;
	char segment[15];

    sprintf(segment, "%lu", seg_no);

    // open the segment file
    fd = open(segment, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); // | O_NOATIME | O_SYNC
    if (fd < 0) {
        throw std::runtime_error(std::strerror(errno));
    }

    //put segment file descriptor into segment map
    segments[seg_no] = fd;

    return segments.at(seg_no);
}

/*
* Closes all currently opened segment files
*/
void BufferManager::close_segments() {
	for (auto it = segments.begin(); it != segments.end(); ++it) {
		close(it->second);
	}
}

/*
* Returns least recently used frame
*/
BufferFrame *BufferManager::lru(){
	this->m_lru.lock();

	BufferFrame* f = to_delete;
	if(to_delete != NULL) {
		to_delete = f->next;
		f->next = NULL;

		if(to_delete != NULL)
			to_delete->prev = NULL;
		else
		just_unfixed = NULL;
	}

	this->m_lru.unlock();

	return f;
}

/*
* Inserts frame into the lru eviction queue
*/
void BufferManager::insertInEvictionQueue(BufferFrame *f){
	this->m_lru.lock();

	if((--f->_userNumber) == 0) {
		if(just_unfixed == NULL) {
			to_delete = f;
		} else {
			f->prev  = just_unfixed;
			just_unfixed->next = f;
		}
			just_unfixed = f;
	}

	this->m_lru.unlock();
}

/*
* Removes frame from the lru eviction queue
*/
void BufferManager::removeFromEvictionQueue(BufferFrame *f){
	this->m_lru.lock();

	if(f->_userNumber++ == 0) {
		if (f->prev != NULL) f->prev->next = f->next;

		if (f->next != NULL) f->next->prev = f->prev;

		if (to_delete == f) to_delete = f->next;

		if (just_unfixed == f) just_unfixed = f->prev;

		f->next = NULL;
		f->prev = NULL;
    }

    this->m_lru.unlock();
}

/*
* Destructor - writes all dirty frames to segment file and closes open segments
*/
BufferManager::~BufferManager() {

	this->m.lock();

	for (auto it = frames.begin(); it != frames.end(); ++it) {
		if (it->second.state == frame_state_t::DIRTY) {
			it->second.store();
		}
	}

	close_segments();
}
