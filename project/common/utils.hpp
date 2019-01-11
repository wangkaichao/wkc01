/*
* utils.hpp
* wangkaichao2@163.com 2018-09-21
*/
#ifndef _UTILS_H
#define _UTILS_H

#include <stdint.h>
#include <string>
#include <time.h>

namespace common {

	/*
	 * @brief get random ascii string, which length is 8 bytes
	 * @return random value
	 */
	std::string getRandString(int len = 8);

	/*
	 * @brief caculate md5
	 * @return md5 value
	 */
	std::string getMd5(const std::string& data);

	/*
	 * @brief get crc32
	 * @return crc32 value
	 */
	unsigned int crc32(const unsigned char *buf, int len);

	/*
	 * @brief get soft version
	 * @return version number
	 */
	std::string getSoftVersion();

	/*
	 * @brief get hardware type
	 * @return hardware type
	 */
	std::string getHardwareType();

	/*
	 * @brief get date
	 * @return YYYY-MM-DD
	 */
	std::string getDate();

	/*
	 * @brief get time
	 * @return hh:mm:ss
	 */
	std::string getTime();

	/*
	 * @brief get weekday
	 * @return Weekday
	 */
	std::string getWeekday();

	/*
	 * @brief get date and time
	 * @return YYYY-MM-DD hh:mm:ss
	 */
	std::string getDateTime();

	/*
	 * @brief get date-time
	 * @param[out] ptm
	 */
	void getDateTime(struct tm *ptm);

    /**
     * @brief get build date time
     *
     * @param date
     * @param time
     */
    void getBuildDateTime(string &date, string &time);
}

#endif
