//======================================================================
// File:		dsowid.cpp
// Author:	Matthias Toussaint
// Created:	Sun Jun  9 22:28:27 CEST 2002
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

#include <unistd.h>

#include <qdatetime.h>
#include <qdrawutil.h>
#include <qfile.h>
#include <qfile.h>
#include <qimage.h>
#include <qpainter.h>
#include <qprinter.h>
#include <qtextstream.h>

#include <dampedfloat.h>
#include <dsowid.h>
#include <math.h>
#include <pcs64i.h>
#include <simulator.h>
#include <simulatorwid.h>
#include <converterhistogramdlg.h>

#include <iostream>

#include <algorithm>

//#include <triangle_left.xbm>
//#include <triangle_right.xbm>
//#include <triangle_bottom.xbm>

QColor DsoWid::bgColor = Qt::black;
QColor DsoWid::borderColor = Qt::white;
QColor DsoWid::gridColor = Qt::darkGray;
QColor DsoWid::chColor[3] = { QColor( 50, 255, 50 ), QColor( 255, 255, 50 ), QColor(0,0,0) };
QColor DsoWid::chChangedColor[3] = { QColor( 255, 150, 255 ), QColor( 150, 255, 255 ), QColor(0,0,0) };
QColor DsoWid::addColor = QColor( 180, 180, 255 );
QColor DsoWid::subColor = QColor( 255, 180, 180 );
QColor DsoWid::amplitudeColor = Qt::cyan;
QColor DsoWid::timeColor = Qt::cyan;
int    DsoWid::s_lineWidth = 3;
int    DsoWid::s_lineWidthFft = 0;
                 
DsoWid::DsoWid( QWidget *parent, const char *name ) :
  QWidget( parent, name ),
  m_dso( 0 ),
  m_xOff( 768 ),
  m_timeMarkerMode( Off ),
  m_showAmplitudeMarker( false ),
  m_mathMode( NoMath ),
  m_moving( DsoWid::None ),
  m_mode( DsoWid::DSO ),
  m_drawTriggerLine( false ),
  m_fftWindow( 0 ),
  m_fftZoom( 2 ),
  m_fftPostMag( 1. ),
  m_fftShowPeaks( false ),
  m_fftType( DsoWid::Power ),
  m_sinXRegister( 0 ),
  m_sinXCoeff( 0 ),
  m_sinXLength( 0 ),
  m_sinXPointer( 0 ),
  m_interpolation( Linear ),
  m_fftDisplayMode( RT ),
  m_updateTimer( -1 ),
  m_frameRate( FR30 ),
  m_continuousSampling( false ),
  m_wisdomString( 0 ),
  m_fftAverageBufferLength( 100 ),
  m_fftGridMode( Lin ),
  m_drawMeasuredVolts( true ),
  m_hasData( false ),
  m_comeAgainTimer( -1 ),
  m_realFrameRate( 30 ),
  m_simWid( 0 ),
  m_model( Dso::Unknown ),
  m_showEnvelope( false ),
  m_dataStretch( 1.0 ),
  m_triggerStabilizer( false ),
  m_converterHistogram( 0 )
{
  m_fftDrawMode[0] = m_fftDrawMode[1] = Line;
  m_data[0] = m_data[1] = m_data[2] = 0;
  m_arr = m_arr2 = m_arr3 = 0;
  
  m_invCh[2] = m_absCh[2] = false;
  
  setChannelOpSLOT( false, false, false, false );
  
  // load wisdom if avail
  //
/*  QFile file( "/home/mt/qtdso_wisdom" );
  
  if (file.open( IO_ReadOnly ))
  {
    QString wisdomString;
    
    QTextStream ts( &file );
    ts >> wisdomString;
    file.close();
    
    fftw_import_wisdom_from_string( wisdomString.latin1() );
  }
  else
  {
    m_fftw_plan = fftw_create_plan( 4096, FFTW_FORWARD,
                                    FFTW_MEASURE | FFTW_USE_WISDOM );
  }
  */
  
  m_showChannel[0] = m_showChannel[1] = true;
  m_showChannel[2] = false;  // math
  m_stretch[0] = m_stretch[1] = m_stretch[2] = 1.0;  
  m_tb[DSO] = m_tb[XY] = m_tb[FFT] = Dso::TB1ms;  
  m_dcOffset[0] = m_dcOffset[1] = m_dcOffset[2] = 0;
  
  m_timeMarker[0] = 16;
  m_timeMarker[1] = 240;
  m_timeMarker[2] = 464;
  m_freqMarker = 64;
  m_amplitudeMarker[0] = 16;
  m_amplitudeMarker[1] = 240;
  m_dbMarker[0] = 16;
  m_dbMarker[1] = 240;
  m_yOff[0] = -80;
  m_yOff[1] = 80;
  m_yOff[2] = 0;
  
  setBackgroundMode( QWidget::NoBackground );
      
  m_fftOut = new float [FFT_SIZE_MAX/2];
  
  m_fftOutAvPointer[0] = 0;
  m_fftOutAvPointer[1] = 0;
  
  m_fftOutSum[0] = new float [FFT_SIZE_MAX/2];
  m_fftOutSum[1] = new float [FFT_SIZE_MAX/2];
  
  m_fftwIn  = (fftw_complex *)fftw_malloc( sizeof(fftw_complex)*FFT_SIZE_MAX );
  m_fftwOut = (fftw_complex *)fftw_malloc( sizeof(fftw_complex)*FFT_SIZE_MAX );
    
  m_fftw_plan = fftw_plan_dft_1d( 4096, m_fftwIn, m_fftwOut,
                                  FFTW_FORWARD, FFTW_ESTIMATE );
  
  for (int i=0; i<MAX_FFT_BUFFER_LENGTH; ++i)
  {
    m_fftOutBuffer[0][i] = 0;
    m_fftOutBuffer[1][i] = 0;
  }

  setFftBufferLength( 100 );
  internalAdjustSize();
  m_probe[0] = m_probe[1] = m_probe[2] = 1;
  setDcOffsetSLOT( 0, 128 );
  setDcOffsetSLOT( 1, 128 );
  setDcOffsetSLOT( 2, 126 );
  m_vpp[0] = new DampedFloat;
  m_vpp[1] = new DampedFloat;
  m_rms[0] = new DampedFloat;
  m_rms[1] = new DampedFloat;
  m_freq[0] = new DampedFloat;
  m_freq[1] = new DampedFloat;
 
  for (int i=0; i<3; ++i)
  { 
    m_envelopeMin[i] = new int [4096];
    m_envelopeMax[i] = new int [4096];
  }
  resetEnvelopeSLOT();
  selectModel( Dso::PCS64i );
  //selectModel( Dso::Simulator );
  
  setFrameRateSLOT( FR30 );
  setFftSizeSLOT( 3 );
  applyPrefsSLOT();
}

DsoWid::~DsoWid()
{
/*  QFile file( "/home/mt/qtdso_wisdom" );
  
  if (file.open( IO_WriteOnly ))
  {
    QDataStream ds( &file );
    ds << QString( m_wisdomString );
    file.close();
  }
  */
  fftw_destroy_plan( m_fftw_plan );
  fftw_free( m_fftwIn );
  fftw_free( m_fftwOut );
  
  for (int i=0; i<3; ++i)
  {
    delete [] m_envelopeMin[i];
    delete [] m_envelopeMax[i];
  }
  
  delete m_vpp[0];
  delete m_vpp[1];
  delete m_rms[0];
  delete m_rms[1];
}

void
DsoWid::setEnvelopeSLOT( bool on )
{
  m_showEnvelope = on;
  
  resetEnvelopeSLOT();
}

void
DsoWid::resetEnvelopeSLOT()
{
  for (int i=0; i<4096; ++i)
  {
    m_envelopeMin[0][i] = m_envelopeMin[1][i] = m_envelopeMin[2][i] = 100000;
    m_envelopeMax[0][i] = m_envelopeMax[1][i] = m_envelopeMax[2][i] = -100000;
  }
}

void
DsoWid::setProbeSLOT( int channel, float probe )
{
  m_probe[channel] = probe;
  
  m_vpp[0]->reset();
  m_vpp[1]->reset();
  m_rms[0]->reset();
  m_rms[1]->reset();
  m_freq[0]->reset();
  m_freq[1]->reset();
  
  update();
}

void
DsoWid::setTriggerStabilizerSLOT( bool on )
{
  m_triggerStabilizer = on;
}

void
DsoWid::setFftDrawModeSLOT( int channel, FFTDrawMode mode )
{
  m_fftDrawMode[channel] = mode;
}

void 
DsoWid::setTriggerSLOT( bool on )
{
  m_dso->setTriggerEnabled( on );
  resetEnvelopeSLOT();
}

void 
DsoWid::setTriggerRaisingSLOT( bool on )
{
  m_dso->setTriggerRaising( on );
  resetEnvelopeSLOT();
}

void 
DsoWid::setTriggerChannelSLOT( int channel )
{
  m_dso->setTriggerChannel( channel );
  resetEnvelopeSLOT();
}

void 
DsoWid::setTriggerExternalSLOT( bool ext )
{
  m_dso->setTriggerExternal( ext );
  resetEnvelopeSLOT();
}

void
DsoWid::setDcOffsetSLOT( int channel, int offset )
{
  m_dcOffset[channel] = offset;
  
  update();
}

void
DsoWid::setYOffsetSLOT( int channel, int offset )
{
  m_yOff[channel] = offset;
  
  update();
}

void
DsoWid::setInterpolSLOT( DsoWid::Interpolation type )
{
  m_interpolation = type;
  
  update();
}

void
DsoWid::setFftWindowNameSLOT( const QString & windowName )
{
  m_windowName = windowName;
  
  update();
}

void
DsoWid::setFftTypeSLOT( int type )
{
  m_fftType = (FFTType)type;
  flushFftAverageBufferSLOT();
}

void
DsoWid::setFftDisplaySLOT( int mode )
{
  m_fftDisplayMode = (FFTDisplayMode)mode;
  flushFftAverageBufferSLOT();
}

void
DsoWid::setFftShowPeaksSLOT( bool on )
{
  m_fftShowPeaks = on;
  
  update();
}

void
DsoWid::setFftZoomSLOT( int zoom )
{
  m_fftZoom = zoom;
  
  update();
}

void
DsoWid::setFftPostMagSLOT( float mag )
{
  m_fftPostMag = mag;
  flushFftAverageBufferSLOT();
  
  update();
}

void
DsoWid::setFftWindowSLOT( int id )
{
  m_fftWindow = id;
  flushFftAverageBufferSLOT();
}

void
DsoWid::setFftFreqSLOT( Dso::TimeBase tb )
{
  m_dso->setTimeBase( tb );  
  m_hasData = false;
  m_dso->resetNewData();
  
  setDsoAcqLength();
  
  flushFftAverageBufferSLOT();
}

void
DsoWid::setStepSLOT( int step )
{
  if (step >= 0)
  {
    createSinXInterpol();
  }
}

void
DsoWid::setStretchSLOT( int channel, float stretch )
{
  m_stretch[channel] = stretch;
  
  update();
}

void
DsoWid::setDrawTriggerLineSLOT( bool on )
{
  m_drawTriggerLine = on;
  
  update();
}

void
DsoWid::setShowChannel1SLOT( bool on )
{
  m_showChannel[0] = on;
  m_dso->setChannelEnable( 0, on );
  
  resetEnvelopeSLOT();
  update();
}

void
DsoWid::setShowChannel2SLOT( bool on )
{
  m_showChannel[1] = on;
  m_dso->setChannelEnable( 1, on );
  
  resetEnvelopeSLOT();
  update();
}

void
DsoWid::setMathModeSLOT( int mode )
{
  std::cerr << "SET MATH MODE SLOT " << mode << std::endl;
  
  m_mathMode = (MathMode)mode;

  switch (m_mathMode)
  {
  case NoMath:
    chColor[2] = addColor;
    break;
  case Add:
    chColor[2] = addColor;
    break;
  case Sub:
    chColor[2] = subColor;
    break;
  case Magnitude:
    chColor[2] = addColor;
    break;
  }
  
  applyPrefsSLOT();
  
  update();
}

