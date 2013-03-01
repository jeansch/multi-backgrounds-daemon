#!/bin/sh

libtoolize --force
aclocal
autoconf
autoheader
automake --add-missing
./configure
