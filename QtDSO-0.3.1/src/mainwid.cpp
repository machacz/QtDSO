//======================================================================
// File:		mainwid.cpp
// Author:	Matthias Toussaint
// Created:	Sun Jun  9 22:30:36 CEST 2002
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

#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qfile.h>
#include <qscrollbar.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qtoolbutton.h>
#include <qlabel.h>

#include <math.h>
#include <stdlib.h>

#include <dso.h>
#include <dsowid.h>
#include <mainwid.h>
#include <buttongrid.h>

#include <iostream>

MainWid::MainWid( QWidget *parent, const char *name ) :
  UIMainWid( parent, name ),
  m_mode( 0 )
{
  m_data[0] = new Q_UINT8 [SAMPLE_LENGTH];
  m_data[1] = new Q_UINT8 [SAMPLE_LENGTH];
  
  m_fac[0] = m_fac[1] = 1.0;
  m_voltsDivId[0] = m_voltsDivId[1] = 0;
  
  m_yPos[0][0] = m_yPos[1][0] = m_yPos[0][1] = m_yPos[1][1] = 0;
  m_yStretch[0][0] = m_yStretch[1][0] = 
      m_yStretch[0][1] = m_yStretch[1][1] = 0;
  
  // x-pos
  connect( ui_xPos, SIGNAL( valueChanged( int ) ),
           ui_dsoWid, SLOT( setXOffSLOT( int ) ));
  
  // y-pos math
  connect( ui_yPosMath, SIGNAL( valueChanged( int ) ),
           this, SLOT( mathOffsetChangedSLOT( int ) ));
  
  // stretch math
  connect( ui_stretchMath, SIGNAL( valueChanged( int ) ),
           this, SLOT( mathStretchChangedSLOT( int ) ));
  
  // trigger level
  connect( ui_triggerLevel, SIGNAL( valueChanged( int ) ),
           ui_dsoWid, SLOT( setTriggerLevelSLOT( int ) ));
  connect( ui_triggerLevel, SIGNAL( sliderPressed() ),
           this, SLOT( triggerPressSLOT() ));
  connect( ui_triggerLevel, SIGNAL( sliderReleased() ),
           this, SLOT( triggerReleaseSLOT() ));
  
  // trigger selection
  connect( ui_trigger, SIGNAL( toggled( bool ) ),
           ui_dsoWid, SLOT( setTriggerSLOT( bool ) ));
  connect( ui_triggerStabilizer, SIGNAL( toggled( bool ) ),
           ui_dsoWid, SLOT( setTriggerStabilizerSLOT( bool ) ));
  connect( ui_ch1Trigger, SIGNAL( toggled( bool ) ),
           this, SLOT( setTriggerChannelSLOT() ));
  connect( ui_ch2Trigger, SIGNAL( toggled( bool ) ),
           this, SLOT( setTriggerChannelSLOT() ));
  connect( ui_extTrigger, SIGNAL( toggled( bool ) ),
           this, SLOT( setTriggerChannelSLOT() ));
  connect( ui_raising, SIGNAL( toggled( bool ) ),
           this, SLOT( setTriggerEdgeSLOT() ));
  connect( ui_falling, SIGNAL( toggled( bool ) ),
           this, SLOT( setTriggerEdgeSLOT() ));
  
  // marker visibility
  connect( ui_timeMarker, SIGNAL( toggled( bool ) ),
           this, SLOT( setShowTimeMarkerSLOT( bool ) ));
  connect( ui_timeMarker2, SIGNAL( toggled( bool ) ),
           this, SLOT( setShowTimeMarker2SLOT( bool ) ));
  connect( ui_amplitudeMarker, SIGNAL( toggled( bool ) ),
           ui_dsoWid, SLOT( setShowAmplitudeMarkerSLOT( bool ) ));
  
  // math visibility
  connect( ui_mathModeGroup, SIGNAL( clicked( int ) ),
           ui_dsoWid, SLOT( setMathModeSLOT( int ) ));
      
  // channel enable
  connect( ui_ch1Enable, SIGNAL( toggled( bool ) ),
           ui_dsoWid, SLOT( setShowChannel1SLOT( bool ) ));
  connect( ui_ch2Enable, SIGNAL( toggled( bool ) ),
           ui_dsoWid, SLOT( setShowChannel2SLOT( bool ) ));
  connect( ui_ch1Enable, SIGNAL( toggled( bool ) ),
           this, SLOT( setChEnableColorSLOT() ));
  connect( ui_ch2Enable, SIGNAL( toggled( bool ) ),
           this, SLOT( setChEnableColorSLOT() ));
  
  // mode selection
  connect( ui_modeGroup, SIGNAL( clicked( int ) ),
           this, SLOT( modeSLOT( int ) ));
  
  // center and reset math
  connect( ui_centerMath, SIGNAL( clicked() ),
           this, SLOT( centerMathSLOT() ));
  connect( ui_resetStretchMath, SIGNAL( clicked() ),
           this, SLOT( resetMathSLOT() )); 
  
  // reset stretch channels
  connect( ui_resetStretchCh1, SIGNAL( clicked() ),
           this, SLOT( resetStretchCh1SLOT() ));
  connect( ui_resetStretchCh2, SIGNAL( clicked() ),
           this, SLOT( resetStretchCh2SLOT() ));
  
  // reset dc-offset
  connect( ui_centerDcCh1, SIGNAL( clicked() ),
           this, SLOT( resetDCOffsetCh1SLOT() ));
  connect( ui_centerDcCh2, SIGNAL( clicked() ),
           this, SLOT( resetDCOffsetCh2SLOT() ));

  // coupling mode
  connect( ui_couplingModeCh1, SIGNAL( activated( int ) ),
           this, SLOT( couplingModeCh1SLOT( int ) ));
  connect( ui_couplingModeCh2, SIGNAL( activated( int ) ),
           this, SLOT( couplingModeCh2SLOT( int ) ));
  
  // center channels
  connect( ui_centerCh1, SIGNAL( clicked() ),
           this, SLOT( resetOffsetCh1SLOT() ));
  connect( ui_centerCh2, SIGNAL( clicked() ),
           this, SLOT( resetOffsetCh2SLOT() ));
  
  // timebase
  connect( ui_timebase, SIGNAL( clicked( int ) ),
           this, SLOT( timeBaseChangedSLOT( int ) ));
  
  // VOLTS/div
  connect( ui_voltsGridCh1, SIGNAL( clicked( int ) ),
           this, SLOT( ch1VoltsDivChangedSLOT( int ) ));
  connect( ui_voltsGridCh2, SIGNAL( clicked( int ) ),
           this, SLOT( ch2VoltsDivChangedSLOT( int ) ));
  
  // channel stretching
  connect( ui_stretchCh1, SIGNAL( valueChanged( int ) ),
           this, SLOT( stretchCh1SLOT( int ) ));
  connect( ui_stretchCh2, SIGNAL( valueChanged( int ) ),
           this, SLOT( stretchCh2SLOT( int ) ));
  
  // dc-offset
  connect( ui_dcOffsetCh1, SIGNAL( valueChanged( int ) ),
           this, SLOT( dcOffsetCh1SLOT( int ) ));
  connect( ui_dcOffsetCh2, SIGNAL( valueChanged( int ) ),
           this, SLOT( dcOffsetCh2SLOT( int ) ));
  
  // probe factor
  connect( ui_probeCh1, SIGNAL( activated( int ) ),
           this, SLOT( probeCh1SLOT( int ) ));
  connect( ui_probeCh2, SIGNAL( activated( int ) ),
           this, SLOT( probeCh2SLOT( int ) ));
  
  // fft
  connect( ui_fftWindow, SIGNAL( activated( int ) ),
           ui_dsoWid, SLOT( setFftWindowSLOT( int ) ));
  connect( ui_fftWindow, SIGNAL( activated( int ) ),
           this, SLOT( fftWindowSLOT() ));
  connect( ui_fftFreq, SIGNAL( clicked( int ) ),
           this, SLOT( fftFreqRangeChangedSLOT( int ) ));
  connect( ui_fftZoom, SIGNAL( activated( int ) ),
           this, SLOT( fftZoomSLOT( int ) ));
  connect( ui_interpolation, SIGNAL( activated( int ) ),
           this, SLOT( interpolSLOT( int ) ));
  connect( ui_showEnvelope, SIGNAL( toggled( bool ) ),
           this, SLOT( setEnvelopeSLOT( bool ) ));
  connect( ui_resetEnvelope, SIGNAL( clicked() ),
           this, SLOT( resetEnvelopeSLOT() ));
  connect( ui_fftPostMag, SIGNAL( activated( int ) ),
           this, SLOT( fftPostMagSLOT( int ) ));
  connect( ui_fftType, SIGNAL( activated( int ) ),
           ui_dsoWid, SLOT( setFftTypeSLOT( int ) ));
  connect( ui_fftDisplayValue, SIGNAL( activated( int ) ),
           ui_dsoWid, SLOT( setFftDisplaySLOT( int ) ));
  connect( ui_fftSize, SIGNAL( activated( int ) ),
           this, SLOT( fftSizeSLOT( int ) ));
  connect( ui_fftShowPeaks, SIGNAL( toggled( bool ) ),
           ui_dsoWid, SLOT( setFftShowPeaksSLOT( bool ) ));
  connect( ui_freeFftLength, SIGNAL( valueChanged(int) ),
           ui_dsoWid, SLOT( setFftSizeFreeSLOT(int) ));
  connect( ui_fftScale, SIGNAL( activated(int) ),
           ui_dsoWid, SLOT( setFftScaleSLOT(int) ));
  
  // channel position
  connect( ui_yPosCh1, SIGNAL( valueChanged( int ) ),
           this, SLOT( yPosSLOT( int ) ));
  connect( ui_yPosCh2, SIGNAL( valueChanged( int ) ),
           this, SLOT( yPosSLOT( int ) ));
  
  // acquisition mode
  connect( ui_continuous, SIGNAL( toggled( bool ) ),
           this, SLOT( setContinuousSamplingSLOT( bool ) ));
  connect( ui_single, SIGNAL( clicked() ),
           this, SLOT( singleShotSLOT() ));
  connect( ui_fastAcq, SIGNAL( toggled( bool ) ),
           ui_dsoWid, SLOT( fastAcqSLOT( bool ) ));
  connect( ui_equivalentSampling, SIGNAL( toggled( bool ) ),
           ui_dsoWid, SLOT( equivalentSamplingSLOT( bool ) ));
  
  // average buffer reset
  connect( ui_resetFftDisplay, SIGNAL( clicked() ),
           ui_dsoWid, SLOT( flushFftAverageBufferSLOT() ));  
  
  // channel ops.
  //
  connect( ui_invCh1, SIGNAL( toggled( bool ) ),
           this, SLOT( channelOpSLOT() ));
  connect( ui_absCh1, SIGNAL( toggled( bool ) ),
           this, SLOT( channelOpSLOT() ));
  connect( ui_invCh2, SIGNAL( toggled( bool ) ),
           this, SLOT( channelOpSLOT() ));
  connect( ui_absCh2, SIGNAL( toggled( bool ) ),
           this, SLOT( channelOpSLOT() ));
    
  connect( ui_dsoWid, SIGNAL( dsoChanged() ),
           this, SLOT( requestCapabilitiesSLOT() ));
  
  // signal forwarding
  connect( ui_dsoWid, SIGNAL( dynamicRange( int, int, int, int, int ) ),
           this, SIGNAL( dynamicRange( int, int, int, int, int ) ));
  connect( ui_dsoWid, SIGNAL( fps( float ) ),
           this, SIGNAL( fps( float ) ));
  connect( ui_dsoWid, SIGNAL( triggerOk( bool ) ),
           this, SIGNAL( triggerOk( bool ) ));

  requestCapabilitiesSLOT();
    
  fftZoomSLOT( 1 );
  fftWindowSLOT();
  
  fontChange( font() );
  
  setChEnableColorSLOT();
  
  setDsoXRange();
}

