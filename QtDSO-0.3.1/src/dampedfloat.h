//======================================================================
// File:		dampedfloat.h
// Author:	Matthias Toussaint
// Created:	Sun Sep 22 02:31:49 CEST 2002
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

#ifndef DAMPEDFLOAT_HH
#define DAMPEDFLOAT_HH

/** DampedFloat encapsulates a float value wich is damped through
    gliding average.
    
    This class is usefull for displays of measured values which 
    oscillate rapidly. This gives a more stable and human readable
    display.
*/
class DampedFloat
{
public:
  /** Contructor
  
      \param depth Size of the average window.
    */
  DampedFloat();
  /// Destructor
  ~DampedFloat();
  /** Clears the value and set to invalid.
  
      Call this member if you know that the value has significantly
      changed (e.g. measured range was changed). It resets the averaging
      resulting in a quick update to the new value.
    */
  void reset();
  /** Retrieve averaged value.
  
      \returns Averaged value.
    */
  const float value() const { return m_value; };
  /** Add a value to the averaging.
  
      \param value Add this value to the averaging window.
    */
  void addValue( float value );
  /// Returns true if value is valid.
  const bool valid() const { return m_valid; }
  
protected:
  /// Current damped value
  float m_value;
  /// True if current value is stable
  bool  m_valid;
};

#endif // DAMPEDFLOAT_HH
