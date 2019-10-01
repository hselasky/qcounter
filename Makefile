#
# Copyright (c) 2014 Hans Petter Selasky. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
#
# Makefile for Quick Random Counter
#

VERSION=1.0.0

DESTDIR?=
PREFIX?=/usr/local

all: Makefile.unix
	make -f Makefile.unix -j2 all

Makefile.unix: qcounter.pro
	qmake PREFIX=${PREFIX} DESTDIR=${DESTDIR} \
		-o Makefile.unix qcounter.pro
help:
	@echo "Targets are: all, install, clean, package, help"

install: Makefile.unix
	make -f Makefile.unix install

clean: Makefile.unix
	make -f Makefile.unix clean
	rm -f Makefile.unix

package: clean

	tar -cvf temp.tar --exclude="*~" --exclude="*#" \
		--exclude=".svn" --exclude="*.orig" --exclude="*.rej" \
		Makefile qcounter*.pro qcounter*.qrc \
		qcounter*.cpp qcounter*.h *.png qcounter*.desktop

	rm -rf qcounter-${VERSION}

	mkdir qcounter-${VERSION}

	tar -xvf temp.tar -C qcounter-${VERSION}

	rm -rf temp.tar

	tar --uid=0 --gid=0 -jcvf qcounter-${VERSION}.tar.bz2 qcounter-${VERSION}