MainWid::~MainWid()
{
}

void
MainWid::requestCapabilitiesSLOT()
{
  std::cerr << "request capabilities" << std::endl;
  
  ui_voltsGridCh1->clear();
  ui_voltsGridCh2->clear();

  for (int vd=Dso::VD20v; vd<=Dso::VD2mv; ++vd)
  {
    if (ui_dsoWid->dso()->hasVoltsDiv( (Dso::VoltsDiv)vd ))
    {
      ui_voltsGridCh1->addButton( ui_dsoWid->dso()->voltsDivText( (Dso::VoltsDiv)vd, false ),
                                  vd );
      ui_voltsGridCh2->addButton( ui_dsoWid->dso()->voltsDivText( (Dso::VoltsDiv)vd, false ),
                                  vd );
    }
  }
  
  ui_voltsGridCh1->setButton( Dso::VD5v );
  ui_voltsGridCh2->setButton( Dso::VD5v );
  
  QColor blue( 200, 200, 255 );
  QColor red( 255, 200, 200 );
  QColor green( 200, 255, 200 );
  
  ui_timebase->clear();
  
  for (int tb=Dso::TB100ms; tb<=Dso::TB10ns; ++tb)
  {
    if (ui_dsoWid->dso()->hasTimeBase( (Dso::TimeBase)tb ))
    {
      const int step = ui_dsoWid->dso()->step( (Dso::TimeBase)tb );
      
      if (step == 1)
      {
        ui_timebase->addButton( ui_dsoWid->dso()->timeBaseText( (Dso::TimeBase)tb ), 
                                tb, &blue );
      }
      else if (step > 0)
      {
        ui_timebase->addButton( ui_dsoWid->dso()->timeBaseText( (Dso::TimeBase)tb ), 
                                tb, &red );
      }
      else
      {
        if (step != ui_dsoWid->dso()->defaultStep())
        {
          ui_timebase->addButton( ui_dsoWid->dso()->timeBaseText( (Dso::TimeBase)tb ), 
                                  tb, &green );
        }
        else
        {
          ui_timebase->addButton( ui_dsoWid->dso()->timeBaseText( (Dso::TimeBase)tb ), 
                                  tb );
        }
      }
    }
  }
  ui_timebase->setButton( 6 );
  
  ui_fftFreq->clear();
  
  ui_fftFreq->addButton( "800Hz", 0 );
  ui_fftFreq->addButton( "1.6kHz", 1 );
  ui_fftFreq->addButton( "4kHz", 2 );
  ui_fftFreq->addButton( "8kHz", 3 );
  ui_fftFreq->addButton( "16kHz", 4 );
  ui_fftFreq->addButton( "40kHz", 5 );
  ui_fftFreq->addButton( "80kHz", 6 );
  ui_fftFreq->addButton( "160kHz", 7 );
  ui_fftFreq->addButton( "400kHz", 8 );
  ui_fftFreq->addButton( "800kHz", 9 );
  ui_fftFreq->addButton( "1.6MHz", 10 );
  ui_fftFreq->addButton( "4MHz", 11 );
  ui_fftFreq->addButton( "8MHz", 12 );
  ui_fftFreq->addButton( "16MHz", 13 );

  ui_fftFreq->setButton( 6 );  
  ui_fftGroup->hide();
  ui_fftParamGroup->hide();
  ui_fftDisplayGroup->hide();
  
  if (ui_dsoWid->dso()->hasDcOffset())
  {
    ui_dcOffsetCh1->show();
    ui_dcLabel1->show();
    ui_centerDcCh1->show();
    ui_dcOffsetCh2->show();
    ui_dcLabel2->show();
    ui_centerDcCh2->show();
  }
  else
  {
    ui_dcOffsetCh1->hide();
    ui_dcLabel1->hide();
    ui_centerDcCh1->hide();
    ui_dcOffsetCh2->hide();
    ui_dcLabel2->hide();
    ui_centerDcCh2->hide();
  }

  if (ui_dsoWid->dso()->hasCouplingSwitch())
  {  
    ui_couplingModeCh1->show();
    ui_couplingModeCh2->show();
  }
  else
  {  
    ui_couplingModeCh1->hide();
    ui_couplingModeCh2->hide();
  }

  if (ui_dsoWid->dso()->hasExternalTrigger())
  {  
    ui_extTrigger->show();
  }
  else
  {
    ui_extTrigger->hide();
  }

  if (ui_dsoWid->dso()->hasEquivalentSampling())
  {  
    ui_equivalentSampling->show();
  }
  else
  {
    ui_equivalentSampling->hide();
    ui_equivalentSampling->setOn( false );
    ui_dsoWid->equivalentSamplingSLOT( false );
  }

  if (ui_dsoWid->dso()->hasTriggerStabilization())
  {  
    ui_triggerStabilizer->show();
  }
  else
  {
    ui_triggerStabilizer->hide();
    ui_triggerStabilizer->setOn( false );
    ui_dsoWid->setTriggerStabilizerSLOT( false );
  }

  if (ui_dsoWid->dso()->hasFastAcq())
  {  
    ui_fastAcq->show();
  }
  else
  {
    ui_fastAcq->hide();
    ui_fastAcq->setOn( false );
    ui_dsoWid->fastAcqSLOT( false );
  }

  ui_interpolation->clear();
  ui_interpolation->insertItem( tr("Off"), DsoWid::Dots );
  ui_interpolation->insertItem( tr("Linear"), DsoWid::Linear );
  ui_interpolation->insertItem( tr("Linear Av."), DsoWid::LinearAverage );
  
  if (ui_dsoWid->dso()->needsSinXInterpol())
  {  
    ui_interpolation->insertItem( tr("Sin(x)/x"), DsoWid::SinX );
  }
  
  adjustSize();
}

