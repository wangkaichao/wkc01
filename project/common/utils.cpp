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

/*
 * @brief get random ascii string, which length is 8 bytes
 * @return random value
 */
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

/*
 * @brief caculate md5
 * @return md5 value
 */
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

/*
 * @brief get crc32
 * @return crc32 value
 */
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

/*
 * @brief get soft version
 * @return version number
 */
string common::getSoftVersion()
{
	return string("1.0.0");
}

/*
 * @brief get hardware type
 * @return hardware type
 */
std::string common::getHardwareType()
{
	return string("Hi3516D");
}

/*
 * @brief get build date 
 * @return YYYY-MM-DD
 */
string common::getBuildDate()
{
	struct tm tm;
	getBuildDateTime(&tm);
	
	array<char, 64> buf;
	strftime(buf.data(), buf.size(), "%F", &tm);
	
	return string(buf.data());
}

/*
 * @brief get build time 
 * @return hh:mm:ss
 */
string common::getBuildTime()
{
	struct tm tm;
	getBuildDateTime(&tm);
	
	array<char, 64> buf;
	strftime(buf.data(), buf.size(), "%T", &tm);
	
	return string(buf.data());
}

/*
 * @brief get build weekday
 * @return Weekday
 */
string common::getBuildWeekday()
{
	struct tm tm;
	getBuildDateTime(&tm);
	
	array<char, 64> buf;
	strftime(buf.data(), buf.size(), "%a", &tm);
	
	return string(buf.data());
}

/*
 * @brief get build date and time
 * @return YYYY-MM-DD hh:mm:ss
 */
string common::getBuildDateTime()
{
	struct tm tm;
	getBuildDateTime(&tm);
	
	array<char, 64> buf;
	strftime(buf.data(), buf.size(), "%F %T %a", &tm);
	
	return string(buf.data());
}

/*
 * @brief get build date-time
 * @param[out] ptm
 */
void common::getBuildDateTime(struct tm *ptm)
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

