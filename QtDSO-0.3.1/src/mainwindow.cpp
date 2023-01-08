//======================================================================
// File:		mainwindow.cpp
// Author:	Matthias Toussaint
// Created:	Sat Jun 29 11:51:09 CEST 2002
//----------------------------------------------------------------------
// Permission to use, copy, modify, and distribute this software and its
// documentation  for any  purpose and  without fee is  hereby  granted,
// provided  that below copyright notice appear  in all copies  and that
// both  that  copyright  notice and  this permission  notice  appear in
// supporting documentation.
// 
// This  file is  provided AS IS  with no  warranties  of any kind.  The
// author shall  have no liability  with respect  to the infringement of
// copyrights, trade  secrets  or any patents by  this file  or any part
// thereof.  In no event will the author be liable  for any lost revenue
// or profits or other special, indirect and consequential damages.
//----------------------------------------------------------------------
// (c) 2000-2002 Matthias Toussaint
//======================================================================

#include <qaction.h>
#include <qdir.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qprinter.h>
#include <qstatusbar.h>
#include <qwhatsthis.h>
#include <qapplication.h>

#include <uiabout.h>

#include <dampedfloat.h>
#include <dsovumeter.h>
#include <mainwid.h>
#include <mainwindow.h>
#include <prefdlg.h>

#include "xpm/icon.xpm"

MainWindow::MainWindow( QWidget *parent, const char *name ) :
  QMainWindow( parent, name ),
  m_about( 0 ),
  m_waitingTrigger( false )
{
  setIcon( QPixmap( (const char **)icon_xpm ) );
  setCaption( "QtDSO 0.3.1 (c) 2002-2007 M. Toussaint" );
  
  m_prefDlg = new PrefDlg( this );
  m_prefDlg->setIcon( *icon() );
  connect( m_prefDlg, SIGNAL( applyPrefs() ),
           this, SLOT( applyPrefsSLOT() ));
  
  m_mainWid = new MainWid( this );
  setCentralWidget( m_mainWid );
  connect( m_mainWid, SIGNAL( dynamicRange( int, int, int, int, int ) ),
           this, SLOT( dynamicRangeSLOT( int, int, int, int, int ) ));
  connect( m_mainWid, SIGNAL( resetFps() ),
           this, SLOT( resetFpsSLOT() ));
  connect( m_mainWid, SIGNAL( fps( float ) ),
           this, SLOT( fpsSLOT( float ) ));
  connect( m_mainWid, SIGNAL( triggerOk( bool ) ),
           this, SLOT( triggerOkSLOT( bool ) ));
  connect( m_mainWid, SIGNAL( sampleMode(bool) ),
           this, SLOT( setSampleStatusSLOT() ));
  connect( m_mainWid, SIGNAL( channelsEnabled( bool, bool ) ),
           this, SLOT( setChannelsEnabledSLOT( bool, bool ) ));
  
  createActions();
  createMenu();
  
  // create status bar
  //
  m_statusLabel = new QLabel( statusBar() );
  m_samplingRateLabel = new QLabel( statusBar() );
  m_samplingRateLabel->setMinimumWidth( 60 );
  m_fpsLabel = new QLabel( statusBar() );
  m_fpsLabel->setMinimumWidth( 60 );
  m_vuMeter[0] = new DsoVuMeter( 0, 255, statusBar() );
  m_vuMeter[1] = new DsoVuMeter( 1, 255, statusBar() );
  
  statusBar()->addWidget( m_statusLabel, 1, true );
  statusBar()->addWidget( m_samplingRateLabel, 0, true );
  statusBar()->addWidget( m_fpsLabel, 0, true );
  statusBar()->addWidget( m_vuMeter[0], 0, true );
  statusBar()->addWidget( m_vuMeter[1], 0, true );
  
  m_statusLabel->setText( " Halted" );
  
  m_fps = new DampedFloat;
}

MainWindow::~MainWindow()
{
  delete m_fps;
}

