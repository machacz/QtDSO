//======================================================================
// File:		mainwid.h
// Author:	Matthias Toussaint
// Created:	Sun Jun  9 22:29:05 CEST 2002
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

#ifndef MAINWID_HH
#define MAINWID_HH

#include <uimainwid.h>
#include <dsowid.h>
#include <type.h>

#include <qimage.h>

class MainWid : public UIMainWid
{
  Q_OBJECT
public:
  MainWid( QWidget *parent=0, const char *name=0 );
  virtual ~MainWid();

  QImage snapshot() { return ui_dsoWid->snapshot(); }
  void print( QPrinter *prt ) { ui_dsoWid->print( prt ); }
  
  float samplingFrequency() const { return ui_dsoWid->samplingFrequency(); }
  bool running() const;
  
  int frameRate() const;
  
signals:
  void dynamicRange( int, int, int, int, int );
  void fps( float );
  void triggerOk( bool );
  void resetFps();
  void sampleMode( bool );
  void channelsEnabled( bool, bool );
  void triggerSource( int );
  
public slots:
  void setFrameRateSLOT( int );
  void singleShotSLOT();
  void setFftBufferLengthSLOT( int );
  void setDisplayFontSLOT( const QFont & );
  void setFftDrawModeSLOT( int channel, DsoWid::FFTDrawMode );
  void setChEnableColorSLOT();
  void applyPrefsSLOT();
  void histogramSLOT();
  void dcOffsetSLOT();
  void setDeviceSLOT( int );
  
protected slots:
  void modeSLOT( int );
  void centerMathSLOT();
  void resetMathSLOT();
  void mathOffsetChangedSLOT( int );
  void mathStretchChangedSLOT( int );
  void ch1VoltsDivChangedSLOT( int );
  void ch2VoltsDivChangedSLOT( int );
  void timeBaseChangedSLOT( int );
  void triggerPressSLOT();
  void triggerReleaseSLOT();
  void stretchCh1SLOT( int );
  void stretchCh2SLOT( int );
  void dcOffsetCh1SLOT( int );
  void dcOffsetCh2SLOT( int );
  void resetStretchCh1SLOT();
  void resetStretchCh2SLOT();
  void resetOffsetCh1SLOT();
  void resetOffsetCh2SLOT();
  void resetDCOffsetCh1SLOT();
  void resetDCOffsetCh2SLOT();
  void probeCh1SLOT( int );
  void probeCh2SLOT( int );
  void fftFreqRangeChangedSLOT( int );
  void fftZoomSLOT( int );
  void fftPostMagSLOT( int );
  void fftSizeSLOT( int );
  void interpolSLOT( int );
  void fftWindowSLOT();
  void yPosSLOT( int );
  void setTriggerChannelSLOT();
  void setTriggerEdgeSLOT();
  void setShowTimeMarkerSLOT( bool );
  void setShowTimeMarker2SLOT( bool );
  void channelOpSLOT();
  void setContinuousSamplingSLOT( bool );
  void setEnvelopeSLOT( bool );
  void resetEnvelopeSLOT();
  void requestCapabilitiesSLOT();
  void couplingModeCh1SLOT( int mode );
  void couplingModeCh2SLOT( int mode );
  
protected:
  Q_UINT8 *m_data[2];
  double   m_fac[2];
  int      m_voltsDivId[2];
  int      m_yPos[2][2];
  int      m_yStretch[2][2];
  int      m_mode;
  
  void voltsDivChanged( int channel, int id );
  void setFftZoomRange();
  void setDsoXRange();
  void setPosSlider();
  void savePosSlider();
  void fontChange( const QFont & );
  
};

#endif // MAINWID_HH
