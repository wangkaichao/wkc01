#!/bin/sh
ctags --c++-kinds=+p --fields=+iaS --extra=+q -R -f tags ./ 

#ctags -I __THROW --file-scope=yes --langmap=c:+.h --languages=c,c++ --links=yes --c-kinds=+p --fields=+S  -R -f ~/.vim/systags /usr/include /usr/local/include
