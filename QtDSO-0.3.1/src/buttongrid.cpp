//======================================================================
// File:		buttongrid.cpp
// Author:	Matthias Toussaint
// Created:	Sun Sep 22 22:07:32 CEST 2002
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

#include <qgrid.h>
#include <qtoolbutton.h>
#include <qlayout.h>
#include <qpalette.h>

#include <buttongrid.h>

ButtonGrid::ButtonGrid( QWidget *parent, const char *name ) :
  QButtonGroup( parent, name ),
  m_maxId( 0 )
{
  QHBoxLayout *hl = new QHBoxLayout( this );
  hl->setAutoAdd( true );
  
  m_grid = new QGrid( 3, this );
  
  //hl->addWidget( m_grid );
  
  setMargin( 0 );
  setFrameStyle( QFrame::NoFrame );
  
  setExclusive( true );
  
  m_darkCg = colorGroup();
  m_cg = colorGroup();
  m_darkCg.setColor( QColorGroup::Button, m_darkCg.button().dark( 115 ) );
  
  connect( this, SIGNAL( clicked(int) ),
           this, SLOT( clickedSLOT(int) ));
}

ButtonGrid::~ButtonGrid()
{
}

void
ButtonGrid::addButton( const QString &label, int id, QColor *col )
{
  QToolButton *button = new QToolButton( m_grid );
  button->setText( label );
  button->setToggleButton( true );
  insert( button, id );
  
  if (col)
  {
    QColorGroup cg = colorGroup();
    cg.setColor( QColorGroup::Button, *col );
    cg.setColor( QColorGroup::ButtonText, cg.dark() );
    
    button->setPalette( QPalette( cg, cg, cg ) );
    
    m_ownColor[id] = true;
  }
  else
  {
    m_ownColor[id] = false;
  }
  
  m_maxId = QMAX( id, m_maxId );
  
  button->show();
}

void
ButtonGrid::clear()
{
  for (int i=0; i<=m_maxId; ++i)
  {
    QButton *button = find(i);
    
    if (button)
    {
      remove( button );
      delete button;
    }
  }
  
  m_maxId = 0;
  
  // unfortunately the grid can't be reset
  // to ensure that new buttons are inserted at top left position
  // we have to recreate the grid
  //
  delete m_grid;
  
  m_grid = new QGrid( 3, this );
  m_grid->show();
}

void
ButtonGrid::setButton( int id )
{
  QButtonGroup::setButton( id );
  clickedSLOT( id );
}

void
ButtonGrid::clickedSLOT( int id )
{
  QColorGroup cg = colorGroup();
  int i=0;
  
  for (int i=0; i<=m_maxId; ++i)
  {
    QButton *button = find( i );
    
    if (button && !m_ownColor[i])
    {
      button->setPalette( QPalette( m_cg, m_cg, m_cg ) );
    }
  }
  
  if (!m_ownColor[id])
  {
    QButton *button=find( id );
    button->setPalette( QPalette( m_darkCg, m_darkCg, m_darkCg ) );
  }
}