void
MainWindow::createActions()
{
  m_exportPngAction = new QAction( tr("Export color PNG image"),
                                   tr("Export PNG..."), 0, this );
  connect( m_exportPngAction, SIGNAL( activated() ),
           this, SLOT( exportPngSLOT() ));
  
  m_exportEpsAction = new QAction( tr("Export EPS file"),
                                   tr("Export EPS..."), 0, this );
  m_exportDataAction = new QAction( tr("Export sample data as ASCII"),
                                    tr("Export Data..."), 0, this );
  m_importDataAction = new QAction( tr("Import sample data as ASCII"),
                                    tr("Import Data..."), 0, this );
  m_printAction = new QAction( tr("Print"),
                               tr("Print..."), 0, this );
  connect( m_printAction, SIGNAL( activated() ),
           this, SLOT( printSLOT() ));
  
  m_preferencesAction = new QAction( tr("Open preferences dialog."),
                               tr("Preferences..."), 0, this );
  connect( m_preferencesAction, SIGNAL( activated() ),
           m_prefDlg, SLOT( show() ));
  m_quitAction = new QAction( tr("Quit program"),
                              tr("Quit"), CTRL+Key_Q, this );
  connect( m_quitAction, SIGNAL( activated() ),
           qApp, SLOT( quit() ));
  
  m_dsoModeAction = new QAction( tr("Switch to DSO mode"),
                                 tr("DSO"), 0, this );
  m_xyModeAction = new QAction( tr("Switch to XY mode"),
                                tr("XY"), 0, this );
  m_fftModeAction = new QAction( tr("Switch to FFT mode"),
                                 tr("FFT"), 0, this );
  m_dsoModeAction->setToggleAction( true );
  m_xyModeAction->setToggleAction( true );
  m_fftModeAction->setToggleAction( true );
  connect( m_dsoModeAction, SIGNAL( activated() ),
           this, SLOT( modeSLOT() ));
  connect( m_xyModeAction, SIGNAL( activated() ),
           this, SLOT( modeSLOT() ));
  connect( m_fftModeAction, SIGNAL( activated() ),
           this, SLOT( modeSLOT() ));
  
  m_continuousAction = new QAction( tr("Continuous data sampling (Switch off by"
                                       " going to Single Shot mode)"),
                                    tr("Continuous sampling"), 0, this );  
  m_singleShotAction = new QAction( tr("Select for single shot data sampling"),
                                    tr("Single Shot"), 0, this );
  m_continuousAction->setToggleAction( true );
  connect( m_continuousAction, SIGNAL( activated() ),
           this, SLOT( sampleModeSLOT() ));
  connect( m_singleShotAction, SIGNAL( activated() ),
           this, SLOT( sampleModeSLOT() ));
  
  m_triggerOffAction = new QAction( tr("Switch triggering on/off"),
                                    tr("On"), 0, this );
  m_triggerCh1Action = new QAction( tr("Trigger on channel 1"),
                                    tr("CH1"), 0, this );
  m_triggerCh2Action = new QAction( tr("Trigger on channel 2"),
                                    tr("CH2"), 0, this );
  m_triggerRaisingAction = new QAction( tr("Trigger on raising edge"),
                                        tr("Raising edge"), 0, this );
  m_triggerFallingAction = new QAction( tr("Trigger on falling edge"),
                                        tr("Falling edge"), 0, this );
  m_triggerOffAction->setToggleAction( true );
  m_triggerCh1Action->setToggleAction( true );
  m_triggerCh2Action->setToggleAction( true );
  m_triggerRaisingAction->setToggleAction( true );
  m_triggerFallingAction->setToggleAction( true );
  connect( m_triggerOffAction, SIGNAL( activated() ),
           this, SLOT( triggerSLOT() ));
  connect( m_triggerCh1Action, SIGNAL( activated() ),
           this, SLOT( triggerChannelSLOT() ));
  connect( m_triggerCh2Action, SIGNAL( activated() ),
           this, SLOT( triggerChannelSLOT() ));
  connect( m_triggerRaisingAction, SIGNAL( activated() ),
           this, SLOT( triggerEdgeSLOT() ));
  connect( m_triggerFallingAction, SIGNAL( activated() ),
           this, SLOT( triggerEdgeSLOT() ));

  m_rmsAction = new QAction( tr("Show true RMS value (AC only)"),
                              tr("RMS (AC)"), 0, this );
  m_frequencyAction = new QAction( tr("Show estimated frequency of measured signal"),
                              tr("Estimated frequency"), 0, this );
  m_rmsAction->setToggleAction( true );
  m_frequencyAction->setToggleAction( true );
  
  m_dotsAction = new QAction( tr("Display sample points (no interpolation)"),
                              tr("Off"), 0, this );
  m_linearAction = new QAction( tr("Linear interpolation"),
                                tr("Linear"), 0, this );
  m_linearAvAction = new QAction( tr("Linear interpolation (averaged)"),
                                tr("Linear Averaged"), 0, this );
  m_sinxAction = new QAction( tr("Sin(x)/x interpolation"),
                              tr("Sin(x)/x"), 0, this );
  m_dotsAction->setToggleAction( true );
  m_linearAction->setToggleAction( true );
  m_linearAvAction->setToggleAction( true );
  m_sinxAction->setToggleAction( true );
  connect( m_dotsAction, SIGNAL( activated() ),
           this, SLOT( interpolationSLOT() ));
  connect( m_linearAction, SIGNAL( activated() ),
           this, SLOT( interpolationSLOT() ));
  connect( m_linearAvAction, SIGNAL( activated() ),
           this, SLOT( interpolationSLOT() ));
  connect( m_sinxAction, SIGNAL( activated() ),
           this, SLOT( interpolationSLOT() ));
  
  m_timeMarkerAction = new QAction( tr("Show time marker"),
                                    tr("Time marker"), 0, this );
  m_freqMarkerAction = new QAction( tr("Show frequency marker"),
                                    tr("Frequency marker"), 0, this );
  m_amplitudeMarkerAction = new QAction( tr("Show amplitude marker"),
                                         tr("Amplitude marker"), 0, this );
  m_timeMarkerAction->setToggleAction( true );
  m_freqMarkerAction->setToggleAction( true );
  m_amplitudeMarkerAction->setToggleAction( true );
  
  m_showCh1Action = new QAction( tr("Show channel 1"),
                                 tr("Show"), 0, this );
  m_showCh2Action = new QAction( tr("Show channel 2"),
                                 tr("Show"), 0, this );
  m_showCh1Action->setToggleAction( true );
  m_showCh2Action->setToggleAction( true );

  m_voltsCh1Action[0] = new QAction( tr("Set channel 1 to 5V / division"),
                                     tr("5V / div"), 0, this );
  m_voltsCh1Action[1] = new QAction( tr("Set channel 1 to 2V / division"),
                                     tr("2V / div"), 0, this );
  m_voltsCh1Action[2] = new QAction( tr("Set channel 1 to 1V / division"),
                                     tr("1V / div"), 0, this );
  m_voltsCh1Action[3] = new QAction( tr("Set channel 1 to 0.5V / division"),
                                     tr("0.5V / div"), 0, this );
  m_voltsCh1Action[4] = new QAction( tr("Set channel 1 to 0.2V / division"),
                                     tr("0.2V / div"), 0, this );
  m_voltsCh1Action[5] = new QAction( tr("Set channel 1 to 0.1V / division"),
                                     tr("0.1V / div"), 0, this );
  m_voltsCh1Action[6] = new QAction( tr("Set channel 1 to 10mV / division"),
                                     tr("50mV / div"), 0, this );
  m_voltsCh1Action[7] = new QAction( tr("Set channel 1 to 20mV / division"),
                                     tr("20mV / div"), 0, this );
  m_voltsCh1Action[8] = new QAction( tr("Set channel 1 to 10mV / division"),
                                     tr("10mV / div"), 0, this );
  for (int i=0; i<9; ++i)
  {
    m_voltsCh1Action[i]->setToggleAction( true );
    connect( m_voltsCh1Action[i], SIGNAL( activated() ),
             this, SLOT( voltsCh1SLOT() ));
  }
  
  m_voltsCh2Action[0] = new QAction( tr("Set channel 1 to 5V / division"),
                                     tr("5V / div"), 0, this );
  m_voltsCh2Action[1] = new QAction( tr("Set channel 1 to 2V / division"),
                                     tr("2V / div"), 0, this );
  m_voltsCh2Action[2] = new QAction( tr("Set channel 1 to 1V / division"),
                                     tr("1V / div"), 0, this );
  m_voltsCh2Action[3] = new QAction( tr("Set channel 1 to 0.5V / division"),
                                     tr("0.5V / div"), 0, this );
  m_voltsCh2Action[4] = new QAction( tr("Set channel 1 to 0.2V / division"),
                                     tr("0.2V / div"), 0, this );
  m_voltsCh2Action[5] = new QAction( tr("Set channel 1 to 0.1V / division"),
                                     tr("0.1V / div"), 0, this );
  m_voltsCh2Action[6] = new QAction( tr("Set channel 1 to 10mV / division"),
                                     tr("50mV / div"), 0, this );
  m_voltsCh2Action[7] = new QAction( tr("Set channel 1 to 20mV / division"),
                                     tr("20mV / div"), 0, this );
  m_voltsCh2Action[8] = new QAction( tr("Set channel 1 to 10mV / division"),
                                     tr("10mV / div"), 0, this );
  for (int i=0; i<9; ++i)
  {
    m_voltsCh2Action[i]->setToggleAction( true );
    connect( m_voltsCh2Action[i], SIGNAL( activated() ),
             this, SLOT( voltsCh2SLOT() ));
  }

  m_timebaseAction[0] = new QAction( tr("Set timebase to 100ms / division"),
                                     tr("100ms / div"), 0, this );
  m_timebaseAction[1] = new QAction( tr("Set timebase to 50ms / division"),
                                     tr("50ms / div"), 0, this );
  m_timebaseAction[2] = new QAction( tr("Set timebase to 20ms / division"),
                                     tr("20ms / div"), 0, this );
  m_timebaseAction[3] = new QAction( tr("Set timebase to 10ms / division"),
                                     tr("10ms / div"), 0, this );
  m_timebaseAction[4] = new QAction( tr("Set timebase to 5ms / division"),
                                     tr("5ms / div"), 0, this );
  m_timebaseAction[5] = new QAction( tr("Set timebase to 2ms / division"),
                                     tr("2ms / div"), 0, this );
  m_timebaseAction[6] = new QAction( tr("Set timebase to 1ms / division"),
                                     tr("1ms / div"), 0, this );
  m_timebaseAction[7] = new QAction( tr("Set timebase to 0.5ms / division"),
                                     tr("0.5ms / div"), 0, this );
  m_timebaseAction[8] = new QAction( tr("Set timebase to 0.2ms / division"),
                                     tr("0.2ms / div"), 0, this );
  m_timebaseAction[9] = new QAction( tr("Set timebase to 0.1ms / division"),
                                     tr("0.1ms / div"), 0, this );
  m_timebaseAction[10] = new QAction( tr("Set timebase to 50s / division"),
                                      tr("50s / div"), 0, this );
  m_timebaseAction[11] = new QAction( tr("Set timebase to 20s / division"),
                                      tr("20s / div"), 0, this );
  m_timebaseAction[12] = new QAction( tr("Set timebase to 10s / division"),
                                      tr("10s / div"), 0, this );
  m_timebaseAction[13] = new QAction( tr("Set timebase to 5s / division"),
                                      tr("5s / div"), 0, this );
  m_timebaseAction[14] = new QAction( tr("Set timebase to 2s / division"),
                                      tr("2s / div"), 0, this );
  m_timebaseAction[15] = new QAction( tr("Set timebase to 1s / division"),
                                      tr("1s / div"), 0, this );
  m_timebaseAction[16] = new QAction( tr("Set timebase to 0.5s / division"),
                                      tr("0.5s / div"), 0, this );
  m_timebaseAction[17] = new QAction( tr("Set timebase to 0.2s / division"),
                                      tr("0.2s / div"), 0, this );
  m_timebaseAction[18] = new QAction( tr("Set timebase to 0.1s / division"),
                                      tr("0.1s / div"), 0, this );
  for (int i=0; i<19; ++i)
  {
    m_timebaseAction[i]->setToggleAction( true );
    connect( m_timebaseAction[i], SIGNAL( activated() ),
             this, SLOT( timebaseSLOT() ));
  }
  
  m_fftFreqAction[0] = new QAction( tr("800Hz FFT (sampling frequency 1.6kHz)"),
                                    tr("800Hz"), 0, this );
  m_fftFreqAction[1] = new QAction( tr("1.6kHz FFT (sampling frequency 3.2kHz)"),
                                    tr("1.6kHz"), 0, this );
  m_fftFreqAction[2] = new QAction( tr("4kHz FFT (sampling frequency 8kHz)"),
                                    tr("4kHz"), 0, this );
  m_fftFreqAction[3] = new QAction( tr("8kHz FFT (sampling frequency 16kHz)"),
                                    tr("8kHz"), 0, this );
  m_fftFreqAction[4] = new QAction( tr("16kHz FFT (sampling frequency 32kHz)"),
                                    tr("16kHz"), 0, this );
  m_fftFreqAction[5] = new QAction( tr("40kHz FFT (sampling frequency 80kHz)"),
                                    tr("40kHz"), 0, this );
  m_fftFreqAction[6] = new QAction( tr("80kHz FFT (sampling frequency 160kHz)"),
                                    tr("80kHz"), 0, this );
  m_fftFreqAction[7] = new QAction( tr("160kHz FFT (sampling frequency 32�Hz)"),
                                    tr("160kHz"), 0, this );
  m_fftFreqAction[8] = new QAction( tr("0.4MHz FFT (sampling frequency 0.8MHz)"),
                                    tr("0.4MHz"), 0, this );
  m_fftFreqAction[9] = new QAction( tr("0.8MHz FFT (sampling frequency 1.6MHz)"),
                                    tr("0.8Mz"), 0, this );
  m_fftFreqAction[10] = new QAction( tr("1.6MHz FFT (sampling frequency 3.2MHz)"),
                                     tr("1.6Mz"), 0, this );
  m_fftFreqAction[11] = new QAction( tr("4MHz FFT (sampling frequency 8MHz)"),
                                     tr("4Mz"), 0, this );
  m_fftFreqAction[12] = new QAction( tr("8MHz FFT (sampling frequency 16MHz)"),
                                     tr("8Mz"), 0, this );
  m_fftFreqAction[13] = new QAction( tr("16MHz FFT (sampling frequency 32MHz)"),
                                     tr("16Mz"), 0, this );
  for (int i=0; i<14; ++i)
  {
    m_fftFreqAction[i]->setToggleAction( true );
    connect( m_fftFreqAction[i], SIGNAL( activated() ),
             this, SLOT( fftFreqSLOT() ));
  }
  
  m_dcOffsetAction = new QAction( tr("Capture zero offset (For FFT). Ground input and adjust offset befor calling this."),
                                  tr("Zero offset"), 0, this );
  m_histogrammAction = new QAction( tr("Converter Histogramm. Apply sinewave to input when using this."),
                                   tr("Converter Histogramm..."), 0, this );
  connect( m_histogrammAction, SIGNAL( activated() ),
           m_mainWid, SLOT( histogramSLOT() ));
  connect( m_dcOffsetAction, SIGNAL( activated() ),
           m_mainWid, SLOT( dcOffsetSLOT() ));
  
  m_whatsThisAction = new QAction( tr("Direct help. Click on any item on the screen to get a short help text."),
                                  tr("What's this?"), Key_F1, this );
  connect( m_whatsThisAction, SIGNAL( activated() ),
           this, SLOT( whatsThisSLOT() ));
  m_copyrightAction = new QAction( tr("Show copyright information."),
                                   tr("About QtDSO..."), 0, this );
  connect( m_copyrightAction, SIGNAL( activated() ),
           this, SLOT( copyrightSLOT() ));
}
  
