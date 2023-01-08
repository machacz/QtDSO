//======================================================================
// File:		mainwindow.h
// Author:	Matthias Toussaint
// Created:	Sun Jun  9 23:22:57 CEST 2002
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

#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include <qmainwindow.h>

class MainWid;
class QAction;
class QPopupMenu;
class UIAbout;
class PrefDlg;
class DsoVuMeter;
class QLabel;
class DampedFloat;

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  MainWindow( QWidget *parent=0, const char *name=0 );
  virtual ~MainWindow();
  
protected slots:
  void whatsThisSLOT();
  void voltsCh1SLOT();
  void voltsCh2SLOT();
  void interpolationSLOT();
  void modeSLOT();
  void timebaseSLOT();
  void triggerSLOT();
  void triggerChannelSLOT();
  void triggerEdgeSLOT();
  void sampleModeSLOT();
  void fftFreqSLOT();
  void copyrightSLOT();
  void frameRateSLOT( int );
  void applyPrefsSLOT();
  void dynamicRangeSLOT( int, int, int, int, int );
  void fpsSLOT( float );
  void exportPngSLOT();
  void printSLOT();
  void triggerOkSLOT( bool );
  void resetFpsSLOT();
  void setSampleStatusSLOT();
  void setChannelsEnabledSLOT( bool, bool );
  
protected:
  MainWid     *m_mainWid;
  UIAbout     *m_about;
  PrefDlg     *m_prefDlg;
  DampedFloat *m_fps;
  
  QAction *m_exportPngAction;
  QAction *m_exportEpsAction;
  QAction *m_exportDataAction;
  QAction *m_importDataAction;
  QAction *m_printAction;
  QAction *m_preferencesAction;
  QAction *m_quitAction;
  
  QAction *m_dsoModeAction;
  QAction *m_xyModeAction;
  QAction *m_fftModeAction;
  
  QAction *m_continuousAction;
  QAction *m_singleShotAction;
  
  QAction *m_triggerOffAction;
  QAction *m_triggerCh1Action;
  QAction *m_triggerCh2Action;  
  QAction *m_triggerRaisingAction;
  QAction *m_triggerFallingAction;
  
  QAction *m_dotsAction;  
  QAction *m_linearAction;
  QAction *m_linearAvAction;
  QAction *m_sinxAction;

  QAction *m_showCh1Action;  
  QAction *m_showCh2Action;
  
  QAction *m_voltsCh1Action[9];
  QAction *m_voltsCh2Action[9];
  
  QAction *m_timebaseAction[19];
  QAction *m_fftFreqAction[14];

  QAction *m_rmsAction;  
  QAction *m_frequencyAction;  
  
  QAction *m_timeMarkerAction;  
  QAction *m_freqMarkerAction;
  QAction *m_amplitudeMarkerAction;
  
  QAction *m_dcOffsetAction;
  QAction *m_histogrammAction;
  
  QAction *m_whatsThisAction;
  QAction *m_copyrightAction;
  
  QPopupMenu *m_ch1Popup;
  QPopupMenu *m_ch2Popup;
  QPopupMenu *m_fftPopup;
  QPopupMenu *m_timebasePopup;
  QPopupMenu *m_displayPopup;
  QPopupMenu *m_frameRatePopup;
  
  QLabel     *m_statusLabel;
  QLabel     *m_samplingRateLabel;
  QLabel     *m_fpsLabel;
  
  DsoVuMeter *m_vuMeter[2];
  
  bool m_waitingTrigger;
  
  void createActions();
  void createMenu();
  
};

#endif // MAINWINDOW_HH
