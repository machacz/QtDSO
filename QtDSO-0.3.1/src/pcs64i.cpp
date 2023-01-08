//======================================================================
// File:		pcs64i.cpp
// Author:	Matthias Toussaint
// Created:	Mon Jun 10 17:23:31 CEST 2002
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

#include <pcs64i.h>
#include <math.h>
#include <iostream>
#include <qdatetime.h>

#include <string.h>
#include <errno.h>

#include <stdlib.h>

Pcs64i::Pcs64i() :
  Dso( Dso::PCS64i, 8 )
{
  m_numSamples = 4096;
  m_preTriggerSize = 1024;
  
  m_outBuffer[0] = new unsigned [m_numSamples];
  m_outBuffer[1] = new unsigned [m_numSamples];
  
  m_buffer[0] = new unsigned [m_numSamples];
  m_buffer[1] = new unsigned [m_numSamples];
  
  int ports = ieee1284_find_ports( &m_list, 0 );
  
  if (E1284_OK == ports)
  {
    if (0 == m_list.portc)
    {
      std::cerr << "Couldn't find ports" << std::endl;
      abort();
    }
    else
    {
      std::cerr << "find ports: " << m_list.portc << " Using first" << std::endl;
      std::cerr << "name: " << m_list.portv[0]->name << std::endl;
      std::cerr << "device: " << m_list.portv[0]->filename << std::endl;
      m_port = m_list.portv[0];
    }
  }
  else
  {
    std::cerr << "ports fucked up!" << std::endl;
    abort();
  }
  
  setTimeBase( Dso::TB1ms );
  setVoltsDiv( 0, Dso::VD5v );
  setVoltsDiv( 1, Dso::VD5v );
  setTriggerEnabled( false );
  setTriggerChannel( 0 );
  setTriggerRaising( true );
  setTriggerLevel( 128 );
  
  m_channelEnable[0] = true;
  m_channelEnable[1] = true;
  
  m_defaultStep = -5;
  
  calibrate();
  
  connectToHardware();
}

Pcs64i::~Pcs64i()
{
  ieee1284_release( m_port );
  ieee1284_free_ports( &m_list );
}

bool
Pcs64i::hasTimeBase( Dso::TimeBase timeBase ) const
{
  return (timeBase >= Dso::TB100ms && timeBase <= Dso::TB100ns);
}

bool
Pcs64i::hasVoltsDiv( Dso::VoltsDiv voltsDiv ) const
{
  return (voltsDiv >= Dso::VD5v && voltsDiv <= Dso::VD10mv);
}

void 
Pcs64i::setChannelEnable( int channel, bool on )
{
  m_channelEnable[channel] = on;
}

void
Pcs64i::init()
{
  lockParam();
  
  setLatchData( LATCH_TIMEBASE, m_timeBaseByte );
  setLatchData( LATCH_VOLTS_DIV, m_voltsDivByte[0] | m_voltsDivByte[1] ); 
  setLatchData( LATCH_TRIGGER, m_triggerByte );
  
  unlockParam();
}

void
Pcs64i::reset()
{
  setLatchData( LATCH_RESET, 0 );
}

void
Pcs64i::run()
{
  //while (1)
  {
    readSampledData();

    // synchronize with reading from widget
    //
    lock();

    memcpy( m_outBuffer[0], m_buffer[0], 4096*sizeof(unsigned) );
    memcpy( m_outBuffer[1], m_buffer[1], 4096*sizeof(unsigned) );

    m_outTriggerOffset = m_triggerOffset;
    m_hasNewData = true;
    m_triggerOk = true;

    unlock();
  }
  
  //exit();
}

