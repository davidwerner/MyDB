

#ifndef _BUFFERMANAGER_H_
#define _BUFFERMANAGER_H_

#include <list>
#include <inttypes.h>
#include <unordered_map>
#include <map>
#include "BufferFrame.hpp"
#include <mutex>
#include <pthread.h>

using namespace std;


class BufferManager {

public:

	BufferManager(unsigned pageCount);

	BufferFrame& fixPage(uint64_t pageId, bool exclusive);
	void  unfixPage(BufferFrame& frame, bool isDirty);

	~BufferManager();

private:

	uint64_t 										_pageCount;
	std::unordered_map<uint64_t, BufferFrame> 		frames;
	BufferFrame*									to_delete;
	BufferFrame*									just_unfixed;

	std::unordered_map<uint64_t, int>				segments;

	mutex m;

	int open_segment(uint64_t seg_no);

	uint64_t file_size(string file_name);

	void close_segments();

	mutex m_lru;

	void insertInEvictionQueue(BufferFrame *f);

	void removeFromEvictionQueue(BufferFrame *f);

	BufferFrame *lru();
};


#endif /* _BUFFERMANAGER_H_ */
