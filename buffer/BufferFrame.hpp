

#ifndef _BUFFERFRAME_H_
#define _BUFFERFRAME_H_

#define PAGE_SIZE 4096

#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <assert.h>

using namespace std;

enum frame_state_t {CLEAN, DIRTY, CREATED};

class BufferFrame {

	friend class BufferManager;

private:

	uint64_t _pageId;
	uint64_t _pageNo;
	int _fd;
	void *_data;
	BufferFrame* prev;
	BufferFrame* next;
	pthread_rwlock_t _latch;
	unsigned _userNumber;
	
	void lock(bool exclusive);

	void unlock();

	void load();

	void store();

	void setDirty() { this->state = frame_state_t::DIRTY; }

public:

	BufferFrame(int fd,uint64_t pageId);
	~BufferFrame();

	BufferFrame& operator=(BufferFrame& bf) = delete;

	frame_state_t state;

	uint64_t getPageId();

	void* getData();
};


#endif /* _BUFFERFRAME_H_ */