void
MainWid::modeSLOT( int id )
{
  savePosSlider();
  
  m_mode = id;
  
  switch (id)
  {
  case 0:
    ui_dsoWid->setModeSLOT( DsoWid::DSO );
/*    ui_add->setEnabled( true );
    ui_sub->setEnabled( true ); */
    ui_xPos->setEnabled( true );
    ui_amplitudeMarker->setEnabled( true );
    ui_timeMarker->setEnabled( true );
    ui_triggerGroup->setEnabled( true );
    ui_ch1Enable->setEnabled( true );
    ui_ch2Enable->setEnabled( true );
    ui_dsoGroup->show();
    ui_fftGroup->hide();
   // ui_fftDisplayParamGroup->hide();
    ui_fftParamGroup->hide();
    //ui_interpolGroup->show();
    ui_mathGroup->show();
    ui_dsoDisplayGroup->show();
    ui_fftDisplayGroup->hide();
    setDsoXRange();
    break;
  case 1:
    ui_dsoWid->setModeSLOT( DsoWid::XY );
/*    ui_add->setOn( false );
    ui_sub->setOn( false );
    ui_add->setEnabled( false );
    ui_sub->setEnabled( false ); */
    ui_amplitudeMarker->setOn( false );
    ui_timeMarker->setOn( false );
    ui_amplitudeMarker->setEnabled( false );
    ui_timeMarker->setEnabled( false );
    ui_xPos->setEnabled( false );
    ui_ch1Enable->setEnabled( false );
    ui_ch2Enable->setEnabled( false );
    ui_dsoGroup->show();
    ui_fftGroup->hide();
    ui_fftParamGroup->hide();
    //ui_fftDisplayParamGroup->hide();
    //ui_interpolGroup->show();
    ui_mathGroup->hide();
    ui_dsoDisplayGroup->show();
    ui_fftDisplayGroup->hide();
    break;
  case 2:
    ui_dsoWid->setModeSLOT( DsoWid::FFT );
    ui_dsoGroup->hide();
    ui_fftGroup->show();
    ui_fftParamGroup->show();
    //ui_fftDisplayParamGroup->show();
    setFftZoomRange();
    //ui_fftZoomGroup->setButton( 1 );
    ui_ch1Enable->setEnabled( true );
    ui_ch2Enable->setEnabled( true );
    ui_xPos->setValue( 0 );
    //ui_interpolGroup->hide();
    ui_mathGroup->show();
    ui_dsoDisplayGroup->hide();
    ui_fftDisplayGroup->show();
    //fftZoomSLOT( 1 );   // call manually - Qt doensn't
    break;
  default: // oops!
    ui_dsoWid->setModeSLOT( DsoWid::DSO );
    ui_dsoGroup->show();
    ui_fftGroup->hide();
    ui_dsoDisplayGroup->show();
    ui_fftDisplayGroup->hide();
    ui_mathGroup->show();
    break;
  }
  
  setPosSlider();
}