void
DsoWid::setShowTimeMarkerSLOT( TimeMarkerMode mode )
{
  m_timeMarkerMode = mode;
  
  update();
}

void
DsoWid::setShowAmplitudeMarkerSLOT( bool on )
{
  m_showAmplitudeMarker = on;
  
  update();
}

void 
DsoWid::setXOffSLOT( int off )
{
  std::cerr << "xoff=" << off << std::endl;
  m_xOff = off;
  
  update();
}

void 
DsoWid::setMathOffSLOT( int off )
{
  m_yOff[2] = off;
  
  update();
}

void 
DsoWid::setMathStretchSLOT( float stretch )
{
  m_stretch[2] = stretch;
  
  update();
}

void 
DsoWid::setModeSLOT( DsoWid::Mode mode )
{
  m_mode = mode;
  
  if (m_mode != DsoWid::FFT)
  {
    setTimeBaseSLOT( m_tb[m_mode] );
  }
  else
  {
    setFftFreqSLOT( m_tb[m_mode] );    
  }
}

void
DsoWid::setDsoAcqLength()
{
  if (m_mode != DsoWid::FFT)
  {
    if (m_dso->fastAcq())
    {
      if (m_dso->step() < 0)
      {
        m_dso->setAcqLength( (int)((float)m_dso->numSamples() / 5. * 
            (float)(-m_dso->step())) );
        
        m_dso->setAcqOffset( m_dso->preTriggerSize() - (int)((float)m_dso->preTriggerSize() / 5. * 
            (float)(-m_dso->step())) );
      }
      else
      {
        m_dso->setAcqLength( (int)((float)m_dso->numSamples() / (float)m_dso->step() / 5.) );    
        
        m_dso->setAcqOffset( m_dso->preTriggerSize() - 
            (int)((float)m_dso->preTriggerSize() / (float)m_dso->step() / 5.) );
      }
    }
    else
    {
      m_dso->setAcqLength( m_dso->numSamples() );  
      m_dso->setAcqOffset( 0 );
    }
  }
  else
  {
    m_dso->setAcqLength( m_fftSize );
    m_dso->setAcqOffset( 0 );
  }
}

void
DsoWid::timerEvent( QTimerEvent *ev )
{
  static QTime t;
  static bool s_triggerOk = false;
  static int comeAgainCnt = 0;
  bool adjustFps = false;
  
  if (ev->timerId() == m_updateTimer || ev->timerId() == m_comeAgainTimer)
  {    
    if (m_comeAgainTimer != -1)
    {
      killTimer( m_comeAgainTimer );
      m_comeAgainTimer = -1;
    }
    
    if (adjustFps && m_continuousSampling)
    {
      adjustFps = false;
      killTimer( m_updateTimer );
      m_updateTimer = startTimer( 33 );
    }
    
    if (m_dso->hasNewData())
    {
      float rate = 1. / (float)t.elapsed() * 1000.;
      t.start();
      emit fps( rate );
      
      readFromDso();
      m_triggerOffset = m_triggerStabilizer ? m_dso->triggerOffset() : 0;
      m_dso->resetNewData();
      m_hasData = true;
      //std::cerr << "TR OFF: " << m_triggerOffset << std::endl;
      update();
      
      if (-1 == m_updateTimer && m_continuousSampling)
      {
        const int deltaT = QMAX( 0, 30-comeAgainCnt*10 );
        
        m_updateTimer = startTimer( deltaT );
        comeAgainCnt = 0;
        adjustFps = true;
      }
    }
    else
    {
      // COME BACK SOON! 
      // (TRY TO MAINTAIN MAX FRAMERATE, STILL BEEING COOPERATIVE)
      //
      m_comeAgainTimer = startTimer( 10 );
      comeAgainCnt++;
      if (-1 != m_updateTimer)
      {
        killTimer( m_updateTimer );
        m_updateTimer = -1;
      }
    }
    
    if (!m_dso->running())
    {
      m_dso->start();
    }
    
    if (s_triggerOk != m_dso->triggerOk())
    {
      emit triggerOk( m_dso->triggerOk() );
    }

    s_triggerOk = m_dso->triggerOk();  
  }
}

void
DsoWid::setVoltsDivSLOT( int channel, Dso::VoltsDiv id )
{  
  m_dso->setVoltsDiv( channel, id );
  
  m_vpp[0]->reset();
  m_vpp[1]->reset();
  m_rms[0]->reset();
  m_rms[1]->reset();
  
  resetEnvelopeSLOT();
  
  update();
}

void
DsoWid::setTimeBaseSLOT( Dso::TimeBase tb )
{
  m_tb[m_mode] = tb;
  m_dso->setTimeBase( tb );
  setStepSLOT( m_dso->step() );
  m_hasData = false;
  m_dso->resetNewData();
  
  setDsoAcqLength();
  
  resetEnvelopeSLOT();
  
  update();
}

void
DsoWid::resizeEvent( QResizeEvent * )
{
  m_buffer.resize( width(), height() );
}

void
DsoWid::setTriggerLevelSLOT( int triggerLevel )
{
  m_dso->setTriggerLevel( 255-triggerLevel );
  
  resetEnvelopeSLOT();
  
  update();
}

QImage
DsoWid::snapshot()
{
  return m_buffer.convertToImage();
}

void
DsoWid::paintEvent( QPaintEvent * )
{
  m_buffer.fill( bgColor );
  
  QPainter p;
  p.begin( &m_buffer );
  qDrawShadePanel( &p, 0, 0, width(), height(), 
                   colorGroup(), true, 5, 0 );
  p.end();
  paint( &m_buffer );  
  bitBlt( this, QPoint(0,0), &m_buffer, rect(), Qt::CopyROP );
}

void
DsoWid::paint( QPaintDevice *device, bool bw )
{
  QPainter p;
  p.begin( device );
  p.setFont( font() );
    
  // draw according to mode
  //
  switch (m_mode)
  {
  case DsoWid::DSO:
    drawEnvelope( &p, 0, bw );
    drawEnvelope( &p, 1, bw );
    drawEnvelope( &p, 2, bw );
    drawGrid( &p, bw );
    drawDSO( &p, bw );
    break;
  case DsoWid::XY:
    drawGrid( &p, bw );
    drawXY( &p, bw );
    break;
  case DsoWid::FFT:
    drawFFTGrid( &p, bw );
    drawFFT( &p, bw );
    break;
  default:   // oops
    p.end();
    return;
  }
    

  QString str;
  QString unit;
  
  if (DsoWid::DSO == m_mode || DsoWid::XY == m_mode)
  {
    if (m_dso->numSamples() == m_dso->acqLength())
    {
      bool  ok;

      for (int i=0; i<2; ++i)
      {
        const float f = frequency( i, &ok );

        if (ok)
        {
          m_freq[i]->addValue( f );
        }
        else
        {
          m_freq[i]->reset();
        }


        if (m_showChannel[i])
        {
          if (m_freq[i]->valid())
          {
            str = floatValueString( m_freq[i]->value(), Frequency, 4 );
          }
          else
          {
            str = "- Hz";
          }

          if (!bw)
          {
            p.setPen( chColor[i] );
          }
          
          if (0 == i)
          {
            p.drawText( m_x0+WINDOW_X_PIXELS-100-(m_showChannel[1] ? 100 : 0), 
                        5+2, 100, m_fh, 
                        Qt::AlignRight | Qt::AlignVCenter, str );
          }
          else
          {
            p.drawText( m_x0+WINDOW_X_PIXELS-100, 5+2, 100, m_fh, 
                        Qt::AlignRight | Qt::AlignVCenter, str );
          }
        }
      }
    }
  }
  else
  {
    // show string with frequency, windowtype, size, mode and averaging size
    //
    str = floatValueString( m_dso->samplingFrequency() / 2., Frequency );
    
    str += " ";
    str += m_windowName;    
    str += " (";
    str += QString().setNum(m_fftSize);
    str += tr(" points)");
    
    if (m_fftDisplayMode == Average)
    {
      str += tr(" Av. x");
      str += QString().setNum(m_fftOutAvCounter[0]);
    }
    else if (m_fftDisplayMode == Maximum)
    {
      str += " Maximum";
    }
    else if (m_fftDisplayMode == Minimum)
    {
      str += " Minimum";
    }
   
    if (!bw)
    {
      p.setPen( timeColor );
    }
    p.drawText( m_x0, 5+2, WINDOW_X_PIXELS, m_fh, 
                Qt::AlignRight | Qt::AlignVCenter, str );
  }
      
  p.end();
}

void
DsoWid::drawDSO( QPainter *p, bool bw )
{
  // draw in channel color
  //
  if (!bw)
  {
    p->setPen( QPen( chColor[m_dso->triggerChannel()], 0 ));
  }
  else
  {
    p->setPen( QPen( Qt::black, 1 ) );
  }
  
  // trigger position depending on channel y-pos and scaling
  //
  //int y = (int)qRound(m_y0+(WINDOW_Y_PIXELS)/2+m_yOff[m_dso->triggerChannel()]-
  //    ((m_dso->triggerLevel() & m_dso->triggerBits()))*m_stretch[m_dso->triggerChannel()]);
  const float stretch = m_stretch[m_dso->triggerChannel()] * m_dataStretch;
  int y = (WINDOW_Y_PIXELS)/2+m_y0+m_dcOffset[m_dso->triggerChannel()]*stretch+m_yOff[m_dso->triggerChannel()]
      -lrintf(stretch*(((m_dso->triggerLevel() & m_dso->triggerBits()))-m_dcOffset[m_dso->triggerChannel()])+m_dcOffset[m_dso->triggerChannel()]);
  
  
  // small triangles indicating level
  //
  p->moveTo( m_x0-7, y-2 );
  p->lineTo( m_x0-2, y );
  p->lineTo( m_x0-7, y+2 );
  p->lineTo( m_x0-7, y-2 );
  
  int x = m_x0+WINDOW_X_PIXELS+2;
  p->moveTo( x, y );
  p->lineTo( x+5, y-2 );
  p->lineTo( x+5, y+2 );
  p->lineTo( x, y );
  
  p->setClipRect( m_x0, m_y0, WINDOW_X_PIXELS, WINDOW_Y_PIXELS );
  
  // draw level line if slider is down
  //
  if (m_drawTriggerLine)
  {    
    p->setPen( QPen( chColor[m_dso->triggerChannel()], 0, QPen::DotLine ));
    p->drawLine( m_x0, y, m_x0+WINDOW_X_PIXELS, y );
  }
  
  if (!bw)
  {
    p->setPen( QPen( chColor[m_dso->triggerChannel()], 0 ));
  }
  else
  {
    p->setPen( QPen( Qt::black, 1 ));
  }
  
  x = m_x0+(m_dso->preTriggerSize()-m_xOff)/5;
  y = m_y0+WINDOW_Y_PIXELS-2;
  
  // small triangle indicating pretrigger prosition
  //
  p->moveTo( x-2, y );
  p->lineTo( x+2, y );
  p->lineTo( x, y-5 );
  p->lineTo( x-2, y );
    
  // draw position line if slider is down
  //
  if (m_drawTriggerLine)
  {
    if (!bw)
    {
      p->setPen( QPen( chColor[m_dso->triggerChannel()], 0, QPen::DotLine ));
    }
    else
    {
      p->setPen( QPen( Qt::black, 1, QPen::DotLine ));
    }
    
    p->drawLine( x, m_y0+5, x, m_y0+WINDOW_Y_PIXELS-5 );
  }
    
  p->setClipping( false );
  
  // draw all channels
  //
  doMath();

  drawChannel( p, 0, bw );
  drawChannel( p, 1, bw );    
  drawChannel( p, 2, bw );    

  // draw marker
  //
  drawTimeMarker( p, bw );
  drawAmplitudeMarker( p, bw );

  // draw info last. it might overlay something
  //
  drawDiv( p, 0, bw );
  drawDiv( p, 1, bw );
}