void
MainWindow::createMenu()
{
  QPopupMenu *popup = new QPopupMenu( menuBar() );
  m_exportPngAction->addTo( popup );
  m_exportEpsAction->addTo( popup );
  //m_exportDataAction->addTo( popup );
  popup->insertSeparator();
  //m_importDataAction->addTo( popup );
  //popup->insertSeparator();
  m_printAction->addTo( popup );
  popup->insertSeparator();
  m_preferencesAction->addTo( popup );
  popup->insertSeparator();
  m_quitAction->addTo( popup );
  
  menuBar()->insertItem( tr("&File"), popup );
  
  /*
  popup = new QPopupMenu( menuBar() );
  m_dsoModeAction->addTo( popup );
  m_xyModeAction->addTo( popup );
  m_fftModeAction->addTo( popup );
  popup->insertSeparator();
  m_continuousAction->addTo( popup );
  m_singleShotAction->addTo( popup );

  menuBar()->insertItem( tr("&Mode"), popup );
  
  popup = new QPopupMenu( menuBar() );
  m_triggerOffAction->addTo( popup );
  m_triggerCh1Action->addTo( popup );
  m_triggerCh2Action->addTo( popup );
  popup->insertSeparator();
  m_triggerRaisingAction->addTo( popup );
  m_triggerFallingAction->addTo( popup );

  menuBar()->insertItem( tr("&Trigger"), popup );

  m_timebasePopup = new QPopupMenu( menuBar() );
  for (int i=0; i<19; ++i)
  {
    m_timebaseAction[i]->addTo( m_timebasePopup );
  }
    
  menuBar()->insertItem( tr("&Timebase"), m_timebasePopup );
  
  m_fftPopup = new QPopupMenu( menuBar() );
  QPopupMenu *fftSize = new QPopupMenu( m_fftPopup );
  fftSize->setCheckable( true );
  fftSize->insertItem( "128", 0 );
  fftSize->insertItem( "256", 1 );
  fftSize->insertItem( "512", 2 );
  fftSize->insertItem( "1024", 3 );
  fftSize->insertItem( "2048", 4 );
  fftSize->insertItem( "4096", 5 );
  m_fftPopup->insertItem( tr("Size"), fftSize );
  m_fftPopup->insertSeparator();
  QPopupMenu *fftWindow = new QPopupMenu( m_fftPopup );
  fftWindow->setCheckable( true );
  fftWindow->insertItem( tr("Rectangle"), 0 );
  fftWindow->insertItem( tr("Bartlett"), 0 );
  fftWindow->insertItem( tr("Bartlett-Hann"), 0 );
  fftWindow->insertItem( tr("Blackman"), 0 );
  fftWindow->insertItem( tr("Blackman-Harris"), 0 );
  fftWindow->insertItem( tr("Gaussian"), 0 );
  fftWindow->insertItem( tr("Hamming"), 0 );
  fftWindow->insertItem( tr("Hann"), 0 );
  fftWindow->insertItem( tr("Welch"), 0 );
  fftWindow->insertItem( tr("Kaiser-Bessel"), 0 );
  fftWindow->insertItem( tr("Flat-Top"), 0 );  
  m_fftPopup->insertItem( tr("Window"), fftWindow );
  m_fftPopup->insertSeparator();
  for (int i=0; i<14; ++i)
  {
    m_fftFreqAction[i]->addTo( m_fftPopup );
  }
  
  menuBar()->insertItem( tr("&FFT"), m_fftPopup );

  m_ch1Popup = new QPopupMenu( menuBar() );
  m_showCh1Action->addTo( m_ch1Popup );
  m_ch1Popup->insertSeparator();
  for (int i=0; i<9; ++i)
  {
    m_voltsCh1Action[i]->addTo( m_ch1Popup );
  }
  m_ch1Popup->insertSeparator();
  m_ch1Popup->insertItem( tr("Probe x1"), 43 );
  m_ch1Popup->insertItem( tr("Probe x10"), 42 );
  
  menuBar()->insertItem( tr("CH&1"), m_ch1Popup );
  
  m_ch2Popup = new QPopupMenu( menuBar() );
  m_showCh2Action->addTo( m_ch2Popup );
  m_ch2Popup->insertSeparator();
  for (int i=0; i<9; ++i)
  {
    m_voltsCh2Action[i]->addTo( m_ch2Popup );
  }
  m_ch2Popup->insertSeparator();
  m_ch2Popup->insertItem( tr("Probe x1"), 43 );
  m_ch2Popup->insertItem( tr("Probe x10"), 42 );
  
  menuBar()->insertItem( tr("CH&2"), m_ch2Popup );
  
       
  m_displayPopup = new QPopupMenu( menuBar() );
  m_frameRatePopup = new QPopupMenu( m_displayPopup );
  m_frameRatePopup->insertItem( tr("5 FPS"), 0 );
  m_frameRatePopup->insertItem( tr("10 FPS"), 1 );
  m_frameRatePopup->insertItem( tr("20 FPS"), 2 );
  m_frameRatePopup->insertItem( tr("25 FPS"), 3 );
  m_frameRatePopup->insertItem( tr("30 FPS"), 4 );
  m_frameRatePopup->setCheckable( true );
  m_frameRatePopup->setItemChecked( 5, true );
  connect( m_frameRatePopup, SIGNAL( activated(int) ),
           this, SLOT( frameRateSLOT(int) ));
  m_displayPopup->insertItem( tr("Maximum Frame Rate"), m_frameRatePopup );
  m_displayPopup->insertSeparator();
  m_rmsAction->addTo( m_displayPopup );
  m_frequencyAction->addTo( m_displayPopup );
  m_displayPopup->insertSeparator();
  popup = new QPopupMenu( menuBar() );
  m_dotsAction->addTo( popup );
  m_linearAction->addTo( popup );
  m_linearAvAction->addTo( popup );
  m_sinxAction->addTo( popup );
  m_displayPopup->insertItem( tr("Interpolation"), popup );
  
  menuBar()->insertItem( tr("&Display"), m_displayPopup );
*/  
  popup = new QPopupMenu( menuBar() );
  m_dcOffsetAction->addTo( popup );
  m_histogrammAction->addTo( popup );
  popup->insertSeparator();
  m_timeMarkerAction->addTo( popup );
  m_amplitudeMarkerAction->addTo( popup );
  
  menuBar()->insertItem( tr("&Tools"), popup ); 
  
  menuBar()->insertSeparator();
  
  popup = new QPopupMenu( menuBar() );
  m_whatsThisAction->addTo( popup );
  m_copyrightAction->addTo( popup );
  
  menuBar()->insertItem( tr("&Help"), popup );
  
}

