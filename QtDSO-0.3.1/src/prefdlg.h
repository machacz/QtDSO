//======================================================================
// File:		prefdlg.h
// Author:	Matthias Toussaint
// Created:	Sun Sep  1 12:04:46 CEST 2002
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

#ifndef PREFDLG_HH
#define PREFDLG_HH

#include <uiprefdlg.h>
#include <dsowid.h>

class SimpleCfg;

class PrefDlg : public UIPrefDlg
{
  Q_OBJECT
public:
  PrefDlg( QWidget *parent=0, const char *name=0 );
  virtual ~PrefDlg();
  
  //void show();
  int fftBufferLength();
  
  QFont displayFont() const;
  DsoWid::FFTDrawMode fftDrawMode( int ) const;
  
  int device() const;
  
signals:
  void applyPrefs();

protected:
  QColor     m_bgColor;
  QColor     m_borderColor;
  QColor     m_gridColor;
  QColor     m_chColor[2];
  QColor     m_addColor;
  QColor     m_subColor;
  QColor     m_timeColor;
  int        m_lineWidth;
  int        m_lineWidthFft;
  SimpleCfg *m_cfg;
  
  static QColor s_bgColorDef;
  static QColor s_gridColorDef;
  static QColor s_borderColorDef;
  static QColor s_chColorDef[2];
  static QColor s_addColorDef;
  static QColor s_subColorDef;
  static QColor s_timeColorDef;
  static int    s_lineWidth;
  static int    s_lineWidthFft;
  
  void loadDefaults();
  void setDefaults();
  void setLineWidth( int );
  int  lineWidth() const;
  void setLineWidthFft( int );
  int  lineWidthFft() const;
  void loadConfig();
  void saveConfig();
  
protected slots:
  void loadDefaultColorsSLOT();
  void applySLOT();
  void okSLOT();
  void displayFontSLOT();
  
};

#endif // PREFDLG_HH