void
DsoWid::drawFFT( QPainter *p, bool bw )
{
  if (!m_hasData) return;
  
  const float fac = (float)(2*WINDOW_X_PIXELS)/fftSize()*(float)m_fftZoom;
  const int numPoints = fftSize()/2/m_fftZoom;
  
  // channel 0 is on top of channel 1
  //
  for (int channel=1; channel>=0; --channel)
  {
    if (m_showChannel[channel])
    {
      fft( channel );

      if (!bw)
      {
        p->setPen( QPen( chColor[channel], s_lineWidthFft ));
      }
      else
      {
        p->setPen( QPen( Qt::black, s_lineWidth ));
      }

      p->setClipRect( m_x0, m_y0, WINDOW_X_PIXELS, WINDOW_Y_PIXELS );

      int yOff = WINDOW_Y_PIXELS-(int)m_maxFftVal;
      
      if (m_fftDrawMode[channel] != Needles)
      {
        if (Lin == m_fftGridMode)
        {
          for (int i=0; i<numPoints; ++i)
          {
            m_arr3->setPoint( i, 
                              static_cast<int> (m_x0+fac*(float)i), 
                              static_cast<int> (m_y0+WINDOW_Y_PIXELS-m_fftOut[i+m_xOff]-yOff ));
          }
        }
        else
        {
          const float fStep = m_dso->samplingFrequency() / (float)m_fftSize;
          const float lFac = (float)WINDOW_X_PIXELS / log10( m_dso->samplingFrequency() / 2. ) * (float)m_fftZoom;
          const int xOff = lrintf( pow( 10, (m_xOff/lFac)) / fStep );
          
          for (int i=0; i<numPoints; ++i)
          {
            const int x = static_cast<int> (m_x0+lrintf( lFac*log10((float)i*fStep) ));
            
            m_arr3->setPoint( i, x, static_cast<int> (m_y0+WINDOW_Y_PIXELS-m_fftOut[i+xOff]-yOff ));
          }
        }
        
        if (m_fftDrawMode[channel] == Line)
        {
          p->drawPolyline( *m_arr3, 0, numPoints );
        }
        else
        {
          p->drawPoints( *m_arr3, 0, numPoints );
          m_arr3->translate( 0, 1 );
          p->drawPoints( *m_arr3, 0, numPoints );
        }
      }
      else
      {
        if (Lin == m_fftGridMode)
        {
          if (m_fftType == Power)
          {
            for (int i=0; i<numPoints; ++i)
            {
              m_arr3->setPoint( 2*i, static_cast<int> (m_x0+fac*(float)i), 
                                     m_y0+WINDOW_Y_PIXELS );
              m_arr3->setPoint( 2*i+1, static_cast<int> (m_x0+fac*(float)i), 
                                       static_cast<int> (m_y0+WINDOW_Y_PIXELS-m_fftOut[i+m_xOff] ));
            }
          }
          else
          {
            for (int i=0; i<numPoints; ++i)
            {
              m_arr3->setPoint( 2*i, static_cast<int> (m_x0+fac*(float)i), 
                                     m_y0+WINDOW_Y_PIXELS/2 );
              m_arr3->setPoint( 2*i+1, static_cast<int> (m_x0+fac*(float)i), 
                                       static_cast<int> (m_y0+WINDOW_Y_PIXELS-m_fftOut[i+m_xOff] ));
            }
          }
        }
        else
        {
          const float fStep = m_dso->samplingFrequency() / (float)m_fftSize;
          const float lFac = (float)WINDOW_X_PIXELS / log10( m_dso->samplingFrequency() / 2. ) * (float)m_fftZoom;
          
          if (m_fftType == Power)
          {
            for (int i=0; i<numPoints; ++i)
            {
              const int x = m_x0+lrintf( lFac*log10((float)i*fStep) );
              
              m_arr3->setPoint( 2*i, x, m_y0+WINDOW_Y_PIXELS );
              m_arr3->setPoint( 2*i+1, x, 
                                static_cast<int> (m_y0+WINDOW_Y_PIXELS-m_fftOut[i+m_xOff] ));
            }
          }
          else
          {
            for (int i=0; i<numPoints; ++i)
            {
              const int x = m_x0+lrintf( lFac*log10((float)i*fStep) );
              
              m_arr3->setPoint( 2*i, x, m_y0+WINDOW_Y_PIXELS/2 );
              m_arr3->setPoint( 2*i+1, x, 
                                static_cast<int> (m_y0+WINDOW_Y_PIXELS-m_fftOut[i+m_xOff] ));
            }
          }
        }
        
        p->drawLineSegments( *m_arr3, 0, numPoints );
      }

      if (m_fftShowPeaks && m_fftType == Power)
      {
        p->setPen( QPen( timeColor, 0 ));
        showFftPeaks( p, fac );
      }
      p->setClipping( false );
    }
  }
  
  drawFreqMarker( p, bw );
  drawDbMarker( p, bw );
}

void
DsoWid::drawXY( QPainter *p, bool bw )
{
  int xOff = (WINDOW_X_PIXELS-256)/2;
  const float stretchX = m_stretch[0] * m_dataStretch;
  const float stretchY = m_stretch[1] * m_dataStretch;
  
  if (!bw)
  {
    p->setPen( QPen( chColor[0], s_lineWidth ));
  }
  else
  {
    p->setPen( QPen( Qt::black, s_lineWidth ));
  }
  
  
  for (int i=0; i<m_dso->acqLength(); ++i)
  {
    m_arr2->setPoint( i, m_x0+xOff+(int)(stretchX*m_data[0][i])-m_yOff[0], 
                        WINDOW_Y_PIXELS-64-(int)(stretchY*m_data[1][i])+m_y0+m_yOff[1] );
  }
  
  p->setClipRect( m_x0, m_y0, WINDOW_X_PIXELS, WINDOW_Y_PIXELS );
  p->drawPolyline( *m_arr2, 0, m_dso->acqLength() );
  p->setClipping( false );
  
  drawDiv( p, 0, bw );
  drawDiv( p, 1, bw );  
}

void
DsoWid::drawTimeMarker( QPainter *p, bool bw )
{
  if (Off == m_timeMarkerMode) return;
  
  if (!bw)
  {
    p->setPen( QPen( timeColor, 0, QPen::DotLine ));
    //p->setRasterOp( XorROP );
  }
  else
  {
    p->setPen( QPen( Qt::black, 1, QPen::DotLine ));
  }
    
  p->drawLine( m_x0+m_timeMarker[0], m_y0, m_x0+m_timeMarker[0], m_y0+WINDOW_Y_PIXELS );
  if (RatioMarker == m_timeMarkerMode)
  {
    p->drawLine( m_x0+m_timeMarker[1], m_y0, m_x0+m_timeMarker[1], m_y0+WINDOW_Y_PIXELS );
  }
  p->drawLine( m_x0+m_timeMarker[2], m_y0, m_x0+m_timeMarker[2], m_y0+WINDOW_Y_PIXELS );
  
  p->setRasterOp( CopyROP );
  
  float diff[3];
  diff[0] = fabs( m_timeMarker[0]-m_timeMarker[1] );
  diff[1] = fabs( m_timeMarker[1]-m_timeMarker[2] );
  diff[2] = fabs( m_timeMarker[0]-m_timeMarker[2] );
  
  float dt;
  float pv;
  
  // sort for the user (hardcoded bubblesort)
  //
  if (m_timeMarker[0] > m_timeMarker[1])
  {
    if (m_timeMarker[0] > m_timeMarker[2])
    {
      if (m_timeMarker[1] > m_timeMarker[2])
      {
        // 2 1 0
        dt = diff[2];
        if (diff[0] != 0)
        {
          pv = diff[1] / diff[0];
        }
        else
        {
          pv = 0;
        }
      }
      else
      {
        // 1 2 0
        dt = diff[0];
        if (diff[2] != 0)
        {
          pv = diff[1] / diff[2];
        }
        else
        {
          pv = 0;
        }
      }
    }
    else
    {
      // 1 0 2
      dt = diff[1];
      if (diff[2] != 0)
      {
        pv = diff[0] / diff[2];
      }
      else
      {
        pv = 0;
      }
    }
  }
  else
  {
    if (m_timeMarker[0] > m_timeMarker[2])
    {
      // 2 0 1
      dt = diff[1];
      if (diff[0] != 0)
      {
        pv = diff[2] / diff[0];
      }
      else
      {
        pv = 0;
      }
    }
    else
    {
      if (m_timeMarker[1] > m_timeMarker[2])
      {
        // 0 2 1
        dt = diff[0];
        if (diff[1] != 0)
        {
          pv = diff[2] / diff[1];
        }
        else
        {
          pv = 0;
        }
      }
      else
      {
        // 0 1 2
        dt = diff[2];
        if (diff[1] != 0)
        {
          pv = diff[0] / diff[1];
        }
        else
        {
          pv = 0;
        }
      }
    }
  }
  
  dt = dt / 32. * m_dso->timeBase();
  
  if (dt != 0.0)
  {
    float f = 1./dt;
    
    QString str = "dt=";
    str += floatValueString( dt, Time );
    str += "  f=";
    str += floatValueString( f, Frequency );
    
    if (RatioMarker == m_timeMarkerMode)
    {
      str += "  ";
      str += floatValueString( pv, Ratio, 2 );
    }
    
    p->drawText( m_x0, m_y0+WINDOW_Y_PIXELS+2, WINDOW_X_PIXELS, m_fh, 
                 Qt::AlignLeft | Qt::AlignVCenter, str );
  }
}

void
DsoWid::drawFreqMarker( QPainter *p, bool bw )
{
  const float fac = (float)WINDOW_X_PIXELS/fftSize()*2.*(float)m_fftZoom;
  
  if (Off == m_timeMarkerMode) return;
    
  if (!bw)
  {
    p->setPen( QPen( timeColor, 0, QPen::DotLine ));
  }
  else
  {
    p->setPen( QPen( Qt::black, 1, QPen::DotLine ));
  }
    
  p->drawLine( m_x0+m_freqMarker, m_y0, m_x0+m_freqMarker, m_y0+WINDOW_Y_PIXELS );
  
  float f = ( m_xOff * fac + m_freqMarker ) / (float)WINDOW_X_PIXELS * (m_dso->samplingFrequency() / 2.) / m_fftZoom;
  
  QString str = "f=";
  str += floatValueString( f, Frequency );

  p->drawText( m_x0, m_y0-m_fh, WINDOW_X_PIXELS, m_fh, Qt::AlignLeft | Qt::AlignVCenter,
               str );
}

void 
DsoWid::drawAmplitudeMarker( QPainter *p, bool bw )
{
  if (!m_showAmplitudeMarker) return;
  
  int channel = 0;
  
  if (m_showChannel[0])
  {
    channel = 0;
  }
  else
  {
    channel = 1;
    
  }
  
  if (!bw)
  {
    p->setPen( QPen( chColor[channel], 0, QPen::DotLine ));
  }
  else
  {
    p->setPen( QPen( Qt::black, 1, QPen::DotLine ));
  }
  p->drawLine( m_x0, m_y0+m_amplitudeMarker[0], m_x0+WINDOW_X_PIXELS, m_y0+m_amplitudeMarker[0] );
  p->drawLine( m_x0, m_y0+m_amplitudeMarker[1], m_x0+WINDOW_X_PIXELS, m_y0+m_amplitudeMarker[1] );
  
  float dv = volts( channel, fabs( m_amplitudeMarker[0]-m_amplitudeMarker[1] ) ) / m_stretch[channel] * m_probe[channel];
  
  if (dv != 0.0)
  {
    QString str = "dV=";
    str += floatValueString( dv, Voltage );
    
    p->drawText( m_x0, m_y0+WINDOW_Y_PIXELS+2, WINDOW_X_PIXELS, m_fh, 
                 Qt::AlignRight | Qt::AlignVCenter, str );
  }
}

