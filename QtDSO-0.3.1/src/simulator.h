//======================================================================
// File:		simulator.h
// Author:	Matthias Toussaint
// Created:	Sat Jul  6 21:02:10 CEST 2002
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

#ifndef SIMULATOR_HH
#define SIMULATOR_HH

#include <qobject.h>
#include <dso.h>
#include <stdlib.h>

class Simulator : public QObject, public Dso
{
  Q_OBJECT
      
public:
  enum Waveform
  {
    Sine = 0,
    Square,
    Sawtooth
  };
    
  Simulator();
  virtual ~Simulator();
  
  void run();
  
  void setFrequency( int, float );
  void setVpp( int, float );
  
  virtual bool connectToHardware() { return true; }
  /// Timebase in us
  virtual void setTimeBase( Dso::TimeBase );
  /// volts/div in mv
  virtual void setVoltsDiv( int, Dso::VoltsDiv );

  virtual int triggerBits() const { return 0xff; }  
  virtual int  numChannel() const { return 2; }
  virtual bool hasDcOffset() const { return false; }
  virtual bool hasTimeBase( Dso::TimeBase ) const;
  virtual bool hasVoltsDiv( Dso::VoltsDiv ) const;
  virtual bool hasExternalTrigger() const { return false; }
  virtual bool hasCouplingSwitch() const { return false; }
  virtual bool hasFastAcq() const { return false; }
  virtual bool needsSinXInterpol() const { return false; }
  virtual bool hasEquivalentSampling() const { return false; }
  virtual bool hasTriggerStabilization() const { return false; }
  
  virtual int step( Dso::TimeBase ) const { return -5; }
  
public slots:
  void waveformChangedSLOT( unsigned, Simulator::Waveform );
  void frequencyChangedSLOT( unsigned, float );
  void vppChangedSLOT( unsigned, float );
  void noiseToggledSLOT( unsigned, bool );
  void jitterToggledSLOT( unsigned, bool );
  void phaseJitterToggledSLOT( unsigned, bool );
  void symetryChangedSLOT( unsigned, int );
  
protected:
  float     m_freq[2];
  Waveform  m_waveform[2];
  bool      m_addNoise[2];
  bool      m_addJitter[2];
  bool      m_addPhaseJitter[2];
  float     m_vPp[2];
  float     m_vScale[2];
  float     m_threshold[2];
  int       m_cnt;
  unsigned *m_ringBuffer[2];
  int       m_ringBufferPointer;
  float     m_halfValue;
  float     m_sineTable[16384];
  float     m_rectTable[16384];
  float     m_sawtoothTable[16384];
  unsigned  m_counter[2];
  unsigned  m_counterStep[2][2];
  float     m_dutyCycle[2];
  
  void setupSimulator();
  unsigned sample( int );
  bool raisingTrigger() const;
  bool fallingTrigger() const;
  
  float noise( int ) const;
  unsigned clamp( float ) const;
  
};

inline float
Simulator::noise( int channel ) const
{
  return (m_addNoise[channel] ? 
          ((float)random()/(float)RAND_MAX*10.-5.) : 0.);
//          ((float)random()/(float)RAND_MAX-0.5) : 0.);
}

inline unsigned
Simulator::clamp( float value ) const
{
  return (unsigned)qRound( QMIN( (float)m_maxValue, QMAX( value, 0. ) ) );
}

#endif // SIMULATOR_HH
