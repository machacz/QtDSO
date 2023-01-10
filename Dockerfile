FROM testbeds/ubuntu:12.04
RUN echo "deb http://old-releases.ubuntu.com/ubuntu precise main universe multiverse" > /etc/apt/sources.list; 
RUN apt-get --allow-unauthenticated update; apt-get --allow-unauthenticated upgrade
RUN apt-get --allow-unauthenticated -yf install wget unzip make libieee1284-3-dev libfftw3-dev
# RUN apt-get --allow-unauthenticated -yf install gpp
# RUN apt-get --allow-unauthenticated -yf install libjpeg-dev libjpeg8-dev libjpeg-turbo8-dev libc-dev libc6-dev
# RUN apt-get --allow-unauthenticated -yf install libqt3-mt-dev
# ENV QTDIR /usr/share/qt3
# RUN wget http://www.mtoussaint.de/qtdso-0.3.1.tgz && tar zxvf qtdso-0.3.1.tgz && ls 
# RUN cd QtDSO-0.3.1 && ./configure && make
CMD sleep infinity
