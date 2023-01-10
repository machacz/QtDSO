This is build environment for QtDSO, frontend for PC-based oscilloscope, Velleman PSC54i

http://www.mtoussaint.de/qtdso.html


For ubuntu 12: 

(following checked with aptitude)

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



For Ubuntu 14 I installed following packages:

fontconfig-config_2.12.6-0ubuntu2_all.deb
libfontconfig1_2.12.6-0ubuntu2_amd64.deb
libstdc++6_8-20180414-1ubuntu2_amd64.deb
libtqt3-mt_14.0.13-0ubuntu18.04.0+0_amd64.deb
zlib1g_1.2.11.dfsg-0ubuntu2_amd64.deb
gcc-8-base_8-20180414-1ubuntu2_amd64.deb
libpng16-16_1.6.34-1_amd64.deb
libtqt3-mt-dev_14.0.13-0ubuntu18.04.0+0_amd64.deb
tqt3-dev-tools_14.0.13-0ubuntu18.04.0+0_amd64.deb

all downloaded from https://ubuntu.pkgs.org/