This is build environment for QtDSO, frontend for PC-based oscilloscope, Velleman PSC54i

http://www.mtoussaint.de/qtdso.html


For installing GPP:

The following actions will resolve these dependencies:

     Install the following packages:
1)     libc-dev-bin [2.15-0ubuntu10 (precise)]
2)     linux-libc-dev [3.2.0-23.36 (precise)]
3)     manpages [3.35-0.1ubuntu1 (precise)]
4)     manpages-dev [3.35-0.1ubuntu1 (precise)]

     Downgrade the following packages:
5)     libc-bin [2.15-0ubuntu10.15 (now) -> 2.15-0ubuntu10 (precise)]
6)     libc6 [2.15-0ubuntu10.15 (now) -> 2.15-0ubuntu10 (precise)]



For libqt3-mt-dev:

The following actions will resolve these dependencies:

     Install the following packages:
1)     libkms1 [2.4.32-1ubuntu1 (precise)]

     Downgrade the following packages:
2)     libcomerr2 [1.42-1ubuntu2.3 (now) -> 1.42-1ubuntu2 (precise)]
3)     libdrm-intel1 [2.4.52-1~precise2 (now) -> 2.4.32-1ubuntu1 (precise)]
4)     libdrm-nouveau1a [2.4.52-1~precise2 (now) -> 2.4.32-1ubuntu1 (precise)]
5)     libdrm-radeon1 [2.4.52-1~precise2 (now) -> 2.4.32-1ubuntu1 (precise)]
6)     libdrm2 [2.4.52-1~precise2 (now) -> 2.4.32-1ubuntu1 (precise)]
7)     libpng12-0 [1.2.46-3ubuntu4.2 (now) -> 1.2.46-3ubuntu4 (precise)]


in dsowid.h, add:

#include <stdlib.h>


Requirements for running libfftw3 libieee1284