//======================================================================
// File:		simulator.cpp
// Author:	Matthias Toussaint
// Created:	Sat Jul  6 21:07:22 CEST 2002
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

#include <simulator.h>
#include <math.h>

#include <iostream>

Simulator::Simulator() :
  Dso( Dso::Simulator, 8 )
{
  m_outBuffer[0] = new unsigned [4096];
  m_outBuffer[1] = new unsigned [4096];
  
  m_buffer[0] = new unsigned [4096];
  m_buffer[1] = new unsigned [4096];
  
  // use ring buffer to simulate pre-trigger
  // 1024 values pre trigger memory 
  // seems to be what Velleman PCi64 does
  //
  m_ringBuffer[0] = new unsigned [1024];
  m_ringBuffer[1] = new unsigned [1024];
  
  m_freq[0] = 100.;
  m_freq[1] = 100.;
  
  m_waveform[0] = Sine;
  m_waveform[1] = Sine;
  
  m_threshold[0] = 0.0;
  
  m_addNoise[0] = false;
  m_addNoise[1] = false;
  
  m_addJitter[0] = false;
  m_addJitter[1] = false;
  
  m_addPhaseJitter[0] = true;
  m_addPhaseJitter[1] = false;
  
  m_vPp[0] = 0.1;
  m_vPp[1] = 0.1;
  
  m_cnt = 0;
  
  m_acqLength = 4096;
  m_numSamples = 4096;
    
  setTimeBase( Dso::TB1ms );
  setVoltsDiv( 0, Dso::VD5v );
  setVoltsDiv( 1, Dso::VD5v );
  setTriggerEnabled( false );
  setTriggerChannel( 0 );
  setTriggerRaising( true );
  setTriggerLevel( 128 );
  
  m_halfValue = (m_maxValue+1) >> 1;

  for (int i=0; i<4096; ++i)
  {
    m_outBuffer[0][i] = m_outBuffer[1][i] = m_halfValue;
  }
  
  m_defaultStep = -5;
  
  m_preTriggerSize = 1024;
  
  // fill tables
  //
  float w = 0.0f;
  float step = M_PI/8192.0f;
  
  for (int i=0; i<16384; ++i)
  {
    m_sineTable[i] = cos( w );
    w += step;
  }
  
  for (int i=0; i<8192; ++i)
  {
    m_rectTable[i] = 1.0f;
  }  
  for (int i=8192; i<16384; ++i)
  {
    m_rectTable[i] = -1.0f;
  }
  
  w = 1.0f;
  step = 2.0f/8192.0f;
  
  for (int i=0; i<8192; ++i)
  {
    m_sawtoothTable[i] = w;
    w -= step;
  }  
  for (int i=8192; i<16384; ++i)
  {
    w += step;
    m_sawtoothTable[i] = w;
  }
  
  m_counter[0] = m_counter[1] = 0;
  m_counterStep[0][0] = m_counterStep[0][1] = 1;
  m_counterStep[1][0] = m_counterStep[1][1] = 1;
  
  m_dutyCycle[0] = m_dutyCycle[1] = 0.5;
}

Simulator::~Simulator()
{
}

bool
Simulator::hasTimeBase( Dso::TimeBase timeBase ) const
{
  return (timeBase >= Dso::TB100ms && timeBase <= Dso::TB100ns);
}

bool
Simulator::hasVoltsDiv( Dso::VoltsDiv voltsDiv ) const
{
  return (voltsDiv >= Dso::VD10v && voltsDiv <= Dso::VD5mv);
}

void
Simulator::waveformChangedSLOT( unsigned channel, Simulator::Waveform wf )
{
  m_waveform[channel] = wf;
}

void
Simulator::frequencyChangedSLOT( unsigned channel, float freq )
{
  m_freq[channel] = freq;
}

void
Simulator::vppChangedSLOT( unsigned channel, float vpp )
{
  m_vPp[channel] = vpp;
}

void
Simulator::jitterToggledSLOT( unsigned channel, bool on )
{
  m_addJitter[channel] = on;
}

void
Simulator::phaseJitterToggledSLOT( unsigned channel, bool on )
{
  m_addPhaseJitter[channel] = on;
}

void
Simulator::noiseToggledSLOT( unsigned channel, bool on )
{
  m_addNoise[channel] = on;
}