void 
DsoWid::drawDbMarker( QPainter *p, bool bw )
{
  if (!m_showAmplitudeMarker) return;
  
  if (m_showChannel[0])
  {
    if (!bw)
    {
      p->setPen( QPen( chColor[0], 0, QPen::DotLine ));
    }
    else
    {
      p->setPen( QPen( Qt::black, 1, QPen::DotLine ));
    }
  }
  else
  {
    if (!bw)
    {
      p->setPen( QPen( chColor[1], 0, QPen::DotLine ));
    }
    else
    {
      p->setPen( QPen( Qt::black, 1, QPen::DotLine ));
    }
  }
  
  p->drawLine( m_x0, m_y0+m_dbMarker[0], m_x0+WINDOW_X_PIXELS, m_y0+m_dbMarker[0] );
  p->drawLine( m_x0, m_y0+m_dbMarker[1], m_x0+WINDOW_X_PIXELS, m_y0+m_dbMarker[1] );
  
  if (Power == m_fftType)
  {
    float dv = fabs( m_dbMarker[0]-m_dbMarker[1] ) / 64. * 10.;

    if (dv != 0.0)
    {
      QString str; 
      str.sprintf( "d=%.2fdB", (float)dv );
      p->drawText( m_x0+100, m_y0-m_fh, 240, m_fh, 
                   Qt::AlignLeft | Qt::AlignVCenter, str );
    }
  }
  else
  {
    float dv = fabs( m_dbMarker[0]-m_dbMarker[1] ) / (float)WINDOW_Y_PIXELS * 360.;

    if (dv != 0.0)
    {
      QString str; 
      str.sprintf( "d=%.2f", (float)dv );
      p->drawText( m_x0+100, m_y0-m_fh, 240, m_fh, 
                   Qt::AlignLeft | Qt::AlignVCenter, str );
    }
  }
}

void
DsoWid::drawGrid( QPainter *p, bool bw )
{
  if (!bw)
  {
    p->setPen( gridColor );
  }
  else
  {
    p->setPen( QPen( Qt::black, 0 ) );
  }
  
  int verSteps = WINDOW_Y_PIXELS / 32;
  int horSteps = WINDOW_X_PIXELS / 32;
  
  // horizontal lines
  //
  for (int y=1; y<verSteps; ++y)
  {
    p->drawLine( m_x0, m_y0+32*y, m_x0+WINDOW_X_PIXELS-1, m_y0+32*y );
  }
  
  for (int x=1; x<horSteps; ++x)
  {
    p->drawLine( m_x0+32*x, m_y0, m_x0+32*x, m_y0+WINDOW_Y_PIXELS-1 );
  }
  
  float step = 6.4f;
  float pos = m_x0+6.4f;
  
  int xCenter = WINDOW_X_PIXELS / 2;
  int yCenter = WINDOW_Y_PIXELS / 2;
  int numXTicks = lrint((float)WINDOW_X_PIXELS / 6.4f);
  int numYTicks = lrint((float)WINDOW_Y_PIXELS / 6.4f);
  
  // center line grid
  //
  for (int x=1; x<numXTicks; ++x)
  {
    p->drawLine( lrintf(pos), m_y0+yCenter-2, lrintf(pos), m_y0+yCenter+2 );
    
    pos += step;
  }
  
  pos = m_y0+6.4;
  
  for (int y=1; y<numYTicks; ++y)
  {
    int xPos = lrintf(pos);
    
    p->drawLine( m_x0+xCenter-2, xPos, 
                 m_x0+xCenter+2, xPos );
    
    pos += step;
  }
        
  if (!bw)
  {
    p->setPen( QPen( gridColor, 0, QPen::DotLine ) );
  }
  else
  {
    p->setPen( QPen( Qt::black, 0, QPen::DotLine ) );
  }
  
  p->drawLine( m_x0, m_y0+yCenter-80, 
               m_x0+WINDOW_X_PIXELS, m_y0+yCenter-80 );
  p->drawLine( m_x0, m_y0+yCenter+80, 
               m_x0+WINDOW_X_PIXELS, m_y0+yCenter+80 );
  
  if (!bw)
  {
    p->setPen( borderColor );
  }
  else
  {
    p->setPen( QPen( Qt::black, 0 ) );
  }
  
  p->drawText( 6, m_y0+yCenter-80-m_fh/2, m_legWidth-8, m_fh, 
               Qt::AlignVCenter | Qt::AlignRight, "100%" );
  p->drawText( 6, m_y0+yCenter-64-m_fh/2, m_legWidth-8, m_fh, 
               Qt::AlignVCenter | Qt::AlignRight, "90%" );
  p->drawText( 6, m_y0+yCenter+64-m_fh/2, m_legWidth-8, m_fh, 
               Qt::AlignVCenter | Qt::AlignRight, "10%" );
  p->drawText( 6, m_y0+yCenter+80-m_fh/2, m_legWidth-8, m_fh, 
               Qt::AlignVCenter | Qt::AlignRight, "0%" );
  
  p->drawRect( m_x0-1, m_y0-1, WINDOW_X_PIXELS+2, WINDOW_Y_PIXELS+2 );
  
}

void
DsoWid::drawFFTGrid( QPainter *p, bool bw )
{
  if (!bw)
  {
    p->setPen( borderColor );
  }
  else
  {
    p->setPen( QPen( Qt::black, 0 )  );
  }
  
  QString str;
  QString unit;
  
  // horizontal lines
  //
  if (Power == m_fftType)
  {
    p->drawText( 8, 8+m_fh, "[dbV]" );

    for (int y=1; y<16; ++y)
    {
      if (!bw)
      {
        p->setPen( gridColor );
      }
      p->drawLine( m_x0, m_y0+y*24, m_x0+WINDOW_X_PIXELS, m_y0+y*24 );

      if (y & 1)
      {
        str.sprintf( "-%d", y*5 );
        if (!bw)
        {
          p->setPen( borderColor );
        }
        p->drawText( 8, m_y0+y*24-m_fh/2, m_x0-12, m_fh, 
                     Qt::AlignRight | Qt::AlignVCenter, str );
      }                 
    }
  }
  else
  {
    for (int y=0; y<=8; ++y)
    {
      if (!bw)
      {
        p->setPen( gridColor );
      }
      p->drawLine( m_x0, m_y0+y*48, m_x0+WINDOW_X_PIXELS, m_y0+y*48 );

      if (y & 1)
      {
        str.sprintf( "%d", 180-45*y );
        if (!bw)
        {
          p->setPen( borderColor );
        }
        p->drawText( 8, m_y0+y*48-m_fh/2, m_x0-12, m_fh, 
                     Qt::AlignRight | Qt::AlignVCenter, str );
        
        if (4 == y)
        {
          p->drawLine( m_x0, m_y0+y*48, m_x0+WINDOW_X_PIXELS, m_y0+y*48 );
        }
      }                 
    }    
  }
  
  if (Lin == m_fftGridMode)
  {
    // vertical lines
    //
    float freqStep = computeFreqScaleStep();
    const float fac = (float)WINDOW_X_PIXELS/(fftSize()-1)*2.*(float)m_fftZoom;
    float step = ((float)WINDOW_X_PIXELS/(m_dso->samplingFrequency() / 2.)*(float)m_fftZoom*freqStep);
    //step /= 2.;
    int numLines = (int)( WINDOW_X_PIXELS / step + 1 );

    float startFreq = (int)(fac*m_xOff/step)*freqStep;

    for (int i=0; i<numLines; ++i)
    {
      int pos = (int)( (float)m_x0+step*(float)i-fmod(m_xOff*fac, step) );

      if (pos > m_x0 && pos < m_x0+WINDOW_X_PIXELS)
      {
        if (!bw)
        {
          p->setPen( gridColor );
        }
        p->drawLine( pos, m_y0, pos, m_y0+WINDOW_Y_PIXELS );

        if (pos < m_x0+WINDOW_X_PIXELS-m_tw/2)
        {
          str = floatValueString( startFreq, Frequency );

          if (!bw)
          {
            p->setPen( borderColor );
          }
          p->drawText( pos-m_tw/2, m_y0+WINDOW_Y_PIXELS, m_tw, m_fh, 
                       Qt::AlignCenter, str );
        }
      }
      startFreq += freqStep;
    }
  }
  else
  {
    // vertical lines
    //
    float fac = 1;
    float step = ((float)WINDOW_X_PIXELS/log10(m_dso->samplingFrequency() / 2.)*(float)m_fftZoom);
    //step /= 2.;
    int numLines = (int)( WINDOW_X_PIXELS / step + 1 );

    float startFreq = (int)(m_xOff/step);

    for (int i=0; i<numLines; ++i)
    {
      int pos = (int)( (float)m_x0+step*(float)i-fmod(m_xOff*fac, step) );
      
      if (!bw)
      {
        p->setPen( gridColor );
      }
      for (int j=1; j<10; ++j)
      {
        int sp = pos + lrintf( step*log10((float)j));
        if (sp > m_x0 && sp < m_x0+WINDOW_X_PIXELS)
        {
          p->drawLine( sp, m_y0, sp, m_y0+WINDOW_Y_PIXELS );
        }
      }
      
      if (pos > m_x0 && pos < m_x0+WINDOW_X_PIXELS)
      {
        if (!bw)
        {
          p->setPen( gridColor );
        }
        p->drawLine( pos, m_y0, pos, m_y0+WINDOW_Y_PIXELS );

        if (pos < m_x0+WINDOW_X_PIXELS-m_tw/2)
        {
          str = floatValueString( pow( 10, startFreq ), Frequency );

          if (!bw)
          {
            p->setPen( borderColor );
          }
          p->drawText( pos-m_tw/2, m_y0+WINDOW_Y_PIXELS, m_tw, m_fh, 
                       Qt::AlignCenter, str );
        }
      }
      startFreq ++;
    }
  }
  
  // draw border
  //
  if (!bw)
  {
    p->setPen( borderColor );
  }
  
  p->drawRect( m_x0-1, m_y0-1, WINDOW_X_PIXELS+2, WINDOW_Y_PIXELS+2 );
}

float
DsoWid::computeFreqScaleStep()
{
  float winWidth = (m_dso->samplingFrequency() / 2.) / m_fftZoom;  
  float winStep = winWidth / (float)WINDOW_X_PIXELS * (float)m_tw;    
  float step = 1.;
  
  while (step < 1e8)
  {
    if (winStep > step && winStep <= 2.*step)
    {
      return 2.*step;
    }
    else if (winStep > 2.*step && winStep <= 5.*step)
    {
      return 5.*step;
    }
    else if (winStep > 5.*step && winStep <= 10.*step)
    {
      return 10.*step;
    }
    
    step *= 10.;
  }
  
  return 1e6;
}

void
DsoWid::drawEnvelope( QPainter *p, int channel, bool bw )
{
  if (!m_showChannel[channel]) return;
  
  const int lineOffset = s_lineWidth >> 1; 
  const float stretch = m_stretch[channel] * m_dataStretch;
  const int yOff = (WINDOW_Y_PIXELS)/2+m_y0+1+m_yOff[channel];
  
  p->setClipRect( m_x0, m_y0, WINDOW_X_PIXELS, WINDOW_Y_PIXELS );
  
  // show envelope
  //
  if (m_showEnvelope)
  {
    if (m_dso->step() < 0)      // oversampling
    {  
      int xOff = lrintf( (float)m_xOff / 5. * (float)(-m_dso->step()) + 
                 1024. + 204.8 * (float)m_dso->step() );
    
      QPointArray env( 2*m_dso->numSamples() );
      int numPoints = 0;
      
      for (int i=0; i<WINDOW_X_PIXELS; ++i)
      {
        int dataIndex = -m_dso->step()*i+xOff;
        
        for (int j=0; j<-m_dso->step(); ++j)
        {  
          env.setPoint( numPoints, m_x0+i, 
                        yOff-lrintf(stretch *
                          (m_envelopeMin[channel][dataIndex]-m_dcOffset[channel])+m_dcOffset[channel]-lineOffset));
          env.setPoint( numPoints+1, m_x0+i, 
                        yOff-lrintf(stretch *
                          (m_envelopeMax[channel][dataIndex]-m_dcOffset[channel])+m_dcOffset[channel]+lineOffset));

          dataIndex++;
          numPoints += 2;
        }
      }
      if (bw)
      {
        p->setPen( QPen( Qt::lightGray, 1 ) );
      }
      else
      {
        p->setPen( chColor[channel].dark() );
      }
      
      p->drawLineSegments( env, 0, numPoints );
    } 
  }  
  
  p->setClipping( false );
}