void
MainWindow::voltsCh1SLOT()
{
  for (int i=0; i<9; ++i)
  {
    m_voltsCh1Action[i]->setOn( sender() == m_voltsCh1Action[i] );
  }
}

void
MainWindow::voltsCh2SLOT()
{
  for (int i=0; i<9; ++i)
  {
    m_voltsCh2Action[i]->setOn( sender() == m_voltsCh2Action[i] );
  }
}

void
MainWindow::timebaseSLOT()
{
  for (int i=0; i<19; ++i)
  {
    m_timebaseAction[i]->setOn( sender() == m_timebaseAction[i] );
  }
}

void
MainWindow::fftFreqSLOT()
{
  for (int i=0; i<14; ++i)
  {
    m_fftFreqAction[i]->setOn( sender() == m_fftFreqAction[i] );
  }
}

void
MainWindow::interpolationSLOT()
{
  m_dotsAction->setOn( sender() == m_dotsAction );
  m_linearAction->setOn( sender() == m_linearAction );
  m_linearAvAction->setOn( sender() == m_linearAvAction );
  m_sinxAction->setOn( sender() == m_sinxAction );
}

void
MainWindow::triggerSLOT()
{
}

void
MainWindow::triggerChannelSLOT()
{
  m_triggerCh1Action->setOn( sender() == m_triggerCh1Action );
  m_triggerCh2Action->setOn( sender() == m_triggerCh2Action );
}