void
Simulator::symetryChangedSLOT( unsigned channel, int sym )
{
  m_dutyCycle[channel] = (10.0+sym)/20.0;
}

void
Simulator::run()
{
  //while (1)
  {
    //if (!m_hasNewData)
    {
      setupSimulator();

      // first fill buffer
      //
      for (int i=0; i<1024; ++i)
      {
        m_ringBuffer[0][i] = sample( 0 );
        m_ringBuffer[1][i] = sample( 1 );

        ++m_cnt;
      }

      // now wait for trigger condition
      // 
      if (m_triggerEnabled)
      {
        if (m_triggerRaising)
        {
          for (int i=0; i<4096; ++i)
          {
            m_ringBuffer[0][m_ringBufferPointer] = sample( 0 );
            m_ringBuffer[1][m_ringBufferPointer] = sample( 1 );

            ++m_cnt;
            ++m_ringBufferPointer;
            m_ringBufferPointer &= 0x3ff;

            if (raisingTrigger())
            {
              break;
            }
          }
        }
        else
        {
          for (int i=0; i<4096; ++i)
          {
            m_ringBuffer[0][m_ringBufferPointer] = sample( 0 );
            m_ringBuffer[1][m_ringBufferPointer] = sample( 1 );

            ++m_cnt;
            ++m_ringBufferPointer;
            m_ringBufferPointer &= 0x3ff;

            if (fallingTrigger())
            {
              break;
            }
          }
        }
      }

      // copy ring buffer into first values
      //
      for (int i=0; i<1024; ++i)
      {
        m_buffer[0][i] = m_ringBuffer[0][m_ringBufferPointer];
        m_buffer[1][i] = m_ringBuffer[1][m_ringBufferPointer];

        ++m_ringBufferPointer;
        m_ringBufferPointer &= 0x3ff;
      }

      // fill remaining samples
      //
      for (int i=1024; i<4096; ++i)
      {
        m_buffer[0][i] = sample( 0 ); 
        m_buffer[1][i] = sample( 1 ); 

        ++m_cnt;
      }

      // synchronize with reading from widget
      //
      m_mutex.lock();

      memcpy( m_outBuffer[0], m_buffer[0], 4096*sizeof(unsigned) );
      memcpy( m_outBuffer[1], m_buffer[1], 4096*sizeof(unsigned) );

      m_hasNewData = true;

      m_mutex.unlock();
    }
    
    //msleep( 20 );
  }
}