void
DsoWid::drawChannel( QPainter *p, int channel, bool bw )
{
  if (!m_showChannel[channel]) return;
  
  const float stretch = m_stretch[channel] * m_dataStretch;
  
  p->setClipRect( m_x0, m_y0, WINDOW_X_PIXELS, WINDOW_Y_PIXELS );

  // 0 > step means (-step)x averaging (-step measurements per pixel)
  // otherwise step means step in pixel per measurement value
  //
  const int yOff = (WINDOW_Y_PIXELS)/2+m_y0+m_dcOffset[channel]*stretch+m_yOff[channel];
  
  if (m_dso->step() < 0)      // oversampling
  {  
    int xOff = lrintf( (float)m_xOff / 5. * (float)(-m_dso->step()) + 
        1024. + 204.8 * (float)m_dso->step() );
      
    if (Dots == m_interpolation)                    // no interpolation
    {
      // fill array
      //
      for (int i=0; i<WINDOW_X_PIXELS; ++i)
      {
        for (int j=0; j<-m_dso->step(); ++j)
        {  
          const float val = mathSample( channel, 
                                        -m_dso->step()*i+j+xOff ) + m_dcOffset[channel];

          m_arr->setPoint( -m_dso->step()*i+j, m_x0+i, 
                            yOff-lrintf(stretch*(val-m_dcOffset[channel])+m_dcOffset[channel]) );
        }
      }
      drawPoints( p, channel, (-m_dso->step())*WINDOW_X_PIXELS, bw );
    }
    else if (LinearAverage == m_interpolation)      // linear average
    {
      // fill array
      //
      for (int i=0; i<WINDOW_X_PIXELS; ++i)
      {
        float sum = 0.;

        for (int j=0; j<-m_dso->step(); ++j)
        {
          sum += mathSample( channel, -m_dso->step()*i+j+xOff ) + m_dcOffset[channel];
        }
        sum /= (float)(-m_dso->step());

        m_arr->setPoint( i, m_x0+i, yOff-lrintf(stretch*(sum-m_dcOffset[channel])+m_dcOffset[channel]) );
      }
      
      drawLine( p, channel, WINDOW_X_PIXELS, bw );
    }
    else                                            // linear
    {
      // fill array
      //
      for (int i=0; i<WINDOW_X_PIXELS; ++i)
      {
        for (int j=0; j<-m_dso->step(); ++j)
        {  
          const float val = mathSample( channel, -m_dso->step()*i+j+xOff ) + m_dcOffset[channel];

          m_arr->setPoint( -m_dso->step()*i+j, m_x0+i, yOff-lrintf(stretch*(val-m_dcOffset[channel])+m_dcOffset[channel]) );
        }
      }
      
      drawLine( p, channel, (-m_dso->step())*WINDOW_X_PIXELS, bw );
    }
  }
  else              // undersampling
  {
    int xOff = lrintf( ((float)m_xOff-1024.) / 5. / (float)m_dso->step() + 1024. );
    
    if (SinX == m_interpolation)                    // sin(x)/x interpolation
    {
      // fill filter
      //
      int off = 0;
      xOff -= m_sinXLength/m_dso->step()+1;
      
      for (int i=0; i<m_sinXLength/m_dso->step()+1; ++i, ++off)
      {
        float val = (float)(mathSample( channel, i+xOff ) + m_dcOffset[channel]) / (float)SAMPLE_MAXVAL;

        for (int j=0; j<m_dso->step(); ++j)
        {
          (void)sinXFilter(val);
        }
      }
        
      // fill array
      //
      for (int i=0; i<WINDOW_X_PIXELS/m_dso->step()+1; ++i)
      {
        float val = (float)( mathSample( channel, i+xOff+off ) + m_dcOffset[channel]) / (float)SAMPLE_MAXVAL;

        for (int j=0; j<m_dso->step(); ++j)
        {
          m_arr->setPoint( i*m_dso->step()+j, m_x0+i*m_dso->step()+j, 
                           yOff-lrintf(stretch*(sinXFilter(val)*(float)SAMPLE_MAXVAL-m_dcOffset[channel])+
                              m_dcOffset[channel]) );
        }
      }
      
      drawLine( p, channel, WINDOW_X_PIXELS, bw );      
    }
    else                                          // linear or no interpolation
    {
      // fill array
      //
      int start = m_triggerOffset/m_dso->step();
      for (int i=0; i<WINDOW_X_PIXELS/m_dso->step(); ++i)
      {
        float val = (float)mathSample( channel, i+xOff+start ) + m_dcOffset[channel];

        m_arr->setPoint( i, m_x0+i*m_dso->step()-m_triggerOffset, yOff-lrintf(stretch*(val-m_dcOffset[channel])+m_dcOffset[channel]) );
      }
      
      if (Dots == m_interpolation)
      {
        drawPoints( p, channel, WINDOW_X_PIXELS/m_dso->step(), bw );    
      }
      else
      {
        drawLine( p, channel, WINDOW_X_PIXELS/m_dso->step(), bw );
      }
    }
  }
  
  p->setClipping( false );
}

void
DsoWid::drawLine( QPainter *p, int channel, int numPoints, bool print ) const
{
  p->save();
  
  int lw = s_lineWidth-1;
  const int lineOffset = s_lineWidth >> 1;
  
  if (print)
  {
    p->setPen( QPen( Qt::black, 1 ) );    
    p->translate( 0, -lineOffset );

    do
    {
      p->translate( 0, 1 );
      p->drawPolyline( *m_arr, 0, numPoints );
    }
    while (lw--);
  }
  else
  { 
    if (m_invCh[channel] || m_absCh[channel])
    {
      p->setPen( QPen( chChangedColor[channel], 0 ) );
    }
    else
    {
      p->setPen( QPen( chColor[channel], 0 ) );
    }
    
    p->translate( 0, -lineOffset );

    do
    {       
      p->translate( 0, 1 );
      p->drawPolyline( *m_arr, 0, numPoints );
    }
    while (lw--);
  }
  
  p->restore();
}

void
DsoWid::drawPoints( QPainter *p, int channel, int numPoints, bool /*print*/ ) const
{
  p->save();
  
  int lw = s_lineWidth-1;
  const int lineOffset = s_lineWidth >> 1;
  //const int lineWidth = print ? 1 : 0;
  
  if (lw > 0)
  {
    if (m_invCh[channel] || m_absCh[channel])
    {
      p->setPen( QPen( chChangedColor[channel], 0 ) );
    }
    else
    {
      p->setPen( QPen( chColor[channel], 0 ) );
    }
    p->translate( 0, -lineOffset );

    do
    {
      p->translate( 0, 1 );
      p->drawPoints( *m_arr, 0, numPoints );
    }
    while (lw--);
  }
 
  p->restore();  
}

void
DsoWid::drawDiv( QPainter *p, int channel, bool bw )
{  
  if (!m_showChannel[channel]) return;
  
  if (!bw)
  {
    p->setPen( QPen( chColor[channel], s_lineWidth ) );
  }
  else
  {
    p->setPen( QPen( Qt::black, s_lineWidth ));
  }
  
  float voltsDiv = m_dso->voltsDiv( channel ) / m_stretch[channel] * m_probe[channel];

  QString str = floatValueString( voltsDiv, Voltage );
  str += tr( " / div" );
  
  p->drawText( m_x0+100*channel, 5+2, 200, m_fh, Qt::AlignLeft | Qt::AlignVCenter,
               str );
  
  // draw measured voltages
  //
  if (m_drawMeasuredVolts && m_dso->numSamples() == m_dso->acqLength())
  {
    if (m_vpp[channel]->valid() && m_rms[channel]->valid())
    {
      str = floatValueString( m_vpp[channel]->value(), Voltage );
      str += "pp / ";
      str += floatValueString( m_rms[channel]->value(), Voltage );
      str += "rms";
    
      if (0 == channel)
      {
        const int w = fontMetrics().width( str );
        if (!bw)
        {
          p->fillRect( m_x0+1, m_y0+1, w, m_fh, bgColor );
        }
        else
        {
          p->fillRect( m_x0+1, m_y0+1, w, m_fh, Qt::white );
        }
        p->drawText( m_x0+1, 
                     m_y0+1, w, m_fh, Qt::AlignLeft | Qt::AlignVCenter,
                     str );
      }
      else
      {
        const int w = fontMetrics().width( str );
        if (!bw)
        {
          p->fillRect( m_x0+WINDOW_X_PIXELS-w-1, m_y0+1, w, m_fh, bgColor );
        }
        else
        {
          p->fillRect( m_x0+WINDOW_X_PIXELS-w-1, m_y0+1, w, m_fh, Qt::white );
        }
        p->drawText( m_x0+WINDOW_X_PIXELS-w-1, 
                     m_y0+1, w, m_fh, Qt::AlignRight | Qt::AlignVCenter,
                     str );
      }
    }
  }
  
  // draw time division
  //
  str = floatValueString( m_dso->timeBase(), Time );
  str += " / div";
  
  p->setPen( timeColor );
  p->drawText( m_x0+200, 5+2, 200, m_fh, Qt::AlignLeft | Qt::AlignVCenter, str );
}

void
DsoWid::doMath()
{
  const int acqPlus = m_dso->acqOffset() + m_dso->acqLength();
  
  switch (m_mathMode)
  {
  case NoMath:
    m_showChannel[2] = false;
    break;    
  case Add:
    m_showChannel[2] = true;
    
    for (int i=m_dso->acqOffset(); i<acqPlus; ++i)
    {
      m_data[2][i] = mathSample( 0, i ) + mathSample( 1, i ) + 127;
    
      m_envelopeMin[2][i] = QMIN( m_envelopeMin[2][i], m_data[2][i] - 127 );   
      m_envelopeMax[2][i] = QMAX( m_envelopeMax[2][i], m_data[2][i] - 127 );
    }   
    break;
  case Sub:
    m_showChannel[2] = true;
  
    for (int i=m_dso->acqOffset(); i<acqPlus; ++i)
    {
      m_data[2][i] = mathSample( 0, i ) - mathSample( 1, i ) + 127;
    
      m_envelopeMin[2][i] = QMIN( m_envelopeMin[2][i], m_data[2][i] - 127 );   
      m_envelopeMax[2][i] = QMAX( m_envelopeMax[2][i], m_data[2][i] - 127 );
    }
    break;
  case Magnitude:
    m_showChannel[2] = true;
  
    for (int i=m_dso->acqOffset(); i<acqPlus; ++i)
    {
      float a = mathSample( 0, i );
      float b = mathSample( 1, i );
      
      m_data[2][i] = sqrtf( a*a + b*b ) + 127;
    
      m_envelopeMin[2][i] = QMIN( m_envelopeMin[2][i], m_data[2][i] - 127 );   
      m_envelopeMax[2][i] = QMAX( m_envelopeMax[2][i], m_data[2][i] - 127 );
    }
    break;

  }
}

