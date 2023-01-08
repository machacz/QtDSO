//======================================================================
// File:		dso.h
// Author:	Matthias Toussaint
// Created:	Mon Jun 10 17:13:44 CEST 2002
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

#ifndef DSO_HH
#define DSO_HH

#include <qthread.h>

/** Abstract base class for DSO protocoll
*/
class Dso : public QThread
{
public:
  enum TimeBase
  {
    TB1s = 0,
    TB500ms,
    TB200ms,
    TB100ms,
    TB50ms,
    TB20ms,
    TB10ms,
    TB5ms,
    TB2ms,
    TB1ms,
    TB500us,
    TB200us,
    TB100us,
    TB50us,
    TB20us,
    TB10us,
    TB5us,
    TB2us,
    TB1us,
    TB500ns,
    TB200ns,
    TB100ns,
    TB50ns,
    TB20ns,
    TB10ns
  };
    
  enum VoltsDiv
  {
    VD50v = 0,
    VD20v,
    VD10v,
    VD5v,
    VD2v,
    VD1v,
    VD500mv,
    VD200mv,
    VD100mv,
    VD50mv,
    VD20mv,        
    VD10mv,
    VD5mv,
    VD2mv,
    VD1mv
  };
   
  enum Model
  {
    Simulator = 0,
    PCS64i,
    PCS32,
    PCS100,
    PCS500,
    Unknown
  };
    
  enum CouplingMode
  {
    DC = 0,
    GND,
    AC
  };
    
  Dso( Model, int bits );
  virtual ~Dso();
   
  Dso::Model model() const { return m_model; }
  
  virtual void run() = 0;
  
  /// Timebase in us
  virtual void setTimeBase( Dso::TimeBase );
  /// volts/div in mv
  virtual void setVoltsDiv( int, Dso::VoltsDiv );
  /// enable trigger
  virtual void setTriggerEnabled( bool );
  virtual void setTriggerExternal( bool );
  /// should be 0, 1
  virtual void setTriggerChannel( int );
  /// trigger on raising edge, else on falling
  virtual void setTriggerRaising( bool );
  /// trigger level
  virtual void setTriggerLevel( int );
  /// connect to the selected hardware (port or whatsoever)
  virtual bool connectToHardware() = 0;
  /// set coupling mode dc, gnd or ac
  virtual void setCouplingMode( int channel, CouplingMode );
  virtual void setDCOffset( int channel, int offset );
  
  void setFastAcq( bool on ) { m_fastAcq = on; }
  bool fastAcq() const { return m_fastAcq; }
  void setEquivalentSampling( bool on ) { m_equivalentSampling = on; }
  bool equivalentSampling() const { return m_equivalentSampling; }
  void setAcqLength( int len ) { m_acqLength = len; }
  int acqLength() const { return m_acqLength; }
  void setAcqOffset( int off ) { m_acqOffset = off; }
  int acqOffset() const { return m_acqOffset; }
  int preTriggerSize() const { return m_preTriggerSize; }
  
  virtual void setChannelEnable( int, bool ) {};
  
  void lock();
  void unlock();
  
  // capabilities
  //
  virtual bool hasDcOffset() const = 0;
  virtual int  numChannel() const = 0;
  virtual int  triggerBits() const = 0;
  virtual bool hasEquivalentSampling() const = 0;
  virtual bool hasTriggerStabilization() const = 0;
  
  int  bits() const { return m_bits; };
  int  maxValue() const { return m_maxValue; }
  
  virtual bool hasTimeBase( Dso::TimeBase ) const = 0;
  virtual bool hasVoltsDiv( Dso::VoltsDiv ) const = 0;
  virtual bool hasExternalTrigger() const = 0;
  virtual bool hasCouplingSwitch() const = 0;
  virtual bool hasFastAcq() const = 0;
  virtual bool needsSinXInterpol() const = 0;
  QString voltsDivText( Dso::VoltsDiv, bool probe10 ) const;
  QString timeBaseText( Dso::TimeBase ) const;
  
  int data( int, int ) const;
  float samplingFrequency() const { return m_samplingFrequency; }
  float timeBase() const { return m_timeBaseSec; }  
  bool triggerOk() const { return m_triggerOk; }  
  int triggerChannel() const { return m_triggerChannel; }
  int triggerLevel() const { return m_triggerLevel; }
  int step() const { return m_step; }
  int numSamples() const { return m_numSamples; }
  float voltsDiv( int channel ) const { return m_voltsDivVolts[channel]; }
  bool hasNewData() const { return m_hasNewData; }
  void resetNewData() { m_hasNewData = false; }
  virtual int step( Dso::TimeBase ) const { return -5; };
  
  int defaultStep() const { return m_defaultStep; }
  int triggerOffset() const { return m_outTriggerOffset; }
  
protected:
  Model     m_model;
  unsigned *m_outBuffer[2];
  unsigned *m_buffer[2];
  TimeBase  m_timeBase;
  float     m_timeBaseSec;
  VoltsDiv  m_voltsDiv[2];
  float     m_voltsDivVolts[2];
  bool      m_triggerEnabled;
  int       m_triggerChannel;
  bool      m_triggerExternal;
  bool      m_triggerRaising;
  int       m_triggerLevel;
  QMutex    m_mutex;
  bool      m_triggerOk;
  int       m_step;
  int       m_defaultStep;
  float     m_samplingFrequency;
  int       m_numSamples;
  int       m_preTriggerSize;
  bool      m_hasNewData;
  bool      m_fastAcq;
  bool      m_equivalentSampling;
  int       m_acqLength;
  int       m_acqOffset;
  int       m_maxValue;
  int       m_bits;
  int       m_triggerOffset;
  int       m_outTriggerOffset;
  
};

inline int
Dso::data( int channel, int index ) const
{
  return m_outBuffer[channel][index];
}

#endif // DSO_HH