void
Pcs64i::readSampledData()
{
  if (m_equivalentSampling)
  {
    unsigned buffer0[4096];
    unsigned buffer1[4096];
    
    setTriggerDelay( false );

    init();    
    reset();
    
    readData();
    
    // copy even samples
    //
    /*for (int i=0; i<2048; ++i)
    {
      buffer0[2*i+1] = m_buffer[0][i+2+512];
      buffer1[2*i+1] = m_buffer[1][i+2+512];
  }*/
    
    for (int i=0; i<4096; ++i)
    {
      buffer0[i] = m_buffer[0][i];
    }
    
    // do it again with delay switched on
    //
    setTriggerDelay( true );
    init();    
    reset();

    if (readData())
    {  
      // copy even samples
      //
      /*for (int i=0; i<2048; ++i)
      {
        buffer0[2*i] = m_buffer[0][i+512];
        buffer1[2*i] = m_buffer[1][i+512];
    }*/
      
      for (int i=0; i<4096; ++i)
      {
        m_buffer[1][i] = m_buffer[0][i];
        m_buffer[0][i] = buffer0[i];
      }
      
      // copy back
      //
      //memcpy( m_buffer[0], buffer0, 4096*sizeof(unsigned) );
      //memcpy( m_buffer[1], buffer1, 4096*sizeof(unsigned) );
    
    }
  }
  else
  {
    setTriggerDelay( false );

    init();    
    reset();
    
    readData();
    
    int level = triggerLevel();
    m_triggerOffset = 0;
    int tc = triggerChannel();
    //std::cerr << "LEVEL=" << level << " CHANNEL=" << tc << std::endl;
    
    std::cerr << "step=" << m_step << std::endl;
    // interpolating only
    // Assumption: Trigger is too late. Look back to find the offset
    //             starting at pretrigger index
    if (m_step > 0)
    {   
      if (triggerRaising())
      {
        for (int i=1024; i>800; --i)
        {
          int delta = m_buffer[tc][i]-m_buffer[tc][i-1];
          
          if (delta > 0 && m_buffer[tc][i] > level && m_buffer[tc][i-1] < level)
          {
            if (i <= 800) 
            {
              std::cerr << "FUCKUP: " << i-1024 << std::endl;
              sleep(2);
            }
            
            m_triggerOffset = m_step*(i-1024);
                      
            //std::cerr << "off=" << m_triggerOffset;
            float fac = (float)(level-m_buffer[tc][i-1]) / (float)delta;
            int dx = (int)qRound((float)m_step * fac);
            m_triggerOffset += dx;
            //std::cerr << " delta=" << delta << " dx=" << dx << " fac=" << fac 
            //          << " step=" << m_step << std::endl;
            break;
          }
        }
      }
      else
      {
        for (int i=1024; i>800; --i)
        {
          int delta = m_buffer[tc][i]-m_buffer[tc][i-1];
          
          if (delta < 0 && m_buffer[tc][i] < level && m_buffer[tc][i-1] > level)
          {
            if (i <= 800) 
            {
              std::cerr << "FUCKUP: " << i-1024 << std::endl;
              sleep(2);
            }
            
            m_triggerOffset = m_step*(i-1023);
                      
            std::cerr << "off=" << m_triggerOffset;
            float fac = (float)(m_buffer[tc][i-1]-level) / (float)delta;
            int dx = (int)qRound((float)m_step * fac);
            m_triggerOffset += dx;
            std::cerr << " delta=" << delta << " dx=" << dx << " fac=" << fac 
                << " step=" << m_step << std::endl;
            break;
          }
        }
      }
    }
  }
}

bool
Pcs64i::readData()
{
  struct timeval timeout;
    
  // juice to the optocoupler before reading
  //
  ieee1284_write_data( m_port, 0xf8 );

  // timeout is time needed for sampling + 0.5s
  //
  unsigned long usecs = (unsigned long)((4096. / m_samplingFrequency) * 1000000. + 500000.);
  unsigned long secs = usecs / 1000000L;
  usecs %= 1000000L;

  timeout.tv_sec = secs;
  timeout.tv_usec = usecs;

  if (E1284_TIMEDOUT != 
      ieee1284_wait_status( m_port, 0x80, 0, &timeout )) // if not timeout
  {
    const int lengthPlus = m_acqOffset+m_acqLength;
    
    // forewind (writing as fast as we can seems no problem)
    //
    for (int i=0; i<m_acqOffset; ++i)
    {
      ieee1284_write_data( m_port, 0xff ); // 11111111
      ieee1284_write_data( m_port, 0xff ); // 11111111
      
      ieee1284_write_data( m_port, 0xf8 ); // 11111000
      ieee1284_write_data( m_port, 0xf8 ); // 11111000
    }
      
    // read data from dso (channel wise)
    // With a _fast_ computer connected communication screws 
    // up completely. A delay between write_data and
    // read_status healed the problem (NEEDS MORE TESTING)
    //
    for (int i=m_acqOffset; i<lengthPlus; ++i)
    {
      if (m_channelEnable[0])
      {
        ieee1284_write_data( m_port, 0xff ); // 11111111
        ieee1284_write_data( m_port, 0xff ); // 11111111
        
        ieee1284_read_status( m_port );
        m_buffer[0][i] = (ieee1284_read_status( m_port ) << 1) & 0xf0;
        
        ieee1284_write_data( m_port, 0xfe ); // 11111110
        ieee1284_write_data( m_port, 0xfe ); // 11111110
        
        ieee1284_read_status( m_port );
        m_buffer[0][i] |= (ieee1284_read_status( m_port ) >> 3) & 0x0f;
      }
      
      if (m_channelEnable[1])
      {
        ieee1284_write_data( m_port, 0xfd ); // 11111101
        ieee1284_write_data( m_port, 0xfd ); // 11111101
        
        ieee1284_read_status( m_port );
        m_buffer[1][i] = (ieee1284_read_status( m_port ) << 1) & 0xf0;
        
        ieee1284_write_data( m_port, 0xfc ); // 11111100
        ieee1284_write_data( m_port, 0xfc ); // 11111100
        
        ieee1284_read_status( m_port );
        m_buffer[1][i] |= (ieee1284_read_status( m_port ) >> 3) & 0x0f;
      }
      
      ieee1284_write_data( m_port, 0xf8 ); // 11111000
      ieee1284_write_data( m_port, 0xf8 ); // 11111000
    }   
      
    // go to the end 
    // (It's my impression that the PCS64i gives more stable
    // results if we forewind to the end. Maybe a bug in QtDSO
    // or a proerty of the scope)
    for (int i=lengthPlus; i<m_numSamples; ++i)
    {
      ieee1284_write_data( m_port, 0xff ); // 11111111
      ieee1284_write_data( m_port, 0xff ); // 11111111
      
      ieee1284_write_data( m_port, 0xf8 ); // 11111000
      ieee1284_write_data( m_port, 0xf8 ); // 11111000
    }
    
    return true;
  }
  else
  {
    m_triggerOk = false;
  }
  
  return false;
}
  