void
MainWindow::triggerEdgeSLOT()
{
  m_triggerRaisingAction->setOn( sender() == m_triggerRaisingAction );
  m_triggerFallingAction->setOn( sender() == m_triggerFallingAction );
}

void
MainWindow::modeSLOT()
{
  m_dsoModeAction->setOn( sender() == m_dsoModeAction );
  m_xyModeAction->setOn( sender() == m_xyModeAction );
  m_fftModeAction->setOn( sender() == m_fftModeAction );
  
  if (sender() == m_dsoModeAction)
  {
    setCaption( tr("QtDSO: Digital storage oscilloscope") );
  }
  if (sender() == m_xyModeAction)
  {
    setCaption( tr("QtDSO: Digital storage oscilloscope (XY mode)") );
  }
  if (sender() == m_fftModeAction)
  {
    setCaption( tr("QtDSO: Spectrum analyzer") );
  }
}


void
MainWindow::sampleModeSLOT()
{
  m_singleShotAction->setEnabled( !m_continuousAction->isOn() );
}

void
MainWindow::whatsThisSLOT()
{
  QWhatsThis::enterWhatsThisMode();
}
  
void
MainWindow::copyrightSLOT()
{
  if (!m_about)
  {
    m_about = new UIAbout( this );
  }
  
  m_about->show();
}