void
DsoWid::mouseMoveEvent( QMouseEvent *ev )
{
  int x = ev->x()-m_x0;
  int y = ev->y()-m_y0;
  
  if (m_mouseButton == LeftButton)
  {
    int dx = 0;
    int dy = 0;

    switch (m_moving)
    {
    case Freq:
      dx = m_freqMarker - x;
      m_freqMarker -= dx;
      if (m_freqMarker < 1) m_freqMarker = 1;
      if (m_freqMarker > WINDOW_X_PIXELS-1) m_freqMarker = WINDOW_X_PIXELS-1;
      break;
    case Time1:
      dx = m_timeMarker[0] - x;
      m_timeMarker[0] -= dx;
      if (m_timeMarker[0] < 1) m_timeMarker[0] = 1;
      if (m_timeMarker[0] > WINDOW_X_PIXELS-1) m_timeMarker[0] = WINDOW_X_PIXELS-1;
      if (TimeMarker == m_timeMarkerMode)
      {
        m_timeMarker[1] = QMIN( m_timeMarker[0], m_timeMarker[2] ) + 
            (QMAX( m_timeMarker[0], m_timeMarker[2] ) - QMIN( m_timeMarker[0], m_timeMarker[2] ))/2;  
      }
      break;
    case Time2:
      dx = m_timeMarker[1] - x;
      m_timeMarker[1] -= dx;
      if (m_timeMarker[1] < 1) m_timeMarker[1] = 1;
      if (m_timeMarker[1] > WINDOW_X_PIXELS-1) m_timeMarker[1] = WINDOW_X_PIXELS-1;
      break;
    case Time3:
      dx = m_timeMarker[2] - x;
      m_timeMarker[2] -= dx;
      if (m_timeMarker[2] < 1) m_timeMarker[2] = 1;
      if (m_timeMarker[2] > WINDOW_X_PIXELS-1) m_timeMarker[2] = WINDOW_X_PIXELS-1;
      if (TimeMarker == m_timeMarkerMode)
      {
        m_timeMarker[1] = QMIN( m_timeMarker[0], m_timeMarker[2] ) + 
            (QMAX( m_timeMarker[0], m_timeMarker[2] ) - QMIN( m_timeMarker[0], m_timeMarker[2] ))/2;  
      }
      break;
    case Amplitude1:
      dy = m_amplitudeMarker[0] - y;
      m_amplitudeMarker[0] -= dy;
      if (m_amplitudeMarker[0] < 1) m_amplitudeMarker[0] = 1;
      if (m_amplitudeMarker[0] > WINDOW_Y_PIXELS-1) m_amplitudeMarker[0] = WINDOW_Y_PIXELS-1;
      break;
    case Amplitude2:
      dy = m_amplitudeMarker[1] - y;
      m_amplitudeMarker[1] -= dy;
      if (m_amplitudeMarker[1] < 1) m_amplitudeMarker[1] = 1;
      if (m_amplitudeMarker[1] > WINDOW_Y_PIXELS-1) m_amplitudeMarker[1] = WINDOW_Y_PIXELS-1;
      break;
    case Db1:
      dy = m_dbMarker[0] - y;
      m_dbMarker[0] -= dy;
      if (m_dbMarker[0] < 1) m_dbMarker[0] = 1;
      if (m_dbMarker[0] > WINDOW_Y_PIXELS-1) m_dbMarker[0] = WINDOW_Y_PIXELS-1;
      break;
    case Db2:
      dy = m_dbMarker[1] - y;
      m_dbMarker[1] -= dy;
      if (m_dbMarker[1] < 1) m_dbMarker[1] = 1;
      if (m_dbMarker[1] > WINDOW_Y_PIXELS-1) m_dbMarker[1] = WINDOW_Y_PIXELS-1;
      break;
    default:
      if (m_mode == FFT)
      {
        dx = abs(m_x - ev->x() + m_x0);
        dy = abs(m_y - ev->y() + m_y0);

        if (dx > 3 || dy > 3)
        {
          if (dx > dy && abs(dx-dy) > 3 && m_timeMarkerMode != Off)
          {
            m_moving = Freq;
            m_freqMarker = ev->x()-m_x0;
          }
        }
      }
      break;
    }

    update();
  }
}

void
DsoWid::mousePressEvent( QMouseEvent *ev )
{
  m_x = ev->x()-m_x0;
  m_y = ev->y()-m_y0;
  
  m_mouseButton = ev->button();
  
  if (m_mouseButton == LeftButton)
  {
    m_moving = None;

    if (m_mode == DSO)
    {
      if (Off != m_timeMarkerMode && abs(m_x-m_timeMarker[0]) < 3)
      {
        m_moving = Time1;
      }
      else if (RatioMarker == m_timeMarkerMode && abs(m_x-m_timeMarker[1]) < 3)
      {
        m_moving = Time2;
      }
      else if (Off != m_timeMarkerMode && abs(m_x-m_timeMarker[2]) < 3)
      {
        m_moving = Time3;
      }
      else if (m_showAmplitudeMarker && abs(m_y-m_amplitudeMarker[0]) < 3)
      {
        m_moving = Amplitude1;
      }
      else if (m_showAmplitudeMarker && abs(m_y-m_amplitudeMarker[1]) < 3)
      {
        m_moving = Amplitude2;
      }
    }
    else if (m_mode == FFT)
    {
      if (Off != m_timeMarkerMode && abs(m_x-m_freqMarker) < 3)
      {
        m_moving = Freq;
      }
      else if (m_showAmplitudeMarker && abs(m_y-m_dbMarker[0]) < 3)
      {
        m_moving = Db1;
      }
      else if (m_showAmplitudeMarker && abs(m_y-m_dbMarker[1]) < 3)
      {
        m_moving = Db2;
      }
    }
  }
}

void
DsoWid::mouseReleaseEvent( QMouseEvent */*ev*/ )
{
  m_mouseButton = NoButton;
}

void
DsoWid::fft( int channel )
{
  // apply taper window
  //
  if (m_fftWindow)
  {
    for (int i=0; i<fftSize(); ++i)
    {
      //m_fftwIn[i].re = ((float)m_data[channel][i]-m_dcOffset[0]) * 
      //                  m_window[m_fftWindow-1][i] / (float)SAMPLE_MAXVAL;
      //m_fftwIn[i].im = 0.0;
      m_fftwIn[i][0] = ((float)m_data[channel][i]-m_dcOffset[0]) * 
                        m_window[m_fftWindow-1][i] / (float)SAMPLE_MAXVAL;
      m_fftwIn[i][1] = 0.0;
    }
  }
  else  // no taper window
  {
    for (int i=0; i<fftSize(); ++i)
    {
      //m_fftwIn[i].re = ((float)m_data[channel][i]-m_dcOffset[0]) / (float)SAMPLE_MAXVAL;
      //m_fftwIn[i].im = 0.0;
      m_fftwIn[i][0] = ((float)m_data[channel][i]-m_dcOffset[0]) / (float)SAMPLE_MAXVAL;
      m_fftwIn[i][1] = 0.0;
    }
  }
  
  // perform FFT
  //
  //fftw_one( m_fftw_plan, m_fftwIn, m_fftwOut );
  fftw_execute( m_fftw_plan );

  // compute normalized magnitude or phase
  // only half of points (it's symetric around f=0)
  //
  const float norm = (float)fftSize();
  const int fftSize2 = (fftSize() >> 1);
  
  if (Power == m_fftType)
  {
    for (int i=0; i<fftSize2; ++i)
    {
      //float re = m_fftwOut[i].re / norm;
      //float im = m_fftwOut[i].im / norm;
      const float re = m_fftwOut[i][0] / norm;
      const float im = m_fftwOut[i][1] / norm;
      
      m_fftOut[i] = sqrt( re*re+im*im ) * m_fftPostMag;
      
      if (m_fftOut[i] != 0)
      {
        m_fftOut[i] = (log10( m_fftOut[i] ) + 4.) / 4. * (float)WINDOW_Y_PIXELS;
      }
    }
  }
  else // phase fft
  {
    for (int i=0; i<fftSize2; ++i)
    {
      m_fftOut[i] = 
     //     (atan2( m_fftwOut[i].im, m_fftwOut[i].re ) + M_PI) * 
     //        (float)WINDOW_Y_PIXELS / 2. / M_PI;
          (atan2( m_fftwOut[i][1], m_fftwOut[i][0] ) + M_PI) * 
             (float)WINDOW_Y_PIXELS / 2. / M_PI;
    }
  }    

  
  // compute averaging or maximum display
  //
  if (Average == m_fftDisplayMode)
  {
    // if buffer full, subtract most distant value
    // which is the value under the pointer
    //
    if (m_fftOutAvCounter[channel] == m_fftAverageBufferLength) 
    {
      for (int i=0; i<(fftSize() >> 1); ++i)
      {
        m_fftOutSum[channel][i] -= m_fftOutBuffer[channel][m_fftOutAvPointer[channel]][i];
      }
    }
    
    // store current data at current pointer position
    //
    memcpy( m_fftOutBuffer[channel][m_fftOutAvPointer[channel]], m_fftOut, (fftSize() >> 1)*sizeof(float) );
    
    // increase pointer (modulo length)
    //
    m_fftOutAvPointer[channel] = (m_fftOutAvPointer[channel] + 1) % m_fftAverageBufferLength;
    
    // increase counter (and clamp to max length) while filling average buffer
    //
    m_fftOutAvCounter[channel]++;
    
    if (m_fftOutAvCounter[channel] > m_fftAverageBufferLength) 
    {
      m_fftOutAvCounter[channel] = m_fftAverageBufferLength;
    }
    
    // now add new values and divide by length of buffer
    //
    for (int i=0; i<fftSize2; ++i)
    {
      m_fftOutSum[channel][i] += m_fftOut[i];

      m_fftOut[i] = m_fftOutSum[channel][i] / (float)m_fftOutAvCounter[channel];
    }
  }
  else if (Maximum == m_fftDisplayMode)
  {
    for (int i=0; i<fftSize2; ++i)
    {
      m_fftOutBuffer[channel][m_fftOutAvPointer[channel]][i] = 
          QMAX( m_fftOutBuffer[channel][m_fftOutAvPointer[channel]][i], m_fftOut[i] );
    }
    memcpy( m_fftOut, m_fftOutBuffer[channel][m_fftOutAvPointer[channel]], (fftSize() >> 1)*sizeof(float) );
  } 
  else if (Minimum == m_fftDisplayMode)
  {
    for (int i=0; i<fftSize2; ++i)
    {
      m_fftOutBuffer[channel][m_fftOutAvPointer[channel]][i] = 
          QMIN( m_fftOutBuffer[channel][m_fftOutAvPointer[channel]][i], m_fftOut[i] );
    }
    memcpy( m_fftOut, m_fftOutBuffer[channel][m_fftOutAvPointer[channel]], (fftSize() >> 1)*sizeof(float) );
  } 
  
  // now find max peak, so we can show dbc
  //
  m_maxFftVal = 0.0f;
  for (int i=1; i<fftSize2; ++i) m_maxFftVal = std::max( m_maxFftVal, m_fftOut[i] );
  std::cerr << "max=" << m_maxFftVal << std::endl;
}