void
Simulator::setupSimulator()
{
  float ts = 1./160e3;
  float perDiv = m_halfValue/8.0f;
  
  m_ringBufferPointer = 0;

  switch (m_timeBase)
  {
  case TB100ns:
    ts = 32.0*1e7;
    break;  
  case TB200ns:
    ts = 32.0*5e6;
    break;  
  case TB500ns:
    ts = 32.0*2e6;
    break;  
  case TB1us:
    ts = 32.0*1e6;
    break;  
  case TB2us:
    ts = 32.0*5e5;
    break;  
  case TB5us:
    ts = 32.0*2e5;
    break;  
  case TB10us:
    ts = 32.0*1e5;
    break;
  case TB20us:
    ts = 32.0*5e4;
    break;  
  case TB50us:
    ts = 32.0*2e4;
    break;  
  case TB100us:
    ts = 32.0*1e4;
    break;  
  case TB200us:
    ts = 32.0*5e3;
    break;  
  case TB500us:
    ts = 32.0*2e3;
    break;  
  case TB1ms:
    ts = 32.0*1e3;
    break;  
  case TB2ms:
    ts = 32.0*5e2;
    break;  
  case TB5ms:
    ts = 32.0*2e2;
    break;  
  case TB10ms:
    ts = 32.0*1e2;
    break;  
  case TB20ms:
    ts = 32.0*5e1;
    break;  
  case TB50ms:
    ts = 32.0*2e1;
    break;  
  case TB100ms:
    ts = 32.0*1e1;
    break;  
  default:
    ts = 1.;
    break;  
  }

  for (int i=0; i<2; ++i)
  {
    switch (m_voltsDiv[i])
    {
    case VD10v:
      m_vScale[i] = m_vPp[i] * perDiv / 10.0f;  
      break;
    case VD5v:
      m_vScale[i] = m_vPp[i] * perDiv / 5.0f;   
      break;
    case VD2v:
      m_vScale[i] = m_vPp[i] * perDiv / 2.0f;  
      break;
    case VD1v:
      m_vScale[i] = m_vPp[i] * perDiv;
      break;
    case VD500mv:
      m_vScale[i] = m_vPp[i] * perDiv / 0.5f;  
      break;
    case VD200mv:
      m_vScale[i] = m_vPp[i] * perDiv / 0.2f;  
      break;
    case VD100mv:
      m_vScale[i] = m_vPp[i] * perDiv / 0.1f;  
      break;
    case VD50mv:
      m_vScale[i] = m_vPp[i] * perDiv / 0.05f;  
      break;
    case VD20mv:
      m_vScale[i] = m_vPp[i] * perDiv / 0.02f;  
      break;
    case VD10mv:
      m_vScale[i] = m_vPp[i] * perDiv / 0.01f;  
      break;
    case VD5mv:
      m_vScale[i] = m_vPp[i] * perDiv / 0.005f;  
      break;
    default:
      m_vScale[i] = m_vPp[i] * perDiv;  
      break;
    }
  }
  
 // m_vScale[0] *= (float)m_maxValue / 2.0f / m_halfValue;
 // m_vScale[1] *= (float)m_maxValue / 2.0f / m_halfValue;

  double t1 = 2.0/(double)m_freq[0];
  
  m_counterStep[0][0] = (unsigned)((double)(1.0/t1/m_dutyCycle[0])/ts*16384.0/5.0 *
                                  (double)0x40000);
  
  m_counterStep[1][0] = (unsigned)((double)(1.0/t1/(1.0-m_dutyCycle[0]))/ts*16384.0/5.0 *
                                  (double)0x40000);
  
  double t2 = 2.0/(double)m_freq[1];
  
  m_counterStep[0][1] = (unsigned)((double)(1.0/t2/m_dutyCycle[1])/ts*16384.0/5.0 *
                                  (double)0x40000);
  
  m_counterStep[1][1] = (unsigned)((double)(1.0/t2/(1.0-m_dutyCycle[1]))/ts*16384.0/5.0 *
                                  (double)0x40000);
}

unsigned 
Simulator::sample( int channel )
{
  // it's a dds generator
  // the table is 14 bit -> 18bit for the divider
  //
  unsigned addr = (m_counter[channel] >> 18) & 0x3fff;
  const int raise = (addr >= 8192 ? 1 : 0);
  m_counter[channel] += m_counterStep[raise][channel];
  
  addr = (m_counter[channel] >> 18) & 0x3fff;
  
  float ret;
  
  switch (m_waveform[channel])
  {
  case Sine:
    ret = m_sineTable[addr]*m_vScale[channel];
    break;
  case Square:
    ret = m_rectTable[addr]*m_vScale[channel];
    break;
  case Sawtooth:
    ret = m_sawtoothTable[addr]*m_vScale[channel];
    break;
  }
  
  if (m_addNoise[channel])
  {
    ret += ((float)random()/(float)RAND_MAX-0.5) / 50.0;
  }
  
  ret += m_halfValue;
  
  if (ret < 0.0f) return 0;
  if (ret > m_maxValue) return (unsigned)m_maxValue;
  
  return (unsigned)ret;
  //return (unsigned)(ret* (float)m_maxValue + m_halfValue);
}

bool
Simulator::raisingTrigger() const
{
  const unsigned level = (int)((float)(m_triggerLevel & triggerBits()) * 
                            (float)maxValue() / 255.0 );

  if (m_ringBuffer[m_triggerChannel][m_ringBufferPointer] >= level)
  {
    int pointer = m_ringBufferPointer;
    pointer += 1022;
    pointer &= 0x3ff;
    
    if (m_ringBuffer[m_triggerChannel][pointer] < level)
    { 
      return true;
    }
  }
  
  return false;
}