void
MainWindow::frameRateSLOT( int id )
{
  for (int i=0; i<10; ++i)
  {
    m_frameRatePopup->setItemChecked( i, false );
  }
  
  m_frameRatePopup->setItemChecked( id, true );
  
  m_mainWid->setFrameRateSLOT( id );
}

void
MainWindow::applyPrefsSLOT()
{
  m_mainWid->setFftBufferLengthSLOT( m_prefDlg->fftBufferLength() );  
  m_mainWid->setDisplayFontSLOT( m_prefDlg->displayFont() );  
  m_mainWid->setFftDrawModeSLOT( 0, m_prefDlg->fftDrawMode( 0 ) );  
  m_mainWid->setFftDrawModeSLOT( 1, m_prefDlg->fftDrawMode( 1 ) ); 
  m_mainWid->setDeviceSLOT( m_prefDlg->device() ); 
  
  m_mainWid->applyPrefsSLOT();
}

void
MainWindow::dynamicRangeSLOT( int maxVal,
                              int min0, int max0, 
                              int min1, int max1 )
{
  m_vuMeter[0]->setValuesSLOT( maxVal, min0, max0 );
  m_vuMeter[1]->setValuesSLOT( maxVal, min1, max1 );
}

void
MainWindow::exportPngSLOT()
{
  static QString exportDir = QDir::homeDirPath();
  
  QImage img = m_mainWid->snapshot();
  // crop border
  img = img.copy( 5, 5, img.width()-10, img.height()-10 );
  // convert to 8 bit
  img = img.convertDepth( 8, Qt::ThresholdDither | Qt::AvoidDither );
   
  QString filename = 
      QFileDialog::getSaveFileName( exportDir,
                                    "Images (*.png)",
                                    this, 0, tr("Save PNG image") );
  
  if (!filename.isNull())
  {
    QFile tst( filename );
    
    if (tst.exists())
    {
      if( QMessageBox::warning( this, "QtDSO: Save PNG image",
        "The selected file already exists!\nDo you want to"
        " overwrite the file?",
        "Save anyway",
        "Cancel", 0, 0, 1 ) == 1)
      {
        return;
      }
    }

    img.save( filename, "PNG" );
  }
}

