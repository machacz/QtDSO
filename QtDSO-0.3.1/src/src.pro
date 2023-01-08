TEMPLATE        = app
CONFIG          = qt thread release warn_on  
INCLUDEPATH     = . moc
MOC_DIR         = moc
OBJECTS_DIR     = tmp
DEPENDPATH      = . 

release {
  contains( QMAKE_CC, gcc ) {
    message( "Adding release optimizations for gcc" )
    QMAKE_CFLAGS   += -finline-functions -fexpensive-optimizations \
                      -fstrict-aliasing -fomit-frame-pointer
    QMAKE_CXXFLAGS += -finline-functions -fexpensive-optimizations \
                      -fstrict-aliasing -fomit-frame-pointer
  }
  TARGET = yapide
}

debug {
  message( "Compiling for debug" )
  TARGET = yapidedebug
}

CPU_YAPIDE = $$system( echo $CPU )

contains( CPU_YAPIDE, i686 ) {
        message( "Configuring for Pentium Pro" )
        QMAKE_CFLAGS   += -march=pentiumpro
        QMAKE_CXXFLAGS += -march=pentiumpro
}

HEADERS    = dsowid.h \
             mainwid.h \
             type.h \
             mainwindow.h \
             simulator.h \
             simulatorwid.h \
             colorbutton.h \
             prefdlg.h \
             dsovumeter.h \
             pcs64i.h \
             dampedfloat.h \
             buttongrid.h \
             converterhistogramwid.h \
             converterhistogramdlg.h \
             simplecfg.h \
             xpm/icon.xpm

INTERFACES = uimainwid.ui \
             uiabout.ui \
             uisimulatorwid.ui \
             uiconverterhistogramdlg.ui \
             uiprefdlg.ui
                        
SOURCES   = main.cpp \
            mainwid.cpp \
            dsowid.cpp \
            mainwindow.cpp \
            simulator.cpp \
            simulatorwid.cpp \
            colorbutton.cpp \
            prefdlg.cpp \
            dsovumeter.cpp \
            pcs64i.cpp \
            dampedfloat.cpp \
            buttongrid.cpp \
            converterhistogramwid.cpp \
            converterhistogramdlg.cpp \
            simplecfg.cpp \
            dso.cpp

LIBS      = -lfftw3 -lieee1284 
TARGET    = qtdso
VERSION   = 0.3
DESTDIR   = ../bin

