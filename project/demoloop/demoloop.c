#include <stdio.h>
#include "eventloop.h"
#include "clog.h"

int time_func(void *data)
{
	static int i = 0;
	long id = (long)data;

	log_dbg("id:%ld, cnt:%d\n", id, ++i);
	return 0;
}

int main()
{
	int cnt = 0;
	event_loop_t* pstLoop =  event_loop_create();

	event_source_t* pstSource = event_loop_add_timer(pstLoop, time_func, 0, (void*)1);
	event_source_timer_update(pstSource, 1, 1000);

	while (1) {
		event_loop_dispatch(pstLoop, -1);
		if (++cnt >= 10) {
			event_source_timer_update(pstSource, 0, 0);
			break;
		}
	}
	event_source_remove(pstSource);
	event_loop_destroy(pstLoop);

	return 0;
}
