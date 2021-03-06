#include <thread>
#include "queue_r.hpp"
#include "wm_log.h"

#define TAG "demoqueue_r"

#if 1
static void push_func(int id, void *param)
{
	queue_r<int>* q = (queue_r<int>*)param;
	q->push(id);
}

static void pop_func(void *param)
{
	static int cnt = 0;

	while (cnt < 30) {
		queue_r<int>* q = (queue_r<int>*)param;
		auto val = q->wait_pop();
		cnt++;
		LOGD("val:%d\n", *val);
	}
}
#endif

int main()
{
    LOG_OPEN("demo");
	LOGD("run ...\n");

	queue_r<int> q; 

	for (int i = 0; i < 30; i++)
		std::thread(push_func, i, (void*)&q).detach();

	std::thread t2(pop_func, (void*)&q);
	t2.join();

	return 0;
}
