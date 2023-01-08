//======================================================================
// File:		simulatorwid.h
// Author:	Matthias Toussaint
// Created:	Sat Aug 31 14:36:06 CEST 2002
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

#ifndef SIMULATORWID_HH
#define SIMULATORWID_HH

#include <uisimulatorwid.h>
#include <simulator.h>

class SimulatorWid : public UISimulatorWid
{
  Q_OBJECT
      
public:
  SimulatorWid( QWidget *parent=0, const char *name=0 );
  virtual ~SimulatorWid();
  
signals:
  void waveformChanged( unsigned, Simulator::Waveform );
  void frequencyChanged( unsigned, float );
  void vppChanged( unsigned, float );
  void noiseToggled( unsigned, bool );
  void jitterToggled( unsigned, bool );
  void phaseJitterToggled( unsigned, bool );
  void symetryChanged( unsigned, int );
  
protected:
  Simulator::Waveform m_waveform[2];

  inline Simulator::Waveform waveform( unsigned channel ) const 
  { return m_waveform[channel]; }
  float frequency( unsigned ) const;
  float vpp( unsigned ) const;
  
protected slots:
  void waveform1ChangedSLOT( int );
  void waveform2ChangedSLOT( int );
  void freq1ChangedSLOT();
  void freq2ChangedSLOT();
  void v1ChangedSLOT();
  void v2ChangedSLOT();
  void noise1ToggledSLOT( bool );
  void noise2ToggledSLOT( bool );
  void jitter1ToggledSLOT( bool );
  void jitter2ToggledSLOT( bool );
  void phaseJitter1ToggledSLOT( bool );
  void phaseJitter2ToggledSLOT( bool );
  void waveformChangedSLOT( int, unsigned );
  void symetry1ChangedSLOT( int value );
  void symetry2ChangedSLOT( int value );
  
};

#endif // SIMULATORWID_HH