void
MainWid::fftZoomSLOT( int id )
{
  switch (id)
  {
  case 0:
    ui_dsoWid->setFftZoomSLOT( 1 );
    break;
  case 1:
    ui_dsoWid->setFftZoomSLOT( 2 );
    break;
  case 2:
    ui_dsoWid->setFftZoomSLOT( 4 );
    break;
  case 3:
    ui_dsoWid->setFftZoomSLOT( 8 );
    break;
  }
  
  setFftZoomRange();
}

void
MainWid::fftPostMagSLOT( int id )
{
  switch (id)
  {
  case 0:
    ui_dsoWid->setFftPostMagSLOT( 1 );
    break;
  case 1:
    ui_dsoWid->setFftPostMagSLOT( 2 );
    break;
  case 2:
    ui_dsoWid->setFftPostMagSLOT( 5 );
    break;
  case 3:
    ui_dsoWid->setFftPostMagSLOT( 10 );
    break;
  }
}

void
MainWid::setFftZoomRange()
{
  switch (ui_dsoWid->fftZoom())
  {
  case 1:    
    ui_xPos->setRange( 0, 0 );
    break;
  case 2:    
    ui_xPos->setRange( 0, ui_dsoWid->fftSize()/4 );
    ui_xPos->setSteps( 1, ui_dsoWid->fftSize()/2 );    
    break;
  case 4:    
    ui_xPos->setRange( 0, ui_dsoWid->fftSize()/8*3 );
    ui_xPos->setSteps( 1, ui_dsoWid->fftSize()/4 );  
    break;
  case 8:    
    ui_xPos->setRange( 0, ui_dsoWid->fftSize()/16*7 );
    ui_xPos->setSteps( 1, ui_dsoWid->fftSize()/8 );  
    break;
  }
}

