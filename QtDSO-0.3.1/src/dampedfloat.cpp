//======================================================================
// File:		dampedfloat.cpp
// Author:	Matthias Toussaint
// Created:	Sun Sep 22 02:35:58 CEST 2002
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

#include <dampedfloat.h>
#include <math.h>

#include <iostream>

DampedFloat::DampedFloat()
{
  reset();
}

DampedFloat::~DampedFloat()
{
}

void
DampedFloat::reset()
{
  m_value = 0.0f;
  m_valid = false;
}

void
DampedFloat::addValue( float val )
{
  if (fabs( m_value-val ) > fabs( m_value * 0.05 )) m_valid = false;
  else m_valid = true;
  
  if (!m_valid && m_value == 0.0f) m_value = val;
  else
  {
    // simple, but effective
    m_value = 0.7f*m_value + 0.3f*val;
  }
}