// just do something silly to waist cpu cycles
// hopefully compiler is not smart enough to
// optimize away ...
int
Pcs64i::delay(int duration)
{
  int a = 0;
  for (int i=0; i<duration; ++i)
  {
    a += i;
  }
  
  return a;
}

void
Pcs64i::calibrate()
{
  QTime t;
  t.start();
  float perIt;
  for (int i=0; i<100000; ++i) delay( 2000 ); 
  
  perIt = (float)t.elapsed() / 100000.0f / 2000.0f;
  
  // ENGENEERING VALUE:
  // MY EDUCATED GUESS IS THAT WE NEED 0.0015s delay
  //
  m_readDelay = lrint( 0.0015f / perIt );
  
  std::cerr << "calibration delay = " << m_readDelay << std::endl;
}

void
Pcs64i::setDevice( const QString & /*devName*/ )
{
  //m_devName = devName;
}

int
Pcs64i::step( Dso::TimeBase tb ) const
{
  int step = -5;
  
  switch (tb)
  {
  case Dso::TB100ns:
    step = 10;
    break;
  case Dso::TB200ns:
    step = 5;
    break;
  case Dso::TB500ns:
    step = 2;
    break;
  case Dso::TB1us:
    step = 1;
    break;
  case Dso::TB2us:
    step = -2;
    break;
  default: // can't happen
    break;
  }
  
  return step;
}

