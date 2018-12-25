#include <stdint.h>
#include <unistd.h>
#include <thread>
#include <string>
#include <array>
#include <time.h>

#include "clog.h"
#include "utils.hpp"

using namespace std;
using namespace common;

static void thread_fun(long param)
{
    long id = param;
    
    while (1) {
        log_dbg("~~~~~~~~~~~~~~id:%u\n", id);
        usleep(100*1000);
    }
}

int demo_main()
{
    for (long i = 0; i < 20; i++) {
        std::thread(thread_fun, i).detach();
    }

    while (1)
        sleep(100);

    return 0;
}

int main()
{
	string buildinfo = getBuildDateTime();
	string date = getBuildDate();
	string time = getBuildTime();
	string weekday = getBuildWeekday();

	log_dbg("Version:%s, Build:%s, %s, %s, %s\n",
			getSoftVersion().c_str(),
			buildinfo.c_str(),
			date.c_str(),
			time.c_str(),
			weekday.c_str());

	struct tm tm;
	getBuildDateTime(&tm);
	
	array<char, 64> buf;
	strftime(buf.data(), buf.size(), "%F %T %a", &tm);
	log_dbg("%s\n", buf.data());

	log_dbg("%s\n", getHardwareType().c_str());
	return 0;
}
