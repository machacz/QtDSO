//======================================================================
// File:		dsowid.h
// Author:	Matthias Toussaint
// Created:	Sun Jun  9 22:27:14 CEST 2002
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

#ifndef DSOWID_HH
#define DSOWID_HH

#include <qwidget.h>
#include <qpixmap.h>
#include <type.h>

#include <stdlib.h>

#include <fftw3.h>

#include <dso.h>


#define MAX_FFT_BUFFER_LENGTH 10000

class DampedFloat;
class ConverterHistogramDlg;
class SimulatorWid;

class DsoWid : public QWidget
{
  Q_OBJECT
public:
  enum MathMode
  {
    NoMath,
    Add,
    Sub,
    Magnitude
  };
  
  enum MouseMode
  {
    Time1,
    Time2,
    Time3,
    Amplitude1,
    Amplitude2,
    Freq,
    Db1,
    Db2,
    None
  };
    
  enum TimeMarkerMode
  {
    Off = 0,
    TimeMarker,
    RatioMarker
  };
    
  enum Mode
  {
    DSO = 0,
    XY,
    FFT,
    NumMode
  };
    
  enum FFTType
  {
    Power = 0,
    Phase
  };
    
  enum FFTGridMode
  {
    Lin = 0,
    Log
  };
    
  enum FFTWindow
  {
    Rectangle=0,
    Bartlett,
    BartlettHann,
    Blackman,
    BlackmanHarris,
    Gaussian,
    Hamming,
    Hann,
    Welch,
    KaiserBessel,
    FlatTop,
    NumFFTWindows
  };
    
  enum Interpolation
  {
    Dots=0,
    Linear,
    LinearAverage,
    SinX
  };
    
  enum FFTDisplayMode
  {
    RT = 0,
    Minimum,
    Average,
    Maximum    
  };
    
  enum FFTDrawMode
  {
    Line = 0,
    Needles,
    Points
  };
    
  enum FrameRate
  {
    FR5 = 0,
    FR10,
    FR20,
    FR25,
    FR30
  };
    
  enum ValueType
  {
    Frequency = 0,
    Time,
    Voltage,
    Ratio
  };
    
  DsoWid( QWidget *parent=0, const char *name=0 );
  virtual ~DsoWid();

  static QColor bgColor;
  static QColor gridColor;
  static QColor borderColor;
  static QColor chColor[3];
  static QColor chChangedColor[3];
  static QColor addColor;
  static QColor subColor;
  static QColor timeColor;
  static QColor amplitudeColor;
  static int    s_lineWidth;
  static int    s_lineWidthFft;
    
  int fftZoom() const { return m_fftZoom; }
  int step() const { return m_dso->step(); }
  int fftSize() const { return m_fftSize; }
  int sinXLength() const { return m_sinXLength; }
  void setFftBufferLength( int );
  
  QImage snapshot();
  void print( QPrinter *prt );
  
  bool selectModel( Dso::Model );

  static QString floatValueString( float value, ValueType type, int prec=3 );
  float samplingFrequency() const { return m_dso->samplingFrequency(); }
  
  int frameRate() const;
  