void
MainWid::centerMathSLOT()
{
  ui_yPosMath->setValue( 0 );
}

void
MainWid::resetMathSLOT()
{
  ui_stretchMath->setValue( 0 );
}

void
MainWid::resetStretchCh1SLOT()
{
  ui_stretchCh1->setValue( 0 );
}

void
MainWid::resetStretchCh2SLOT()
{
  ui_stretchCh2->setValue( 0 );
}

void
MainWid::resetOffsetCh1SLOT()
{
  ui_yPosCh1->setValue( 0 );
}

void
MainWid::resetOffsetCh2SLOT()
{
  ui_yPosCh2->setValue( 0 );
}

void
MainWid::resetDCOffsetCh1SLOT()
{
  ui_dcOffsetCh1->setValue( 127 );
}

void
MainWid::resetDCOffsetCh2SLOT()
{
  ui_dcOffsetCh2->setValue( 127 );
}

void 
MainWid::mathOffsetChangedSLOT( int val )
{
  ui_centerMath->setEnabled( val != 0 );
  ui_dsoWid->setMathOffSLOT( val );
}

void 
MainWid::mathStretchChangedSLOT( int val )
{
  ui_resetStretchMath->setEnabled( val != 0 );
  
  if (val < 0)
  {
    ui_dsoWid->setMathStretchSLOT( 1.+(double)(-val) / 100. );  
  }
  else
  {
    ui_dsoWid->setMathStretchSLOT( 1.+(double)(-val) / 200. );  
  }
}

