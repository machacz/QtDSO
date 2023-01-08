//======================================================================
// File:		pcs64i.h
// Author:	Matthias Toussaint
// Created:	Mon Jun 10 17:10:19 CEST 2002
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

#ifndef PCS64I_HH
#define PCS64I_HH

#include <qglobal.h>
#include <dso.h>
#include <ieee1284.h>

#define LATCH_RESET       0x02
#define LATCH_VOLTS_DIV   0x06
#define LATCH_TIMEBASE    0x04
#define LATCH_TRIGGER     0x00

class Pcs64i : public Dso
{
public:
  Pcs64i();
  virtual ~Pcs64i();
  
  virtual void run();
  
  /// set the device name of the used port
  virtual void setDevice( const QString & );
  /// Timebase in us
  virtual void setTimeBase( Dso::TimeBase );
  /// volts/div in mv
  virtual void setVoltsDiv( int, Dso::VoltsDiv );
  /// enable trigger
  virtual void setTriggerEnabled( bool );
  /// should be 1 or 2
  virtual void setTriggerChannel( int );
  /// trigger on raising edge, else on falling
  virtual void setTriggerRaising( bool );
  /// trigger level
  virtual void setTriggerLevel( int );
  
  virtual void setChannelEnable( int, bool );
  
  virtual int triggerBits() const { return 0xf0; }  
  virtual int  numChannel() const { return 2; }
  virtual bool hasDcOffset() const { return false; }
  virtual bool hasTimeBase( Dso::TimeBase ) const;
  virtual bool hasVoltsDiv( Dso::VoltsDiv ) const;
  virtual bool hasExternalTrigger() const { return false; }
  virtual bool hasCouplingSwitch() const { return false; }
  virtual bool hasFastAcq() const { return true; }
  virtual bool needsSinXInterpol() const { return true; }
  virtual bool hasEquivalentSampling() const { return true; }
  virtual bool hasTriggerStabilization() const { return true; }
  
  virtual bool connectToHardware();
  
  virtual int step( Dso::TimeBase ) const;
  
protected:
  struct parport      *m_port;
  struct parport_list  m_list;
  Dso::TimeBase        m_timeBase;
  Q_UINT8              m_timeBaseByte;
  Q_UINT8              m_voltsDivByte[2];
  Q_UINT8              m_triggerByte;
  QMutex               m_paramMutex;
  bool                 m_channelEnable[2];
  int                  m_readDelay;
  
  void init();
  void setLatchData( Q_UINT8 latch, Q_UINT8 data );
  void reset();
  void lockParam();
  void unlockParam();
  void readSampledData();
  
  int delay(int);
  void calibrate();
  bool readData();
  
  void setTriggerDelay( bool );
  int triggerChannel() const;
  bool triggerRaising() const;
  int triggerLevel() const;
  
};

#endif // PCS64I_HH
