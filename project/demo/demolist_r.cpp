#include <thread>
#include "list_r.hpp"
#include "clog.h"

#if 1
static void push_func(int id, void *param)
{
	list_r<int>* l = (list_r<int>*)param;
    l->push_front(id);
}

static void pop_func(void *param)
{
	int cnt = 0;

	while (cnt < 30) {
		list_r<int>* l = (list_r<int>*)param;
		auto val = l->find_first_if([cnt](int const& id) {return id == cnt;});
		cnt++;
		log_dbg("val:%d\n", *val);
	}
}
#endif

int main()
{
	log_dbg("run ...\n");

	list_r<int> l; 

	for (int i = 0; i < 30; i++)
		std::thread(push_func, i, (void*)&l).detach();

	std::thread t2(pop_func, (void*)&l);
	t2.join();

	return 0;
}
