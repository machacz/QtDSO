//======================================================================
// File:		dsovumeter.h
// Author:	Matthias Toussaint
// Created:	Sun Sep  8 12:43:10 CEST 2002
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

#ifndef DSOVUMETER_HH
#define DSOVUMETER_HH

#include <qwidget.h>

class DsoVuMeter : public QWidget
{
  Q_OBJECT
public:
  DsoVuMeter( int, int, 
              QWidget *parent=0, const char *name=0 );
  virtual ~DsoVuMeter();
  QSize sizeHint() const;
  
public slots:
  void setValuesSLOT( int maxValue, int min, int max );

protected:
  int m_channel;
  int m_maxValue;
  int m_min, m_max;
  
  void paintEvent( QPaintEvent * );
  
};

#endif // DSOVUMETER_HH
