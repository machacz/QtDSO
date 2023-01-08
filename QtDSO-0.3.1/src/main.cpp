//======================================================================
// File:		main.cpp
// Author:	Matthias Toussaint
// Created:	Sun Jun  9 22:32:44 CEST 2002
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

#include <mainwindow.h>
#include <qapplication.h>

int
main( int argc, char **argv )
{
  QApplication app( argc, argv );
  
  MainWindow win;
  
  app.setMainWidget( &win );
  win.show();
  
  app.exec();
}