bool
Simulator::fallingTrigger() const
{
  const unsigned level = (int)((float)(m_triggerLevel & triggerBits()) * 
                            (float)maxValue() / 255.0);

  if (m_ringBuffer[m_triggerChannel][m_ringBufferPointer] <= level)
  {
    int pointer = m_ringBufferPointer;
    pointer += 1022;
    pointer &= 0x3ff;
    
    if (m_ringBuffer[m_triggerChannel][pointer] > level)
    { 
      return true;
    }
  }
  
  return false;
}
void
Simulator::setTimeBase( Dso::TimeBase tb )
{
  //lockParam();
  
  m_timeBase = tb;
  m_step = -5;
  m_samplingFrequency = 32000000.;
  
  switch (tb)
  {
  case Dso::TB100ns:
    m_timeBaseSec = 0.0000001;
    m_samplingFrequency = 1600000000.;
    break;
  case Dso::TB200ns:
    m_timeBaseSec = 0.0000002;
    m_samplingFrequency = 800000000.;
    break;
  case Dso::TB500ns:
    m_timeBaseSec = 0.0000005;
    m_samplingFrequency = 320000000.;
    break;
  case Dso::TB1us:
    m_timeBaseSec = 0.000001;
    m_samplingFrequency = 160000000.;
    break;
  case Dso::TB2us:
    m_timeBaseSec = 0.000002;
    m_samplingFrequency = 80000000.;
    break;
  case Dso::TB5us:
    m_timeBaseSec = 0.000005;
    m_samplingFrequency = 32000000.;
    break;
  case Dso::TB10us:
    m_samplingFrequency = 16000000.;
    m_timeBaseSec = 0.00001;
    break;
  case Dso::TB20us:
    m_samplingFrequency = 8000000.;
    m_timeBaseSec = 0.00002;
    break;
  case Dso::TB50us:
    m_samplingFrequency = 3200000.;
    m_timeBaseSec = 0.00005;
    break;
  case Dso::TB100us:
    m_samplingFrequency = 1600000.;
    m_timeBaseSec = 0.0001;
    break;
  case Dso::TB200us:
    m_samplingFrequency = 800000.;
    m_timeBaseSec = 0.0002;
    break;
  case Dso::TB500us:
    m_samplingFrequency = 320000.;
    m_timeBaseSec = 0.0005;
    break;
  case Dso::TB1ms:
    m_samplingFrequency = 160000.;
    m_timeBaseSec = 0.001;
    break;
  case Dso::TB2ms:
    m_samplingFrequency = 80000.;
    m_timeBaseSec = 0.002;
    break;
  case Dso::TB5ms:
    m_samplingFrequency = 32000.;
    m_timeBaseSec = 0.005;
    break;
  case Dso::TB10ms:
    m_samplingFrequency = 16000.;
    m_timeBaseSec = 0.01;
    break;
  case Dso::TB20ms:
    m_samplingFrequency = 8000.;
    m_timeBaseSec = 0.02;
    break;
  case Dso::TB50ms:
    m_samplingFrequency = 3200.;
    m_timeBaseSec = 0.05;
    break;
  case Dso::TB100ms:
    m_samplingFrequency = 1600.;
    m_timeBaseSec = 0.1;
    break;
  }
  
  //unlockParam();
}

void
Simulator::setVoltsDiv( int channel, Dso::VoltsDiv vd )
{
  //lockParam();
  
  m_voltsDiv[channel] = vd;
  
  switch (vd)
  {
  case Dso::VD5mv:
    m_voltsDivVolts[channel] = 0.005;
    break;
  case Dso::VD10mv:
    m_voltsDivVolts[channel] = 0.01;
    break;
  case Dso::VD20mv:
    m_voltsDivVolts[channel] = 0.02;
    break;
  case Dso::VD50mv:
    m_voltsDivVolts[channel] = 0.05;
    break;
  case Dso::VD100mv:
    m_voltsDivVolts[channel] = 0.1;
    break;
  case Dso::VD200mv:
    m_voltsDivVolts[channel] = 0.2;
    break;
  case Dso::VD500mv:
    m_voltsDivVolts[channel] = 0.5;
    break;
  case Dso::VD1v:
    m_voltsDivVolts[channel] = 1.0;
    break;
  case Dso::VD2v:
    m_voltsDivVolts[channel] = 2.0;
    break;
  case Dso::VD5v:
    m_voltsDivVolts[channel] = 5.0;
    break;
  case Dso::VD10v:
    m_voltsDivVolts[channel] = 10.0;
    break;
  } 
  
  //unlockParam();
}
