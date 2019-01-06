#!/bin/sh
#find /opt/hisi-linux/x86-arm/arm-hisiv300-linux/arm-hisiv300-linux-uclibcgnueabi/include/c++/4.8.3 -name "*" > c++.files
#cscope -bkq -i c++.files -f c++.out

#find /opt/hisi-linux/x86-arm/arm-hisiv300-linux/target/usr/include -name "*.[h|c]" > c.files
#cscope -bkq -i c.files -f c.out

find /usr/include -name "*" > sys.files
cscope -bkq -i sys.files -f sys.out

# cs add c.out c++.out
