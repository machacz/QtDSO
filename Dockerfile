FROM testbeds/ubuntu:16.04
#latest checked is 16
#RUN echo "deb http://old-releases.ubuntu.com/ubuntu trusty main universe multiverse" > /etc/apt/sources.list; 
COPY ./debs /root/debs
COPY ./QtDSO-0.3.1 /root/QtDSO-0.3.1
RUN apt-get update; apt-get -y upgrade
RUN apt-get -yf install wget unzip make libieee1284-3-dev libfftw3-dev
 RUN apt-get -yf install gpp g++ aptitude nano qt4-qmake
ENV QTDIR /usr/share/qt3
RUN dpkg -i ./debs/*.deb || true
#Now resolve deps manually
# RUN apt-get -yf install libjpeg-dev libjpeg8-dev libjpeg-turbo8-dev libc-dev libc6-dev
# RUN apt-get -yf install libqt3-mt-dev
# RUN wget http://www.mtoussaint.de/qtdso-0.3.1.tgz && tar zxvf qtdso-0.3.1.tgz && ls 
# RUN cd QtDSO-0.3.1 && ./configure && make
CMD sleep infinity