  Dso *dso() const { return m_dso; }
  
signals:
  void dynamicRange( int, int, int, int, int );
  void fps( float );
  void triggerOk( bool );
  void scrollMe( int );
  void dsoChanged();
  
public slots:
  void setXOffSLOT( int );
  void setMathOffSLOT(int );
  void setShowAmplitudeMarkerSLOT( bool );
  void setShowTimeMarkerSLOT( TimeMarkerMode );
  void setMathModeSLOT( int );
  void setShowChannel1SLOT( bool );
  void setShowChannel2SLOT( bool );
  void setModeSLOT( DsoWid::Mode );
  void setTimeBaseSLOT( Dso::TimeBase );
  void setVoltsDivSLOT( int, Dso::VoltsDiv );
  void setTriggerLevelSLOT( int );
  void setTriggerSLOT( bool );
  void setTriggerRaisingSLOT( bool );
  void setTriggerChannelSLOT( int );
  void setTriggerExternalSLOT( bool );
  void setDrawTriggerLineSLOT( bool );
  void setStretchSLOT( int, float );
  void setMathStretchSLOT( float );
  void setStepSLOT( int );
  void setFftWindowSLOT( int );
  void setFftFreqSLOT( Dso::TimeBase );
  void setFftZoomSLOT( int );
  void setFftPostMagSLOT( float );
  void setFftSizeSLOT( int );
  void setFftSizeFreeSLOT( int );
  void setFftDisplaySLOT( int );
  void setFftShowPeaksSLOT( bool );
  void setFftTypeSLOT( int );
  void setInterpolSLOT( DsoWid::Interpolation );
  void setFftWindowNameSLOT( const QString & );
  void setYOffsetSLOT( int, int );
  void setDcOffsetSLOT( int, int );
  void setFrameRateSLOT( FrameRate );
  void setContinuousSamplingSLOT( bool );
  void singleShotSLOT();
  void flushFftAverageBufferSLOT();
  void setFftDrawModeSLOT( int channel, FFTDrawMode );
  void setProbeSLOT( int channel, float probe );
  void applyPrefsSLOT();
  void setChannelOpSLOT( bool invCh1, bool absCh1, bool invCh2, bool absCh2 );
  void setFftScaleSLOT( int );
  void fastAcqSLOT( bool );
  void setTriggerStabilizerSLOT( bool );
  void equivalentSamplingSLOT( bool );
  void histogramSLOT();
  void dcOffsetSLOT();
  void resetEnvelopeSLOT();
  void setEnvelopeSLOT( bool );
  
protected:
  Dso           *m_dso;
  QPixmap        m_buffer;
  int            m_fh;
  int            m_tw;
  int            m_xOff;
  int            m_yOff[3];
  int           *m_data[3];
  int            m_dcOffset[3];
  float         *m_fftOut;
  float         *m_fftOutBuffer[2][MAX_FFT_BUFFER_LENGTH];
  float         *m_fftOutAv[2];
  int            m_fftOutAvPointer[2];
  int            m_fftOutAvCounter[2];
  fftw_complex  *m_fftwIn;
  fftw_complex  *m_fftwOut;
  QPointArray   *m_arr;
  QPointArray   *m_arr2;
  QPointArray   *m_arr3;
  int            m_timeMarker[3];
  int            m_freqMarker;
  int            m_amplitudeMarker[2];
  int            m_dbMarker[2];
  TimeMarkerMode m_timeMarkerMode;
  bool           m_showAmplitudeMarker;
  MathMode       m_mathMode;
  bool           m_showChannel[3];
  MouseMode      m_moving;
  int            m_x0;
  int            m_y0;
  Mode           m_mode;
  Dso::TimeBase  m_tb[NumMode];
  bool           m_drawTriggerLine;
  float          m_stretch[3];
  int            m_legWidth;
  float          m_window[NumFFTWindows-1][FFT_SIZE_MAX];
  int            m_fftWindow;
  int            m_fftZoom;
  float          m_fftPostMag;
  int            m_x;
  int            m_y;
  int            m_fftSize;
  bool           m_fftShowPeaks;
  FFTType        m_fftType;
  fftw_plan      m_fftw_plan;
  float         *m_sinXRegister;
  float         *m_sinXCoeff;
  int            m_sinXLength;
  int            m_sinXPointer;
  Interpolation  m_interpolation;
  QString        m_windowName;
  FFTDisplayMode m_fftDisplayMode;
  int            m_updateTimer;
  FrameRate      m_frameRate;
  bool           m_continuousSampling;
  float         *m_fftOutSum[2];
  char          *m_wisdomString;
  int            m_fftAverageBufferLength;
  FFTDrawMode    m_fftDrawMode[2];
  FFTGridMode    m_fftGridMode;
  float          m_maxFftVal;
  float          m_probe[3];
  DampedFloat   *m_vpp[2];
  DampedFloat   *m_rms[2];
  DampedFloat   *m_freq[2];
  int            m_averageSample[2];
  int            m_minSample[2];
  int            m_maxSample[2];
  int           *m_envelopeMin[3];
  int           *m_envelopeMax[3];
  bool           m_drawMeasuredVolts;
  bool           m_hasData;
  bool           m_invCh[3];
  bool           m_absCh[3];
  ButtonState    m_mouseButton;
  int            m_updateTime;
  int            m_comeAgainTimer;
  int            m_realFrameRate;
  SimulatorWid  *m_simWid;
  Dso::Model     m_model;
  bool           m_showEnvelope;
  float          m_dataStretch;
  int            m_triggerOffset;
  bool           m_triggerStabilizer;
  