void
DsoWid::fillFFTWindows()
{
  const float arg = 2.0*M_PI/(fftSize()-1);
  const float arg2 = 1./(fftSize()-1);

  float sum = 0.;
  
  // HAMMING
  //
  for (int i=0; i<fftSize(); ++i)
  {
    m_window[Hamming-1][i] = (0.54-0.46*cos(arg*i));
    sum += m_window[Hamming-1][i];  
  }
  sum /= (float)fftSize();
  for (int i=0; i<fftSize(); ++i)
  {
    m_window[Hamming-1][i] /= sum;
  }
  sum = 0.;
  
  // BLACKMAN
  //
  for (int i=0; i<fftSize(); ++i)
  {
    m_window[Blackman-1][i] = (0.42-0.5*cos(arg*i)+
                                 0.08*cos(2*arg*i));
    sum += m_window[Blackman-1][i]; 
  }
  sum /= (float)fftSize();
  for (int i=0; i<fftSize(); ++i)
  {
    m_window[Blackman-1][i] /= sum;
  }
  sum = 0.;
  
  // BLACKMAN HARRIS
  //
  for (int i=0; i<fftSize(); ++i)
  {
    m_window[BlackmanHarris-1][i] = (0.35875-0.48829*cos(arg*i)+
                                          0.14128*cos(2.*arg*i)-
                                          0.01168*cos(3*arg*i));
    sum += m_window[BlackmanHarris-1][i]; 
  }
  sum /= (float)fftSize();
  for (int i=0; i<fftSize(); ++i)
  {
    m_window[BlackmanHarris-1][i] /= sum;
  }
  sum = 0.;
  
  // HANN
  //
  for (int i=0; i<fftSize(); ++i)
  {
    m_window[Hann-1][i] = 2.*(0.5-0.5*cos(arg*i));
    sum += m_window[Hann-1][i]; 
  }
  sum /= (float)fftSize();
  for (int i=0; i<fftSize(); ++i)
  {
    m_window[Hann-1][i] /= sum;
  }
  sum = 0.;
    
  // BARTLLETT HANN
  //
  for (int i=0; i<fftSize(); ++i)
  {
    m_window[BartlettHann-1][i] = (0.62-0.48*fabs(arg2*i-0.5)+
                                     0.38*cos(2*M_PI*(arg2*i-0.5)));
    
    sum += m_window[BartlettHann-1][i];
  }
  sum /= (float)fftSize();
  for (int i=0; i<fftSize(); ++i)
  {
    m_window[BartlettHann-1][i] /= sum;
  }
  sum = 0.;
  
  // GAUSSIAN
  //
  for (int i=0; i<fftSize(); ++i)
  {
    float x = (i-fftSize()/2.)/(float)fftSize()*2.;
    
    m_window[Gaussian-1][i] = exp(-3*(x*x));
    sum += m_window[Gaussian-1][i]; 
  }
  sum /= (float)fftSize();
  for (int i=0; i<fftSize(); ++i)
  {
    m_window[Gaussian-1][i] /= sum;
  }
  sum = 0.;
  
  // WELCH
  //
  for (int i=0; i<fftSize(); ++i)
  {
    float x = (i-fftSize()/2.)/(float)fftSize()*2.;
    
    m_window[Welch-1][i] = (1.-x*x);
    sum += m_window[Welch-1][i]; 
  }
  sum /= (float)fftSize();
  for (int i=0; i<fftSize(); ++i)
  {
    m_window[Welch-1][i] /= sum;
  }
  sum = 0.;
  
  // BARTLETT
  //
  for (int i=0; i<fftSize()/2; ++i)
  {
    m_window[Bartlett-1][i] = 2.*((float)i/(float)fftSize()*2.);  
    sum += m_window[Bartlett-1][i]; 
  }  
  for (int i=fftSize()/2; i<fftSize(); ++i)
  {
    m_window[Bartlett-1][i] = 2.*((float)(fftSize()-i)/(float)fftSize()*2.);  
    sum += m_window[Bartlett-1][i]; 
  }
  sum /= (float)fftSize();
  for (int i=fftSize(); i<fftSize(); ++i)
  {
    m_window[Bartlett-1][i] /= sum;  
  }
  sum = 0.;
  
  // KAISER BESSEL
  //
  for (int i=0; i<fftSize(); ++i)
  {
    m_window[KaiserBessel-1][i] = (0.4021-0.4986*cos(arg*i)+
                                         0.0981*cos(2.*arg*i)-
                                         0.0012*cos(3*arg*i));  
    sum += m_window[KaiserBessel-1][i];   
  }
  sum /= (float)fftSize();
  for (int i=0; i<fftSize(); ++i)
  {
    m_window[KaiserBessel-1][i] /= sum;    
  }
  sum = 0.;
    
  // FLAT TOP
  //
  for (int i=0; i<fftSize(); ++i)
  {
    m_window[FlatTop-1][i] = (0.2155 - 0.4159 * cos( arg * i ) +
                                       0.2780 * cos( 2. * arg * i ) -
                                       0.0836 * cos( 3. * arg * i ) +
                                       0.0070 * cos( 4. * arg * i ) ); 
    sum += m_window[FlatTop-1][i];     
  }
  sum /= (float)fftSize();
  for (int i=0; i<fftSize(); ++i)
  {
    m_window[FlatTop-1][i] /= sum;     
  }
}

void
DsoWid::setFftSizeSLOT( int id )
{  
  switch (id)
  {
  case 0:
    m_fftSize = FFT_SIZE_0;
    break;
  case 1:
    m_fftSize = FFT_SIZE_1;
    break;
  case 2:
    m_fftSize = FFT_SIZE_2;
    break;
  case 3:
    m_fftSize = FFT_SIZE_3;
    break;
  case 4:
    m_fftSize = FFT_SIZE_4;
    break;
  case 5:
    m_fftSize = FFT_SIZE_5;
    break;
  default:    // oops
    m_fftSize = FFT_SIZE_3;
    break;
  }
  
  m_dso->setAcqLength( m_fftSize );
  
  // TODO: Put fft and other calculations into thread - draw in mainthread
  //
  //m_fftw_plan = fftw_create_plan( m_fftSize, FFTW_FORWARD,
  //                                FFTW_MEASURE | FFTW_USE_WISDOM );
  
/*  if (m_wisdomString)
  {
    fftw_free( m_wisdomString );
  }*/
  m_fftw_plan = fftw_plan_dft_1d( m_fftSize, m_fftwIn, m_fftwOut,
                                  FFTW_FORWARD, FFTW_ESTIMATE );
    
/*  m_fftw_plan = fftw_create_plan( m_fftSize, FFTW_FORWARD,
                                  FFTW_ESTIMATE | FFTW_USE_WISDOM );*/

  //m_wisdomString = fftw_export_wisdom_to_string();
  
  fillFFTWindows();
  flushFftAverageBufferSLOT();
  
  setDsoAcqLength();
}

void
DsoWid::setFftScaleSLOT( int id )
{
  switch (id)
  {
  case 0:
    m_fftGridMode = Lin;
    update();
    break;
    
  case 1:
    m_fftGridMode = Log;
    update();
    break;
  }
}

void
DsoWid::setFftSizeFreeSLOT( int len )
{
  m_fftSize = len;
/*    
  if (m_wisdomString)
  {
    fftw_free( m_wisdomString );
    m_fftw_plan = fftw_create_plan( m_fftSize, FFTW_FORWARD,
                                    FFTW_ESTIMATE | FFTW_USE_WISDOM );
  }
  else
  {
    m_fftw_plan = fftw_create_plan( m_fftSize, FFTW_FORWARD,
                                    FFTW_MEASURE | FFTW_USE_WISDOM );
  }
  m_wisdomString = fftw_export_wisdom_to_string();
  */
      
  m_fftw_plan = fftw_plan_dft_1d( m_fftSize, m_fftwIn, m_fftwOut,
                                  FFTW_FORWARD, FFTW_ESTIMATE );
  
  fillFFTWindows(); 
  flushFftAverageBufferSLOT();
}

void
DsoWid::showFftPeaks( QPainter *p, float fac )
{
  p->setClipRect( m_x0, m_y0, WINDOW_X_PIXELS, WINDOW_Y_PIXELS );
  
  float d1 = m_fftOut[1] - m_fftOut[0];
  float d2;
  const float fftStep = m_dso->samplingFrequency()/m_fftSize;
  
  QString str;
  QString unit;
    
  if (Lin == m_fftGridMode)
  {
    for (int i=1; i<fftSize()/2-3; ++i)
    {
      d2 = m_fftOut[i+1] - m_fftOut[i]; 

      if (m_fftOut[i] > 70 && d1 > 0 && d2 < 0)
      {
        if (m_fftOut[i] - m_fftOut[i-2] > 40 && 
            m_fftOut[i+2] - m_fftOut[i] < -40)
        {
          const int x = (int)(m_x0+fac*(i-m_xOff))+1;
          const int y = (int)(m_y0+WINDOW_Y_PIXELS-m_fftOut[i])-1;

          str = floatValueString( i*fftStep, Frequency );

          p->drawText( x, y, str );
        }
      }
      d1 = d2;
    }
  }
  else
  {
    float lFac = (float)WINDOW_X_PIXELS / log10( m_dso->samplingFrequency() / 2. ) * (float)m_fftZoom;
    
    for (int i=1; i<fftSize()/2-3; ++i)
    {
      d2 = m_fftOut[i+1] - m_fftOut[i]; 

      if (m_fftOut[i] > 70 && d1 > 0 && d2 < 0)
      {
        if (m_fftOut[i] - m_fftOut[i-2] > 40 && 
            m_fftOut[i+2] - m_fftOut[i] < -40)
        {
          const int x = m_x0+lrintf( lFac*log10((float)i*fftStep) );
          const int y = (int)(m_y0+WINDOW_Y_PIXELS-m_fftOut[i])-1;

          str = floatValueString( i*fftStep, Frequency );

          p->drawText( x, y, str );
        }
      }
      d1 = d2;
    }
  }
  
  p->setClipping( false );
}
 
void
DsoWid::createSinXInterpol()
{
  delete [] m_sinXRegister;
  delete [] m_sinXCoeff;
  
  // i've choosen these values with my butt.
  // no technical reasons
  //
  switch (m_dso->step())
  {
  case 10:
    m_sinXLength = 401;
    break;
  case 5:
    m_sinXLength = 201;
    break;
  case 2:
    m_sinXLength = 101;
    break;
  default:
    m_sinXLength = 41;
    break;
  }
  
  m_sinXPointer = 0;
  
  m_sinXRegister = new float [m_sinXLength];
  m_sinXCoeff    = new float [m_sinXLength];
      
  float sum = 0;
  int N2 = m_sinXLength/2;
  
  for (int i=-N2; i<=N2; ++i)
  {
    float coeff = sin( M_PI*(float)i/(float)m_dso->step() );
    
    if (i)
    {
      coeff /= M_PI*(float)i/5.;
    }
    else
    {
      coeff = 1.;
    }
      
    sum += coeff;
    
    m_sinXCoeff[i+N2] = coeff;
  }
  
  for (int i=0; i<m_sinXLength; ++i)
  {
    m_sinXCoeff[i] /= sum;
  }
}

void
DsoWid::flushFftAverageBufferSLOT()
{
  if (Maximum == m_fftDisplayMode)
  {
    for (int i=0; i<fftSize()/2; ++i)
    {
      m_fftOutBuffer[0][m_fftOutAvPointer[0]][i] = 0;
      m_fftOutBuffer[1][m_fftOutAvPointer[1]][i] = 0;
    }
  }
  else if (Minimum == m_fftDisplayMode)
  {
    for (int i=0; i<fftSize()/2; ++i)
    {
      m_fftOutBuffer[0][m_fftOutAvPointer[0]][i] = 0xffff;
      m_fftOutBuffer[1][m_fftOutAvPointer[1]][i] = 0xffff;
    }
  }
  else if (Average == m_fftDisplayMode)
  {
    for (int i=0; i<fftSize()/2; ++i)
    {
      m_fftOutSum[0][i] = 0.0;
      m_fftOutSum[1][i] = 0.0;
    }
    m_fftOutAvPointer[0] = 0;
    m_fftOutAvCounter[0] = 0;
    m_fftOutAvPointer[1] = 0;
    m_fftOutAvCounter[1] = 0;
  } 
}

void
DsoWid::setFrameRateSLOT( FrameRate fps )
{
  m_frameRate = fps;  
  m_realFrameRate = 30;
  
  if (!m_continuousSampling) return;
  
  if (m_updateTimer != -1)
  {
    killTimer( m_updateTimer );
  }
  
  switch (fps)
  {
  case FR5:
    m_updateTimer = startTimer( 200 );
    m_updateTime = 200;
    m_realFrameRate = 5;
    break;
  case FR10:
    m_updateTimer = startTimer( 100 );
    m_updateTime = 100;
    m_realFrameRate = 10;
    break;
  case FR20:
    m_updateTimer = startTimer( 50 );
    m_updateTime = 50;
    m_realFrameRate = 20;
    break;
  case FR25:
    m_updateTimer = startTimer( 40 );
    m_updateTime = 40;
    m_realFrameRate = 25;
    break;    
  case FR30:
    m_updateTimer = startTimer( 30 );
    m_updateTime = 30;
    m_realFrameRate = 30;
    break;    
  } 
}

int 
DsoWid::frameRate() const
{
  return m_realFrameRate;
}