void
Pcs64i::setTimeBase( Dso::TimeBase tb )
{
  lockParam();
  
  m_timeBase = tb;
  m_step = -5;
  m_samplingFrequency = 32000000.;
  m_timeBaseByte = 0x00;
  
  //    32MHz         0x00    00000000
  //    16MHz         0x20    00100000
  //     8MHz         0x5e    01011110
  //   3.2MHz         0x5d    01011101
  //   1.6MHz         0x5c    01011100
  //   800kHz         0x5a    01011010
  //   320kHz         0x59    01011001
  //   160kHz         0x58    01011000
  //    80kHz         0x52    01010010
  //    32kHz         0x51    01010001
  //    16kHz         0x50    01010000
  //     8kHz         0x42    01000010
  //   3.2kHz         0x41    01000001
  //   1.6kHz         0x40    01000000
  
  switch (tb)
  {
  case Dso::TB100ns:
    m_timeBaseSec = 0.0000001;
    m_step = 10;
    break;
  case Dso::TB200ns:
    m_timeBaseSec = 0.0000002;
    m_step = 5;
    break;
  case Dso::TB500ns:
    m_timeBaseSec = 0.0000005;
    m_step = 2;
    break;
  case Dso::TB1us:
    m_timeBaseSec = 0.000001;
    m_step = 1;
    break;
  case Dso::TB2us:
    m_timeBaseSec = 0.000002;
    m_step = -2;
    break;
  case Dso::TB5us:
    m_timeBaseSec = 0.000005;
    break;
  case Dso::TB10us:
    m_timeBaseByte = 0x20;
    m_samplingFrequency = 16000000.;
    m_timeBaseSec = 0.00001;
    break;
  case Dso::TB20us:
    m_timeBaseByte = 0x5E;
    m_samplingFrequency = 8000000.;
    m_timeBaseSec = 0.00002;
    break;
  case Dso::TB50us:
    m_timeBaseByte = 0x5D;
    m_samplingFrequency = 3200000.;
    m_timeBaseSec = 0.00005;
    break;
  case Dso::TB100us:
    m_timeBaseByte = 0x5C;
    m_samplingFrequency = 1600000.;
    m_timeBaseSec = 0.0001;
    break;
  case Dso::TB200us:
    m_timeBaseByte = 0x5A;
    m_samplingFrequency = 800000.;
    m_timeBaseSec = 0.0002;
    break;
  case Dso::TB500us:
    m_timeBaseByte = 0x59;
    m_samplingFrequency = 320000.;
    m_timeBaseSec = 0.0005;
    break;
  case Dso::TB1ms:
    m_timeBaseByte = 0x58;
    m_samplingFrequency = 160000.;
    m_timeBaseSec = 0.001;
    break;
  case Dso::TB2ms:
    m_timeBaseByte = 0x52;
    m_samplingFrequency = 80000.;
    m_timeBaseSec = 0.002;
    break;
  case Dso::TB5ms:
    m_timeBaseByte = 0x51;
    m_samplingFrequency = 32000.;
    m_timeBaseSec = 0.005;
    break;
  case Dso::TB10ms:
    m_timeBaseByte = 0x50;
    m_samplingFrequency = 16000.;
    m_timeBaseSec = 0.01;
    break;
  case Dso::TB20ms:
    m_timeBaseByte = 0x42;
    m_samplingFrequency = 8000.;
    m_timeBaseSec = 0.02;
    break;
  case Dso::TB50ms:
    m_timeBaseByte = 0x41;
    m_samplingFrequency = 3200.;
    m_timeBaseSec = 0.05;
    break;
  case Dso::TB100ms:
    m_timeBaseByte = 0x40;
    m_samplingFrequency = 1600.;
    m_timeBaseSec = 0.1;
    break;
  default: // can't happen
    m_timeBaseByte = 0x40;
    m_samplingFrequency = 1600.;
    m_timeBaseSec = 0.1;
    break;
  }
  
  unlockParam();
}

void
Pcs64i::setVoltsDiv( int channel, Dso::VoltsDiv vd )
{
  lockParam();
  
  m_voltsDiv[channel] = vd;
  
  switch (vd)
  {
  case Dso::VD10mv:
    m_voltsDivVolts[channel] = 0.01;
    m_voltsDivByte[channel] = 0x0A;
    break;
  case Dso::VD20mv:
    m_voltsDivVolts[channel] = 0.02;
    m_voltsDivByte[channel] = 0x06;
    break;
  case Dso::VD50mv:
    m_voltsDivVolts[channel] = 0.05;
    m_voltsDivByte[channel] = 0x0E;
    break;
  case Dso::VD100mv:
    m_voltsDivVolts[channel] = 0.1;
    m_voltsDivByte[channel] = 0x09;
    break;
  case Dso::VD200mv:
    m_voltsDivVolts[channel] = 0.2;
    m_voltsDivByte[channel] = 0x05;
    break;
  case Dso::VD500mv:
    m_voltsDivVolts[channel] = 0.5;
    m_voltsDivByte[channel] = 0x0D;
    break;
  case Dso::VD1v:
    m_voltsDivVolts[channel] = 1.0;
    m_voltsDivByte[channel] = 0x0B;
    break;
  case Dso::VD2v:
    m_voltsDivVolts[channel] = 2.0;
    m_voltsDivByte[channel] = 0x07;
    break;
  case Dso::VD5v:
    m_voltsDivVolts[channel] = 5.0;
    m_voltsDivByte[channel] = 0x0F;
    break;
  default: // can't happen
    m_voltsDivVolts[channel] = 5.0;
    m_voltsDivByte[channel] = 0x0F;
    break;
  } 
  
  if (1 == channel)
  {
    m_voltsDivByte[channel] <<= 4;
  }   
  
  unlockParam();
}