  ConverterHistogramDlg *m_converterHistogram;
  
  void resizeEvent( QResizeEvent * );
  void paintEvent( QPaintEvent * );
  void timerEvent( QTimerEvent * );
  void mousePressEvent( QMouseEvent * );
  void mouseMoveEvent( QMouseEvent * );
  void mouseReleaseEvent( QMouseEvent *ev );  
  
  void drawGrid( QPainter *, bool );
  void drawFFTGrid( QPainter *, bool );
  
  void drawChannel( QPainter *, int, bool );
  void drawEnvelope( QPainter *, int, bool );
  
  void drawTimeMarker( QPainter *, bool );
  void drawFreqMarker( QPainter *, bool );
  void drawAmplitudeMarker( QPainter *, bool );
  void drawDbMarker( QPainter *, bool );
  
  void drawDiv( QPainter *, int, bool );
  
  void drawDSO( QPainter *, bool );
  void drawXY( QPainter *, bool );
  void drawFFT( QPainter *, bool );
  
  void doMath();
  void fft( int );
  
  void fillFFTWindows();
  float computeFreqScaleStep();
  float computeFreqScaleStepLog();
  void showFftPeaks( QPainter *, float );
  
  void createSinXInterpol();
  inline float sinXFilter( float );
  
  void readFromDso();
  
  float frequency( int, bool * ) const;
  float rms( int channel );
  
  void paint( QPaintDevice *, bool bw=false );
  float volts( int channel, float value ) const;
  
  void init();
  void internalAdjustSize();
  int mathSample( int, int ) const;

  void setDsoAcqLength();
  
  void drawPoints( QPainter *p, int channel, int numPoints, bool print ) const;
  void drawLine( QPainter *p, int channel, int numPoints, bool print ) const;

};

inline float
DsoWid::sinXFilter( float sample )
{
  // feed into filter
  //
  m_sinXRegister[m_sinXPointer] = sample;
  
  // compute sum
  //
  float sum = 0.0;
  
  for (int i=0, j=m_sinXPointer; i<m_sinXLength; ++i, ++j)
  {
    sum += m_sinXRegister[j % m_sinXLength] * m_sinXCoeff[i];
  }
  
  ++m_sinXPointer;
  m_sinXPointer %= m_sinXLength;
  
  return sum;
}

inline float
DsoWid::volts( int channel, float value ) const
{
  return value / 32. * m_dso->voltsDiv( channel ) * 255.0 / (float)m_dso->maxValue();
}

inline int
DsoWid::mathSample( int channel, int i ) const
{
  if (m_absCh[channel])
  {
    if (m_invCh[channel])
    {
      return m_dcOffset[channel] - abs( m_data[channel][i]  - m_dcOffset[channel]);
    }
    else
    {
      return abs( m_data[channel][i] - m_dcOffset[channel] );
    }
  }
  else if (m_invCh[channel])
  {
    return ( m_dcOffset[channel] - m_data[channel][i] );
  }

  return m_data[channel][i] - m_dcOffset[channel];
}

#endif // DSOWID_HH