void
MainWid::ch1VoltsDivChangedSLOT( int id )
{
  voltsDivChanged( 0, id );
}

void
MainWid::ch2VoltsDivChangedSLOT( int id )
{
  voltsDivChanged( 1, id );
}

void
MainWid::voltsDivChanged( int channel, int id )
{  
  m_voltsDivId[channel] = id;
  ui_dsoWid->setVoltsDivSLOT( channel, (Dso::VoltsDiv)id );
}

void
MainWid::fftFreqRangeChangedSLOT( int id )
{    
  ui_dsoWid->setFftFreqSLOT( (Dso::TimeBase)id );
  
  emit resetFps();
}

void
MainWid::timeBaseChangedSLOT( int id )
{
  ui_dsoWid->setTimeBaseSLOT( (Dso::TimeBase)id );  
  //setDsoXRange();
  
  emit resetFps();
}

void
MainWid::setDsoXRange()
{
  int shift = 4096-WINDOW_X_PIXELS*5;
  
  ui_xPos->setRange( 0, shift );
  ui_xPos->setSteps( 1, 16*32 );
  ui_xPos->setValue( shift/2 );
}

void
MainWid::triggerPressSLOT()
{
  ui_dsoWid->setDrawTriggerLineSLOT( true );
}

void
MainWid::triggerReleaseSLOT()
{
  ui_dsoWid->setDrawTriggerLineSLOT( false );
}

void
MainWid::stretchCh1SLOT( int value )
{
  if (value < 0)
  {
    ui_dsoWid->setStretchSLOT( 0, 1.+(double)(-value) / 100. );  
  }
  else
  {
    ui_dsoWid->setStretchSLOT( 0, 1.+(double)(-value) / 200. );
  }  
  ui_resetStretchCh1->setEnabled( value != 0 );
}

void
MainWid::stretchCh2SLOT( int value )
{
  if (value < 0)
  {
    ui_dsoWid->setStretchSLOT( 1, 1.+(double)(-value) / 100. );
  }
  else
  {
    ui_dsoWid->setStretchSLOT( 1, 1.+(double)(-value) / 200. );
  }
  ui_resetStretchCh2->setEnabled( value != 0 );
}

void
MainWid::dcOffsetCh1SLOT( int offset )
{
  ui_dsoWid->dso()->setDCOffset( 0, 255-offset );
  
  ui_centerDcCh1->setEnabled( offset != 127 );
}

void
MainWid::dcOffsetCh2SLOT( int offset )
{
  ui_dsoWid->dso()->setDCOffset( 1, 255-offset );
  
  ui_centerDcCh2->setEnabled( offset != 127 );
}

void
MainWid::couplingModeCh1SLOT( int mode )
{
  ui_dsoWid->dso()->setCouplingMode( 0, (Dso::CouplingMode)mode );
}

void
MainWid::couplingModeCh2SLOT( int mode )
{
  ui_dsoWid->dso()->setCouplingMode( 1, (Dso::CouplingMode)mode );
}

void
MainWid::probeCh1SLOT( int id )
{
  switch (id)
  {
  case 0:
    for (int vd=Dso::VD20v; vd<=Dso::VD2mv; ++vd)
    {
      if (ui_dsoWid->dso()->hasVoltsDiv( (Dso::VoltsDiv)vd ))
      {
        ui_voltsGridCh1->find( vd )->setText( ui_dsoWid->dso()->voltsDivText( (Dso::VoltsDiv)vd, false ) );
      }
    }
    m_fac[0] = 1.0;
    break;
  case 1:
    for (int vd=Dso::VD20v; vd<=Dso::VD2mv; ++vd)
    {
      if (ui_dsoWid->dso()->hasVoltsDiv( (Dso::VoltsDiv)vd ))
      {
        ui_voltsGridCh1->find( vd )->setText( ui_dsoWid->dso()->voltsDivText( (Dso::VoltsDiv)vd, true ) );
      }
    }
    m_fac[0] = 10.;
    break;
  }
  
  voltsDivChanged( 0, m_voltsDivId[0] );
  ui_dsoWid->setProbeSLOT( 0, m_fac[0] );
}

