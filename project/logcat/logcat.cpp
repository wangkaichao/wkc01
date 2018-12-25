#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

#include "utils.hpp"
#include "log.hpp"

int main()
{
    int8_t buf[1024];

    while (true) {
        size_t len = clslog::instance(0).read(buf, sizeof(buf));
        
        if (len) {
            for (size_t i = 0; i < len; i++)
                putchar(buf[i]);
        }
        else {
            usleep(10 * 1000);
        }
    }

    return 0;
}

