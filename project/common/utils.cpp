/*
* utils.cpp
* wangkaichao2@163.com 2018-10-12
*/
#include <md5.hh>
#include <errno.h>
#include <sstream>
#include <array>

#include "utils.hpp"
#include "exception.hpp"
#include "clog.h"

using namespace std;

string common::getRandString(int len)
{
  FILE *file = fopen("/dev/urandom", "r");
  if (!file)
    THROW(ERR_FOPEN);

  array<char, 8> buffer;
  fread(buffer.data(), len, 1, file);
  fclose(file);

  for (auto& it : buffer)
    it = (it & 0x7F) * (126 - 32) / 127 + 33;

  return string(buffer.begin(), buffer.end());
}

string common::getMd5(const string& data)
{
  if (data.empty())
    THROW(ERR_ARG);

  MD5 md5;
  md5.update((unsigned char*)data.c_str(), data.length());
  md5.finalize();
  char* ptr = md5.hex_digest();
  string result = ptr;
  delete[] ptr;
  return result;
}

unsigned int common::crc32(const unsigned char *buf, int len)
{
    static unsigned char isFirst = 1;
    static unsigned int m_table[256];

    unsigned int crc = 0xffffffff;

    if (isFirst) {
        const unsigned int POLYNOMIAL = 0x04c11db7;
        unsigned short i, j;
        unsigned int crc_accum;

        isFirst = 0;

        for (i = 0; i < 256; i++) {
            crc_accum = (i << 24);

            for (j = 0; j < 8; j++) {
                if (crc_accum & 0x80000000L)
                    crc_accum = (crc_accum << 1) ^ POLYNOMIAL;
                else
                    crc_accum = (crc_accum << 1);
            }

            m_table[i] = crc_accum;
        }
    }

    while (len--) {
        crc = (crc << 8) ^ m_table[((crc >> 24) ^ *buf++) & 0xff];
    }

    return crc;
}

string common::getSoftVersion()
{
	return string("1.0.0");
}

std::string common::getHardwareType()
{
	return string("Hi3516D");
}

string common::getDate()
{
	struct tm tm;
	getDateTime(&tm);

	array<char, 64> buf;
	strftime(buf.data(), buf.size(), "%F", &tm);

	return string(buf.data());
}

string common::getTime()
{
	struct tm tm;
	getDateTime(&tm);

	array<char, 64> buf;
	strftime(buf.data(), buf.size(), "%T", &tm);

	return string(buf.data());
}

string common::getWeekday()
{
	struct tm tm;
	getDateTime(&tm);

	array<char, 64> buf;
	strftime(buf.data(), buf.size(), "%a", &tm);

	return string(buf.data());
}

string common::getDateTime()
{
	struct tm tm;
	getDateTime(&tm);

	array<char, 64> buf;
	strftime(buf.data(), buf.size(), "%F %T %a", &tm);

	return string(buf.data());
}

void common::getDateTime(struct tm *ptm)
{
	struct timespec spec;

	/*if (setenv("TZ", "GMT-8", 1) == -1)
	if (setenv("TZ", "CST-8", 1) == -1)
	if (putenv("TZ=CST") == -1)
			return -1;

	tzset();
	printf("daylight:%d, timezone:%ld, tzname[0]:%s\n",
			daylight, timezone / 3600, tzname[0]);*/

	int ret = clock_gettime(CLOCK_REALTIME, &spec);

	if (ret != 0) {
		log_dbg("%m\n", errno);
		THROW(ERR_TIME);
	}

	localtime_r(&spec.tv_sec, ptm);
}

void common::getBuildDateTime(string &date, string &time)
{
    const char szEnglishMonth[12][4] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    char szTmpDate[16] = {0};
    char szMonth[4] = {0};
    int iYear,iMonth,iDay;
    array<char, 64> buf;

    sprintf(szTmpDate, "%s", __DATE__); //"Sep 18 2010"
    sscanf(szTmpDate,"%s %d %d", szMonth, &iDay, &iYear);
    for (int i = 0; i < 12; i++) {
        if (strncmp(szMonth, szEnglishMonth[i], 3) == 0) {
            iMonth = i + 1;
            break;
        }
    }
    sprintf(buf.data(), "%04d-%02d-%02d", iYear, iMonth, iDay);
    date = string(buf.data());

    sprintf(buf.data(), "%s", __TIME__); //"10:59:19"
    time = string(buf.data());
}