void
Pcs64i::setTriggerEnabled( bool on )
{
  lockParam();
  
  if (on)
  {
    m_triggerByte &= ~(0x40);
  }
  else
  {
    m_triggerByte |= 0x40;
  }
  
  unlockParam();
}

void
Pcs64i::setTriggerChannel( int channel )
{
  lockParam();
  
  m_triggerChannel = channel;
  
  m_triggerByte &= ~(0x10);
  
  if (0 == channel)
  {
    m_triggerByte |= 0x10;
  }
  
  unlockParam();
}

int 
Pcs64i::triggerChannel() const
{
  return (m_triggerByte & 0x10) ? 0 : 1;
}
    
void
Pcs64i::setTriggerRaising( bool raising )
{
  lockParam();
  
  m_triggerByte &= ~(0x20);
  
  if (raising)
  {
    m_triggerByte |= 0x20;
  }
 
  unlockParam();
}

bool 
Pcs64i::triggerRaising() const
{
  return (m_triggerByte & 0x20) ? true : false;
}

void
Pcs64i::setTriggerDelay( bool delay )
{
  lockParam();
  
  m_triggerByte &= ~(0x80);
  
  if (delay)
  {
    m_triggerByte |= 0x80;
  }
 
  unlockParam();
}

void
Pcs64i::setTriggerLevel( int level )
{
  lockParam();
  
  m_triggerLevel = level;
  
  level >>= 4;
  
  m_triggerByte &= 0xf0;
  m_triggerByte |= level & 0x0f;
  
  unlockParam();
}

int 
Pcs64i::triggerLevel() const
{
  return (m_triggerByte & 0x0f) << 4;
}

void
Pcs64i::setLatchData( Q_UINT8 latch, Q_UINT8 data )
{
  ieee1284_write_control( m_port, 6 );
  ieee1284_write_control( m_port, 6 );
  
  ieee1284_write_data( m_port, latch );
  ieee1284_write_data( m_port, latch );
  
  ieee1284_write_control( m_port, 4 );
  ieee1284_write_control( m_port, 4 );
  
  ieee1284_write_control( m_port, 6 );
  ieee1284_write_control( m_port, 6 );
  
  ieee1284_write_data( m_port, data );
  ieee1284_write_data( m_port, data );
  
  ieee1284_write_control( m_port, 2 );
  ieee1284_write_control( m_port, 2 );
  
  ieee1284_write_control( m_port, 6 );
  ieee1284_write_control( m_port, 6 );
  
  ieee1284_write_data( m_port, 0xf8 );
  ieee1284_write_data( m_port, 0xf8 );
}
  
bool
Pcs64i::connectToHardware()
{
  int port = ieee1284_open( m_port, 0, 0 );
  
  if (E1284_OK == port)
  {
    std::cerr << "ieee1284_open OK" << std::endl;
    
    int claim = ieee1284_claim( m_port );
    
    if (E1284_OK == claim)
    {
      std::cerr << "ieee1284_claim OK" << std::endl;
      
      // juice for the optocoupler and init
      // 
      ieee1284_write_data( m_port, 0xf8 );  
      init();
      
      return true;
    }
    else
    {
      std::cerr << "ieee1284_claim FAILED: ";
    
      switch (claim)
      {
      case E1284_NOMEM:
        std::cerr << "No memory left";
        break;
      case E1284_INVALIDPORT:
        std::cerr << "Invalid port";
        break;
      case E1284_SYS:
        std::cerr << "Error interfacing system: ";
        std::cerr << strerror( errno );
        break;
      }

      std::cerr << std::endl;
    }
  }
  else
  {
    std::cerr << "ieee1284_open FAILED: ";
    
    switch (port)
    {
    case E1284_INIT:
      std::cerr << "Error initialising port";
      break;
    case E1284_NOMEM:
      std::cerr << "No memory left";
      break;
    case E1284_NOTAVAIL:
      std::cerr << "Not available on this system";
      break;
    case E1284_INVALIDPORT:
      std::cerr << "Invalid port";
      break;
    case E1284_NEGFAILED:
      std::cerr << "Negotiation went wrong";
      break;    
    case E1284_REJECTED:
      std::cerr << "IEEE 1284 negotiation rejected";
      break;    
    case E1284_SYS:
      std::cerr << "Error interfacing system: ";
      std::cerr << strerror( errno );
      break;
    }
    
    std::cerr << std::endl;
  }
    
  return false;
}

void
Pcs64i::lockParam()
{
  m_paramMutex.lock();
}

void
Pcs64i::unlockParam()
{
  m_paramMutex.unlock();
}