void
MainWindow::printSLOT()
{
  QPrinter prt;
  
  if (prt.setup( this ))
  {
    m_mainWid->print( &prt );
  }
}

void
MainWindow::fpsSLOT( float fps )
{
  static QColorGroup cg = colorGroup();
  static QColorGroup *darkCg = 0;
  
  if (!darkCg)
  {
    darkCg = new QColorGroup( cg );
    
    darkCg->setColor( QColorGroup::Background, cg.background().dark(120) );
  }
      
  m_fps->addValue( fps );
  
  QString str;
  str.sprintf( " %d FPS ", (int)qRound( m_fps->value() ));
  
  
  if (m_fps->value() < m_mainWid->frameRate())
  {
    m_fpsLabel->setPalette( QPalette( *darkCg, *darkCg, *darkCg ) );
  }
  else
  {
    m_fpsLabel->setPalette( QPalette( cg, cg, cg ) );
  }
    
  m_fpsLabel->setText( str );
}

void
MainWindow::resetFpsSLOT()
{
  m_fps->reset();
  
  QString str = " ";
  str += DsoWid::floatValueString( m_mainWid->samplingFrequency(),
                                   DsoWid::Frequency );
  str += " ";
  m_samplingRateLabel->setText( str );
}

void
MainWindow::triggerOkSLOT( bool ok )
{
  if (ok)
  {
    if (m_mainWid->running())
    {
      m_statusLabel->setText( tr( " Sampling" ) );
    }
    else
    {
      m_statusLabel->setText( tr( " Halted" ) );
    }
    m_waitingTrigger = false;
  }
  else
  {
    QString stat = tr( " Waiting for trigger channel " );
    
    if (m_triggerCh1Action->isOn())
    {
      stat += tr( "1..." );
    }
    else
    {
      stat += tr( "2..." );
    }
      
    m_statusLabel->setText( stat );
    if (!m_waitingTrigger)
    {
      qApp->beep();
    }
    m_waitingTrigger = true;
  }
}

void
MainWindow::setSampleStatusSLOT()
{
  if (m_waitingTrigger)
  {
    m_statusLabel->setText( " Waiting for trigger..." );
  }
  else
  {
    if (m_mainWid->running())
    {
      m_statusLabel->setText( " Sampling" );
    }
    else
    {
      m_statusLabel->setText( " Halted" );
    }
    m_waitingTrigger = false;
  }
}
    
void
MainWindow::setChannelsEnabledSLOT( bool ch1, bool ch2 )
{
  m_vuMeter[0]->setEnabled( ch1 );
  m_vuMeter[1]->setEnabled( ch2 );
}

