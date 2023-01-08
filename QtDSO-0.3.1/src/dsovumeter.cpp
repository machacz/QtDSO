//======================================================================
// File:		dsovumeter.cpp
// Author:	Matthias Toussaint
// Created:	Sun Sep  8 12:46:10 CEST 2002
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

#include <qpainter.h>
#include <dsowid.h>
#include <dsovumeter.h>
#include <qpixmap.h>

#include <qdrawutil.h>

#include <iostream>

QSize
DsoVuMeter::sizeHint() const
{
  return QSize( 140, fontMetrics().height()+4 );
}

DsoVuMeter::DsoVuMeter( int channel, int maxValue,
                        QWidget *parent, const char *name ) :
  QWidget( parent, name ),
  m_channel( channel ),
  m_maxValue( maxValue ),
  m_min( 127 ),
  m_max( 127 )
{
  setBackgroundMode( QWidget::NoBackground );
}

DsoVuMeter::~DsoVuMeter()
{
}

void
DsoVuMeter::setValuesSLOT( int maxValue, int min, int max )
{
  m_min = min;
  m_max = max;
  m_maxValue = maxValue;
  
  update();
}

void
DsoVuMeter::paintEvent( QPaintEvent */*ev*/ )
{
  QPixmap doubleBuffer( width(), height() );
  
  QPainter p;
  p.begin(&doubleBuffer);
  
  if ((0 == m_min || m_maxValue == m_max) && isEnabled())
  {
    p.fillRect( 0, 0, width(), height(), Qt::red.light() );
  }
  else
  {
    p.fillRect( 0, 0, width(), height(), colorGroup().background() );
  }
  
  int tw = fontMetrics().width( tr("Ch2") ) + 2;  
  int w = width()-tw-4;
  int h = height()-4;
  
  if (0 == m_channel)
  {
    p.drawText( 2, 2, tw, h, Qt::AlignLeft | Qt::AlignVCenter,
                tr("Ch1") );
  }
  else
  {
    p.drawText( 2, 2, tw, h, Qt::AlignLeft | Qt::AlignVCenter,
                tr("Ch2") );
  }
  
  qDrawShadePanel( &p, 2+tw, 2, w, h, colorGroup(), true, 1, 0 );

  if (isEnabled())
  {
    float min = 3 + tw + (float)m_min / (float)m_maxValue * (float)(w-2);
    float max = 3 + tw + (float)m_max / (float)m_maxValue * (float)(w-2);

    int w2 = (int)qRound( max-min );
    if (w2 < 1) w2 = 1;

    p.fillRect( 3+tw, 3, w-2, h-2, DsoWid::chColor[m_channel].dark( 150 ) );
    p.fillRect( static_cast<int> (min), 5, 
                w2, h-6, DsoWid::chColor[m_channel] );
  }
  p.end();
  p.begin( this );
  p.drawPixmap( 0, 0, doubleBuffer );
  p.end();
}
