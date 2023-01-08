//======================================================================
// File:		simulatorwid.cpp
// Author:	Matthias Toussaint
// Created:	Sat Aug 31 14:42:37 CEST 2002
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

#include <simulatorwid.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qslider.h>

#include <iostream>

SimulatorWid::SimulatorWid( QWidget *parent, const char *name ) :
  UISimulatorWid( parent, name )
{
  m_waveform[0] = m_waveform[1] = Simulator::Sine;
  
  connect( ui_waveformCh1, SIGNAL( clicked( int ) ),
           this, SLOT( waveform1ChangedSLOT( int ) ));
  connect( ui_waveformCh2, SIGNAL( clicked( int ) ),
           this, SLOT( waveform2ChangedSLOT( int ) ));
  connect( ui_freqCh1, SIGNAL( valueChanged( int ) ),
           this, SLOT( freq1ChangedSLOT() ));
  connect( ui_freqCh2, SIGNAL( valueChanged( int ) ),
           this, SLOT( freq2ChangedSLOT() ));
  connect( ui_vCh1, SIGNAL( valueChanged( int ) ),
           this, SLOT( v1ChangedSLOT() ));
  connect( ui_vCh2, SIGNAL( valueChanged( int ) ),
           this, SLOT( v2ChangedSLOT() ));
  connect( ui_symetryCh1, SIGNAL( valueChanged( int ) ),
           this, SLOT( symetry1ChangedSLOT( int ) ));
  connect( ui_symetryCh2, SIGNAL( valueChanged( int ) ),
           this, SLOT( symetry2ChangedSLOT( int ) ));
  connect( ui_vUnitCh1, SIGNAL( activated( int ) ),
           this, SLOT( v1ChangedSLOT() ));
  connect( ui_vUnitCh2, SIGNAL( activated( int ) ),
           this, SLOT( v2ChangedSLOT() ));
  connect( ui_noiseCh1, SIGNAL( toggled( bool ) ),
           this, SLOT( noise1ToggledSLOT( bool ) ));
  connect( ui_noiseCh2, SIGNAL( toggled( bool ) ),
           this, SLOT( noise2ToggledSLOT( bool ) ));
  connect( ui_jitterCh1, SIGNAL( toggled( bool ) ),
           this, SLOT( jitter1ToggledSLOT( bool ) ));
  connect( ui_jitterCh2, SIGNAL( toggled( bool ) ),
           this, SLOT( jitter2ToggledSLOT( bool ) ));
  connect( ui_phaseJitterCh1, SIGNAL( toggled( bool ) ),
           this, SLOT( phaseJitter1ToggledSLOT( bool ) ));
  connect( ui_phaseJitterCh2, SIGNAL( toggled( bool ) ),
           this, SLOT( phaseJitter2ToggledSLOT( bool ) ));
  
  adjustSize();
}

SimulatorWid::~SimulatorWid()
{
}

float
SimulatorWid::vpp( unsigned channel ) const
{
  float v = 0.0f;
  
  if (0 == channel)
  {
    v = (float)ui_vCh1->value();
    
    v /= ( ui_vUnitCh1->currentItem() == 0 ? 1000.0f : 1.0f );
  }
  else
  {
    v = (float)ui_vCh2->value();
    
    v /= ( ui_vUnitCh2->currentItem() == 0 ? 1000.0f : 1.0f );
  }
  
  return v;
}

float
SimulatorWid::frequency( unsigned channel ) const
{
  float freq = 0.;
  
  if (0 == channel)
  {
    freq = (float)ui_freqCh1->value();
  }
  else
  {
    freq = (float)ui_freqCh2->value();
  }
  
  return freq;
}

void
SimulatorWid::waveform1ChangedSLOT( int id )
{
  waveformChangedSLOT( id, 0 );
}

void
SimulatorWid::waveform2ChangedSLOT( int id )
{
  waveformChangedSLOT( id, 1 );
}

void
SimulatorWid::freq1ChangedSLOT()
{
  emit frequencyChanged( 0, frequency( 0 ) );
}

void
SimulatorWid::freq2ChangedSLOT()
{
  emit frequencyChanged( 1, frequency( 1 ) );
}

void
SimulatorWid::v1ChangedSLOT()
{
  emit vppChanged( 0, vpp( 0 ) );
}

void
SimulatorWid::v2ChangedSLOT()
{
  emit vppChanged( 1, vpp( 1 ) );
}

void
SimulatorWid::noise1ToggledSLOT( bool on )
{
  emit noiseToggled( 0, on );
}

void
SimulatorWid::noise2ToggledSLOT( bool on )
{
  emit noiseToggled( 1, on );
}

void
SimulatorWid::jitter1ToggledSLOT( bool on )
{
  emit jitterToggled( 0, on );
}

void
SimulatorWid::jitter2ToggledSLOT( bool on )
{
  emit jitterToggled( 1, on );
}

void
SimulatorWid::phaseJitter1ToggledSLOT( bool on )
{
  emit phaseJitterToggled( 0, on );
}

void
SimulatorWid::phaseJitter2ToggledSLOT( bool on )
{
  emit phaseJitterToggled( 1, on );
}

void
SimulatorWid::symetry1ChangedSLOT( int value )
{
  emit symetryChanged( 0, value );
}

void
SimulatorWid::symetry2ChangedSLOT( int value )
{
  emit symetryChanged( 1, value );
}

void
SimulatorWid::waveformChangedSLOT( int id, unsigned channel )
{
  switch (id)
  {
  case 0:
    m_waveform[channel] = Simulator::Sine;
    break;
  case 1:
    m_waveform[channel] = Simulator::Square;
    break;
  case 2:
    m_waveform[channel] = Simulator::Sawtooth;
    break;
  }
  
  emit waveformChanged( channel, m_waveform[channel] );
}