void
MainWid::probeCh2SLOT( int id )
{
  switch (id)
  {
  case 0:
    for (int vd=Dso::VD20v; vd<=Dso::VD2mv; ++vd)
    {
      if (ui_dsoWid->dso()->hasVoltsDiv( (Dso::VoltsDiv)vd ))
      {
        ui_voltsGridCh2->find( vd )->setText( ui_dsoWid->dso()->voltsDivText( (Dso::VoltsDiv)vd, false ) );
      }
    }
    m_fac[1] = 1.0;
    break;
  case 1:
    for (int vd=Dso::VD20v; vd<=Dso::VD2mv; ++vd)
    {
      if (ui_dsoWid->dso()->hasVoltsDiv( (Dso::VoltsDiv)vd ))
      {
        ui_voltsGridCh2->find( vd )->setText( ui_dsoWid->dso()->voltsDivText( (Dso::VoltsDiv)vd, true ) );
      }
    }
    m_fac[1] = 10.;
    break;
  }
  
  voltsDivChanged( 1, m_voltsDivId[1] );
  ui_dsoWid->setProbeSLOT( 1, m_fac[1] );
}

void
MainWid::fftSizeSLOT( int id )
{
  if (id == 6)
  {
    ui_dsoWid->setFftSizeFreeSLOT( ui_freeFftLength->value() );
    ui_freeFftLength->setEnabled( true );
  }
  else
  {
    ui_dsoWid->setFftSizeSLOT( id );
    ui_freeFftLength->setEnabled( false );
    setFftZoomRange();
  }
}

void
MainWid::interpolSLOT( int id )
{
  ui_dsoWid->setInterpolSLOT( (DsoWid::Interpolation)id );
}

void
MainWid::setEnvelopeSLOT( bool on )
{
  ui_dsoWid->setEnvelopeSLOT( on );
}

void
MainWid::resetEnvelopeSLOT()
{
  ui_dsoWid->resetEnvelopeSLOT();
}

void
MainWid::fftWindowSLOT()
{
  ui_dsoWid->setFftWindowNameSLOT( ui_fftWindow->currentText() );
}

void
MainWid::yPosSLOT( int value )
{
  if (sender() == ui_yPosCh1)
  {
    ui_dsoWid->setYOffsetSLOT( 0, value );
    ui_centerCh1->setEnabled( ui_yPosCh1->value() );
  }
  else
  {
    ui_dsoWid->setYOffsetSLOT( 1, value );
    ui_centerCh2->setEnabled( ui_yPosCh2->value() );    
  }
}

void
MainWid::setFrameRateSLOT( int fps )
{
  ui_dsoWid->setFrameRateSLOT( (DsoWid::FrameRate)fps );
}

int
MainWid::frameRate() const
{
  return ui_dsoWid->frameRate();
}

void
MainWid::singleShotSLOT()
{
  ui_continuous->setOn( false );
  ui_dsoWid->singleShotSLOT();
}

void
MainWid::setPosSlider()
{
  if (2 == m_mode) return;
  
  ui_yPosCh1->setValue( m_yPos[m_mode][0] );
  ui_yPosCh2->setValue( m_yPos[m_mode][1] );
  ui_stretchCh1->setValue( m_yStretch[m_mode][0] );
  ui_stretchCh2->setValue( m_yStretch[m_mode][1] );
}

void
MainWid::savePosSlider()
{
  if (2 == m_mode) return;
  
  m_yPos[m_mode][0] = ui_yPosCh1->value();
  m_yPos[m_mode][1] = ui_yPosCh2->value();
  m_yStretch[m_mode][0] = ui_stretchCh1->value();
  m_yStretch[m_mode][1] = ui_stretchCh2->value();
}

void 
MainWid::setFftBufferLengthSLOT( int length )
{
  ui_dsoWid->setFftBufferLength( length );
}

void 
MainWid::setDisplayFontSLOT( const QFont & fnt )
{
  ui_dsoWid->setFont( fnt );
}

void
MainWid::setTriggerChannelSLOT()
{
  if (sender() == ui_ch1Trigger)
  {
    ui_dsoWid->setTriggerChannelSLOT( 0 );
    ui_dsoWid->setTriggerExternalSLOT( false );
  }
  else if (sender() == ui_ch2Trigger)
  {
    ui_dsoWid->setTriggerChannelSLOT( 1 );
    ui_dsoWid->setTriggerExternalSLOT( false );
  }
  else
  {
    ui_dsoWid->setTriggerExternalSLOT( true );
  }
}

