//======================================================================
// File:		dso.cpp
// Author:	Matthias Toussaint
// Created:	Sat Jul  6 21:06:45 CEST 2002
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

#include <dso.h>

#include <iostream>

Dso::Dso( Model model, int bits ) :
  QThread(),
  m_model( model ),
  m_timeBase( TB1ms ),
  m_timeBaseSec( 0.001 ),
  m_triggerEnabled( false ),
  m_triggerChannel( 0 ),
  m_triggerExternal( false ),
  m_triggerRaising( true ),
  m_triggerLevel( -8 ),
  m_triggerOk( true ),
  m_fastAcq( false ),
  m_equivalentSampling( false ),
  m_acqLength( 4096 ),
  m_acqOffset( 0 )
{
  m_voltsDiv[0] = VD5v;
  m_voltsDiv[1] = VD5v;  
  m_voltsDivVolts[0] = 5;
  m_voltsDivVolts[1] = 5;  
    
  m_bits      = bits; 
  m_maxValue  = (1 << m_bits) - 1;
}

Dso::~Dso()
{
}

void
Dso::lock()
{
  m_mutex.lock();
}

void 
Dso::unlock()
{
  m_mutex.unlock();
}

void
Dso::setTimeBase( Dso::TimeBase timeBase )
{
  m_timeBase = timeBase;
}

void
Dso::setVoltsDiv( int channel, Dso::VoltsDiv voltsDiv )
{
  m_voltsDiv[channel] = voltsDiv;
}

void
Dso::setTriggerEnabled( bool on )
{
  m_triggerEnabled = on;
}

void
Dso::setTriggerChannel( int channel )
{
  m_triggerChannel = channel;
}

void
Dso::setTriggerExternal( bool ext )
{
  m_triggerExternal = ext;
}

void
Dso::setTriggerRaising( bool on )
{
  m_triggerRaising = on;
}

void
Dso::setTriggerLevel( int level )
{
  m_triggerLevel = level;
}

void
Dso::setCouplingMode( int, CouplingMode )
{
}

void
Dso::setDCOffset( int ch, int off )
{
  std::cerr << "ch: " << ch << " " << off << std::endl;
}

QString
Dso::voltsDivText( Dso::VoltsDiv vd, bool probe10 ) const
{
  QString txt;
  
  switch (vd)
  {
  case VD50v:
    if (!probe10)
    {
      txt = "50V";
    }
    else
    {
      txt = "500V";
    }
    break;
  case VD20v:
    if (!probe10)
    {
      txt = "20V";
    }
    else
    {
      txt = "200V";
    }
    break;
  case VD10v:
    if (!probe10)
    {
      txt = "10V";
    }
    else
    {
      txt = "100V";
    }
    break;
  case VD5v:
    if (!probe10)
    {
      txt = "5V";
    }
    else
    {
      txt = "50V";
    }
    break;
  case VD2v:
    if (!probe10)
    {
      txt = "2V";
    }
    else
    {
      txt = "20V";
    }
    break;
  case VD1v:
    if (!probe10)
    {
      txt = "1V";
    }
    else
    {
      txt = "10V";
    }
    break;
  case VD500mv:
    if (!probe10)
    {
      txt = "0.5V";
    }
    else
    {
      txt = "5V";
    }
    break;
  case VD200mv:
    if (!probe10)
    {
      txt = "0.2V";
    }
    else
    {
      txt = "2V";
    }
    break;
  case VD100mv:
    if (!probe10)
    {
      txt = "0.1V";
    }
    else
    {
      txt = "1V";
    }
    break;
  case VD50mv:
    if (!probe10)
    {
      txt = "50mV";
    }
    else
    {
      txt = "0.5V";
    }
    break;
  case VD20mv:
    if (!probe10)
    {
      txt = "20mV";
    }
    else
    {
      txt = "0.2V";
    }
    break;
  case VD10mv:
    if (!probe10)
    {
      txt = "10mV";
    }
    else
    {
      txt = "0.1V";
    }
    break;
  case VD5mv:
    if (!probe10)
    {
      txt = "5mV";
    }
    else
    {
      txt = "50mV";
    }
    break;
  case VD2mv:
    if (!probe10)
    {
      txt = "2mV";
    }
    else
    {
      txt = "20mV";
    }
    break;
  case VD1mv:
    if (!probe10)
    {
      txt = "1mV";
    }
    else
    {
      txt = "10mV";
    }
    break;
  }
  
  return txt;
}

QString
Dso::timeBaseText( Dso::TimeBase tb ) const
{
  QString txt;
  
  switch (tb)
  {
  case TB1s:
    txt = "1s";
    break;
  case TB500ms:
    txt = "0.5s";
    break;
  case TB200ms:
    txt = "0.2s";
    break;
  case TB100ms:
    txt = "0.1s";
    break;
  case TB50ms:
    txt = "50ms";
    break;
  case TB20ms:
    txt = "20ms";
    break;
  case TB10ms:
    txt = "10ms";
    break;
  case TB5ms:
    txt = "5ms";
    break;
  case TB2ms:
    txt = "2ms";
    break;
  case TB1ms:
    txt = "1ms";
    break;
  case TB500us:
    txt = "0.5ms";
    break;
  case TB200us:
    txt = "0.2ms";
    break;
  case TB100us:
    txt = "0.1ms";
    break;
  case TB50us:
    txt = "50\265s";
    break;
  case TB20us:
    txt = "20\265s";
    break;
  case TB10us:
    txt = "10\265s";
    break;
  case TB5us:
    txt = "5\265s";
    break;
  case TB2us:
    txt = "2\265s";
    break;
  case TB1us:
    txt = "1\265s";
    break;
  case TB500ns:
    txt = "0.5\265s";
    break;
  case TB200ns:
    txt = "0.2\265s";
    break;
  case TB100ns:
    txt = "0.1\265s";
    break;
  case TB50ns:
    txt = "50ns";
    break;
  case TB20ns:
    txt = "20ns";
    break;
  case TB10ns:
    txt = "10ns";
    break;
  }
  
  return txt;
}
