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

    /**
     * @brief tz=tzn[+|-]hh[:mm[:ss]][dzn]
     * 其实tzn和dzn可为任意3个字母，只要中间的时差设置正确，localtime（）和gmtime（）函数调用都会处理正确。要注意一点，比如要将时区TZ设置为中国所在的东八区（即UTC+8:00）

     * daylight  如果在TZ设置中指定夏令时时区       1则为非0值;否则为0
     * timezone  UTC和本地时间之间的时差,单位为秒   28800(28800秒等于8小时)
     * tzname[0] TZ环境变量的时区名称的字符串值     如果TZ未设置则为空 PST
     * tzname[1] 夏令时时区的字符串值;              如果TZ环境变量中忽略夏令时时区则为空。
     * @param tz=tzn[+|-]hh[:mm[:ss]][dzn]
     */
    void setZone(std::string &tz);

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
    void getBuildDateTime(std::string &date, std::string &time);
}

#endif