void
DsoWid::readFromDso()
{
  m_minSample[0] = 0xffff;
  m_maxSample[0] = 0;
  m_minSample[1] = 0xffff;
  m_maxSample[1] = 0;
  
  m_averageSample[0] = 0;
  m_averageSample[1] = 0;
  
  m_dso->lock();
  
  if (m_converterHistogram && m_converterHistogram->isVisible())
  {
    m_converterHistogram->addValues( m_dso );
  }
  
  const int acqPlus = m_dso->acqLength()  + m_dso->acqOffset();
  
  for (int i=m_dso->acqOffset(); i<acqPlus; ++i)
  {
    const int val0 = m_dso->data( 0, i ); 
    const int val1 = m_dso->data( 1, i );
    
    // get values from DSO and retrieve min/max
    //
    m_data[0][i] = val0;
    m_data[1][i] = val1;
    
    m_minSample[0] = QMIN( val0, m_minSample[0] );
    m_maxSample[0] = QMAX( val0, m_maxSample[0] );
    m_minSample[1] = QMIN( val1, m_minSample[1] );
    m_maxSample[1] = QMAX( val1, m_maxSample[1] );
    
    m_envelopeMin[0][i] = QMIN( m_envelopeMin[0][i], val0 - m_dcOffset[0] );
    m_envelopeMin[1][i] = QMIN( m_envelopeMin[1][i], val1 - m_dcOffset[1] );
    
    m_envelopeMax[0][i] = QMAX( m_envelopeMax[0][i], val0 - m_dcOffset[0] );
    m_envelopeMax[1][i] = QMAX( m_envelopeMax[1][i], val1 - m_dcOffset[1] );
    
    // do averaging
    //
    m_averageSample[0] += val0;
    m_averageSample[1] += val1;
  }
  
  m_averageSample[0] /= m_dso->acqLength();
  m_averageSample[1] /= m_dso->acqLength();
    
  m_dso->unlock();
  
  emit dynamicRange( m_dso->maxValue(),
                     m_minSample[0], m_maxSample[0], 
                     m_minSample[1], m_maxSample[1] );
  
  m_vpp[0]->addValue( volts( 0, m_maxSample[0]-m_minSample[0] )  * m_probe[0] );
  m_vpp[1]->addValue( volts( 1, m_maxSample[1]-m_minSample[1] )  * m_probe[1] );
  
  m_rms[0]->addValue( rms(0) );
  m_rms[1]->addValue( rms(1) );
}
  
void
DsoWid::singleShotSLOT()
{
  // we really want new data
  //
  m_dso->resetNewData();
  if (!m_dso->running())
  {
    m_dso->start();
  }
  
  if (m_updateTimer != -1)
  {
    killTimer( m_updateTimer );
    m_updateTimer = -1;
  }
  
  // and we want it ASAP
  //
  m_comeAgainTimer = startTimer( 10 ); 
}

void
DsoWid::setContinuousSamplingSLOT( bool on )
{
  m_continuousSampling = on;
  
  if (on)
  {
    setFrameRateSLOT( m_frameRate );
  }
  else
  {
    if (-1 != m_comeAgainTimer)
    {
      killTimer( m_comeAgainTimer );
      m_comeAgainTimer = -1;
    }
    
    if (-1 != m_updateTimer)
    {
      killTimer( m_updateTimer );
      m_updateTimer = -1;
    }
  }
}

float
DsoWid::frequency( int channel, bool *ok ) const
{
  bool  high = false;
  
  if (ok) *ok = true;
    
  int last=0;
  bool first=true;
  
  // trigger levels with hysteresis to avoid false triggering
  //
  const float highLevel = (m_averageSample[channel] + m_maxSample[channel]) >> 2;
  const float lowLevel  = (m_averageSample[channel] + m_minSample[channel]) >> 2;
  
  if (highLevel == lowLevel)
  {
    if (ok) *ok = false;
    return 0.0;
  }
  
  float sum=0.0;
  int periodCount=0;
  
  // first synchronize with the first period
  // (otherwise the first period length might be wrong)
  //
  int start = m_dso->acqOffset();
  const int acqPlus = start + m_dso->acqLength();
  
  for ( ; start<acqPlus; ++start)
  {
    const float val = m_data[channel][start] >> 1;
       
    if (!high && val >= highLevel)
    {
      if (!first) break;
      high = true;
      first = false;
    }
    else if (high && val < lowLevel )
    {
      high = false;
    }    
  }
  
  high = true;
  first = true;
  
  // compute average period length
  //
  for (int i=start; i<acqPlus; ++i)
  {
    const float val = m_data[channel][i] >> 1;
    
    if (!high && val >= highLevel)
    {
      if (!first)
      {
        sum += (float)(i-last);
        ++periodCount;
      }
      else
      {
        first = false;
      }
      
      last = i;
      high = true;
    }
    else if (high && val < lowLevel )
    {
      high = false;
    }
  }
  
  if (periodCount)
  {
    sum /= (float)periodCount;
  }
  else
  {
    if (ok) *ok = false;
  }
  
  return m_dso->samplingFrequency()/sum;;
}
  
QString
DsoWid::floatValueString( float value, ValueType type, int prec )
{
  QString retString;
  QString unit;
  
  switch (type)
  {
  case Frequency:
    {
      unit = "Hz";
      if (value >= 1000.)
      {
        value /= 1000.;
        unit = "kHz";
      }
      if (value >= 1000.)
      {
        value /= 1000.;
        unit = "MHz";
      }
      if (value >= 1000.)
      {
        value /= 1000.;
        unit = "GHz";
      }
      
      retString.setNum( value, 'f', 8 );
    }
    break;
    
  case Time:
    {
      unit = "s";
      if (value < 1.0)
      {
        value *= 1000.;
        unit = "ms";
      }
      if (value < 1.0)
      {
        value *= 1000.;
        unit = "\265s";
      }
      if (value < 1.0)
      {
        value *= 1000.;
        unit = "ns";
      }
      
      retString.setNum( value, 'f', 8 );
    }
    break;
    
  case Voltage:
    {
      unit = "V";     
      if (value < 0.5)
      {
        value *= 1000.;
        unit = "mV";
      }
      if (value < 0.5)
      {
        value *= 1000.;
        unit = "V";
      }
      
      retString.setNum( value, 'f', 8 );
    }
    break;
    
  case Ratio:
    {
      if (value == 0)
      {
        retString = "";
      }
      else
      {
        if (value < 1)
        {
          if (1./value < 100)
          {
            QString str;
            str.setNum( 1./value, 'f', prec );
            retString = "1:";
            retString += str;
          }
          else
          {
            retString.sprintf( "1:%d", (int)(1./value) );
          }
        }
        else
        {
          if (value < 100)
          {
            retString.setNum( value, 'f', prec );
            retString += ":1";
          }
          else
          {
            retString.sprintf( "%d:1", (int)value );
          }
        }
      }
    }
    break;
  }
  
  if (type != Ratio)
  {
    int cnt=0;
    bool separator=false;
    
    for (unsigned i=0; i<retString.length(); ++i)
    {
      if (cnt >= prec && separator)
      {
        retString = retString.left(i);
        if (retString[i-1] == '.')
        {
          retString = retString.left(i-1);
        }
        break;
      }
      
      if (retString[i].isNumber()) cnt++;
      if (retString[i] == '.') separator = true;
    }
  }
  
  retString += unit;
  
  return retString;    
}

float
DsoWid::rms( int channel )
{
  float rms = 0.0;
  
  const int acqPlus = m_dso->acqOffset() + m_dso->acqLength();
  
  for (int i=m_dso->acqOffset(); i<acqPlus; ++i)
  {
    const float value = (float)m_data[channel][i]-(float)m_dcOffset[channel];
    rms += value*value;
  }
  
  rms /= (float)m_dso->acqLength();  
  rms = sqrt( rms );    
  rms = volts( channel, rms ) * m_probe[channel];
  
  return rms;
}

void
DsoWid::setFftBufferLength( int len )
{
  flushFftAverageBufferSLOT();
  
  for (int i=0; i<m_fftAverageBufferLength; ++i)
  {
    delete [] m_fftOutBuffer[0][i];
    delete [] m_fftOutBuffer[1][i];
  }
  
  m_fftAverageBufferLength = len;
  
  for (int i=0; i<m_fftAverageBufferLength; ++i)
  {
    m_fftOutBuffer[0][i] = new float [FFT_SIZE_MAX/2];
    m_fftOutBuffer[1][i] = new float [FFT_SIZE_MAX/2];
  }
}

void
DsoWid::print( QPrinter *prt )
{
  paint( prt, true );
}

bool
DsoWid::selectModel( Dso::Model model )
{
  if (m_model == model) return true;

  if (m_comeAgainTimer != -1)
  {
    killTimer( m_comeAgainTimer );
    m_comeAgainTimer = -1;
  }

  if (m_updateTimer != -1)
  {
    killTimer( m_updateTimer );
    m_updateTimer = -1;
  }
  
  if (m_dso)
  {
    while (m_dso->running()) { usleep(20000); }
  }
  
  delete m_dso;
  m_dso = 0;
  delete m_simWid;
  m_simWid = 0;
  
  Simulator *sim;
  
  switch (model)
  {
  case Dso::Simulator:
    m_simWid = new SimulatorWid;
    m_simWid->show();
    sim = new Simulator;
    m_dso = sim;
    connect( m_simWid, SIGNAL( waveformChanged( unsigned, Simulator::Waveform )),
             sim, SLOT( waveformChangedSLOT( unsigned, Simulator::Waveform )));
    connect( m_simWid, SIGNAL( frequencyChanged( unsigned, float )),
             sim, SLOT( frequencyChangedSLOT( unsigned, float )));
    connect( m_simWid, SIGNAL( vppChanged( unsigned, float )),
             sim, SLOT( vppChangedSLOT( unsigned, float )));
    connect( m_simWid, SIGNAL( noiseToggled( unsigned, bool )),
             sim, SLOT( noiseToggledSLOT( unsigned, bool )));
    connect( m_simWid, SIGNAL( jitterToggled( unsigned, bool )),
             sim, SLOT( jitterToggledSLOT( unsigned, bool )));
    connect( m_simWid, SIGNAL( phaseJitterToggled( unsigned, bool )),
             sim, SLOT( phaseJitterToggledSLOT( unsigned, bool )));
    connect( m_simWid, SIGNAL( symetryChanged( unsigned, int )),
             sim, SLOT( symetryChangedSLOT( unsigned, int )));
    
    init();
    m_dso->start();
    break;
    
  case Dso::PCS64i:
    m_dso = new Pcs64i;
    init();
    m_dso->start();
    break;
    
  default:
    std::cerr << "NOT IMPLEMENTED" << std::endl;
    return false;
    break;
  }
  
  m_model = model;
  
  if (m_dso)
  {
    m_dataStretch = 255.0 / (float)m_dso->maxValue();
  }
  
  emit dsoChanged();
  
  return true;
}

void
DsoWid::init()
{
  delete [] m_data[0];
  delete [] m_data[1];
  delete [] m_data[2];
  
  delete m_arr;
  delete m_arr2;
  delete m_arr3;
  
  m_data[0] = new int [m_dso->numSamples()];
  m_data[1] = new int [m_dso->numSamples()];
  m_data[2] = new int [m_dso->numSamples()];  // this buffer is used for math
  
  m_arr = new QPointArray( 5*WINDOW_X_PIXELS );
  m_arr2 = new QPointArray( m_dso->numSamples() );
  m_arr3 = new QPointArray( FFT_SIZE_MAX );
}

void
DsoWid::internalAdjustSize()
{
  m_fh = fontMetrics().height();
  m_tw = fontMetrics().width( "999.9MHz" );
    
  m_legWidth = fontMetrics().width( "100%" ) + 14;
  
  m_x0 = 1+5 + m_legWidth;
  m_y0 = 1+2 + m_fh + 5;
  
  setFixedSize( WINDOW_X_PIXELS+1+8+5+m_x0, WINDOW_Y_PIXELS+2*m_fh+8+10 );
}

void
DsoWid::applyPrefsSLOT()
{
  internalAdjustSize();
  
  //const int off = s_lineWidth / 2;
  //double aStep = (1.5708-0.201)/(double)off;
  //double a = 0.201;
}

void
DsoWid::setChannelOpSLOT( bool invCh1, bool absCh1, bool invCh2, bool absCh2 )
{
  m_invCh[0] = invCh1;
  m_absCh[0] = absCh1;
  m_invCh[1] = invCh2;
  m_absCh[1] = absCh2;
  
  applyPrefsSLOT();
  
  update();
}

void
DsoWid::fastAcqSLOT( bool on )
{
  m_dso->setFastAcq( on );
  setDsoAcqLength();
}

void
DsoWid::equivalentSamplingSLOT( bool on )
{
  m_dso->setEquivalentSampling( on );
}

void
DsoWid::histogramSLOT()
{
  if (!m_converterHistogram)
  {
    m_converterHistogram = new ConverterHistogramDlg( this );
  }
  
  m_converterHistogram->raise();
  m_converterHistogram->show();
}

void
DsoWid::dcOffsetSLOT()
{
  setDcOffsetSLOT( 0, m_averageSample[0] );
  setDcOffsetSLOT( 1, m_averageSample[1] );
}
