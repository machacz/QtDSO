//======================================================================
// File:		buttongrid.h
// Author:	Matthias Toussaint
// Created:	Sun Sep 22 22:03:14 CEST 2002
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

#ifndef BUTTONGRID_HH
#define BUTTONGRID_HH

#include <qbuttongroup.h>

class QGrid;

class ButtonGrid : public QButtonGroup
{
  Q_OBJECT
public:
  ButtonGrid( QWidget *parent=0, const char *name=0 );
  virtual ~ButtonGrid();
  
  void addButton( const QString &, int id, QColor *col = 0 );
  void clear();
  void setButton( int id );
  
protected slots:
  void clickedSLOT( int );

protected:
  QGrid       *m_grid;
  int          m_maxId;
  QColorGroup  m_darkCg;
  QColorGroup  m_cg;
  bool         m_ownColor[64];
  
};
      
#endif // BUTTONGRID_HH
