#!/bin/sh

#git status | grep -E '*\.mk$|*\.c$|*\.h$|*\.cpp$|*\.hpp$' | cut -f2 -d: | xargs sed -i 's/[ ]*$//g'
git status | grep -E '*\.mk$|*\.c$|*\.h$|*\.cpp$|*\.hpp$' | cut -f2 -d: | sed 's/^[ \t]*//g' | xargs sed -i 's/[ ]*$//g'