void
MainWid::setTriggerEdgeSLOT()
{
  ui_dsoWid->setTriggerRaisingSLOT( sender() == ui_raising );
}

void
MainWid::setFftDrawModeSLOT( int channel, DsoWid::FFTDrawMode mode )
{
  ui_dsoWid->setFftDrawModeSLOT( channel, mode );
}

void
MainWid::fontChange( const QFont & )
{
  QFont fnt = font();
  fnt.setBold( true );
  
  ui_ch1Enable->setFont( fnt );
  ui_ch2Enable->setFont( fnt );
}

void
MainWid::setChEnableColorSLOT()
{
  QPalette pal = palette();
  QColorGroup cg = palette().active();
  
  if (ui_ch1Enable->isOn())
  {
    cg.setColor( QColorGroup::Button, DsoWid::chColor[0] );
    pal.setActive( cg );
    pal.setInactive( cg );
  }
  else
  {
    cg.setColor( QColorGroup::Button, DsoWid::chColor[0].dark( 120 ) );
    pal.setActive( cg );
    pal.setInactive( cg );
  }
    
  ui_ch1Enable->setPalette( pal );
  
  if (ui_ch2Enable->isOn())
  {
    cg.setColor( QColorGroup::Button, DsoWid::chColor[1] );
    pal.setActive( cg );
    pal.setInactive( cg );
  }
  else
  {
    cg.setColor( QColorGroup::Button, DsoWid::chColor[1].dark( 120 ) );
    pal.setActive( cg );
    pal.setInactive( cg );
  }
    
  ui_ch2Enable->setPalette( pal );
  
  cg.setColor( QColorGroup::Button, DsoWid::addColor );
  pal.setActive( cg );
  pal.setInactive( cg );
  
  ui_mathAdd->setPalette( pal );
  
  cg.setColor( QColorGroup::Button, DsoWid::subColor );
  pal.setActive( cg );
  pal.setInactive( cg );
  
  ui_mathSub->setPalette( pal );
  
  cg.setColor( QColorGroup::Button, DsoWid::chChangedColor[0] );
  pal.setActive( cg );
  pal.setInactive( cg );
  
  ui_invCh1->setPalette( pal );
  ui_absCh1->setPalette( pal );
  
  cg.setColor( QColorGroup::Button, DsoWid::chChangedColor[1] );
  pal.setActive( cg );
  pal.setInactive( cg );
  
  ui_invCh2->setPalette( pal );
  ui_absCh2->setPalette( pal );
  
  emit channelsEnabled( ui_ch1Enable->isOn(), ui_ch2Enable->isOn() );
}

void
MainWid::setShowTimeMarkerSLOT( bool on )
{
  if (on)
  {
    ui_timeMarker2->setOn( false );
  }
  
  ui_dsoWid->setShowTimeMarkerSLOT( ui_timeMarker->isOn() ? DsoWid::TimeMarker :
         ( ui_timeMarker2->isOn() ? DsoWid::RatioMarker : DsoWid::Off ) );
}

void
MainWid::setShowTimeMarker2SLOT( bool on )
{
  if (on)
  {
    ui_timeMarker->setOn( false );
  }
  
  ui_dsoWid->setShowTimeMarkerSLOT( ui_timeMarker->isOn() ? DsoWid::TimeMarker :
         ( ui_timeMarker2->isOn() ? DsoWid::RatioMarker : DsoWid::Off ) );
}

void
MainWid::applyPrefsSLOT()
{
  setChEnableColorSLOT(); 
  
  ui_dsoWid->applyPrefsSLOT();
}

void
MainWid::channelOpSLOT()
{
  ui_dsoWid->setChannelOpSLOT( ui_invCh1->isOn(),
                               ui_absCh1->isOn(),
                               ui_invCh2->isOn(),
                               ui_absCh2->isOn() );
}

bool
MainWid::running() const
{
  return ui_continuous->isOn();
}

void
MainWid::setContinuousSamplingSLOT( bool on )
{
  emit sampleMode( on );
  
  ui_dsoWid->setContinuousSamplingSLOT( on );
}

void
MainWid::histogramSLOT()
{
  ui_dsoWid->histogramSLOT();
}

void
MainWid::dcOffsetSLOT()
{
  ui_dsoWid->dcOffsetSLOT();
}

void
MainWid::setDeviceSLOT( int dev )
{
  ui_dsoWid->selectModel( (Dso::Model)dev );
}
