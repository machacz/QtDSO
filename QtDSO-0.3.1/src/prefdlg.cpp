//======================================================================
// File:		prefdlg.cpp
// Author:	Matthias Toussaint
// Created:	Sun Sep  1 12:06:10 CEST 2002
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

#include <qspinbox.h>
#include <qcombobox.h>
#include <qfontdialog.h>
#include <qdir.h>

#include <colorbutton.h>
#include <dsowid.h>
#include <prefdlg.h>
#include <simplecfg.h>

QColor PrefDlg::s_bgColorDef = Qt::black;
QColor PrefDlg::s_borderColorDef = Qt::white;
QColor PrefDlg::s_gridColorDef = Qt::darkGray;
QColor PrefDlg::s_chColorDef[2] = { QColor( 50, 255, 50 ), QColor( 255, 255, 50 ) };
QColor PrefDlg::s_addColorDef = QColor( 180, 180, 255 );
QColor PrefDlg::s_subColorDef = QColor( 255, 180, 180 );
QColor PrefDlg::s_timeColorDef = Qt::cyan;
int    PrefDlg::s_lineWidth = 1;
int    PrefDlg::s_lineWidthFft = 1;

PrefDlg::PrefDlg( QWidget *parent, const char *name ) :
  UIPrefDlg( parent, name )
{
  QString cfgName = QDir::homeDirPath() + "/.qtdsorc";
  m_cfg = new SimpleCfg( cfgName );
  
  connect( ui_apply, SIGNAL( clicked() ),
           this, SLOT( applySLOT() ));
  connect( ui_ok, SIGNAL( clicked() ),
           this, SLOT( okSLOT() ));
  connect( ui_close, SIGNAL( clicked() ),
           this, SLOT( reject() ));
  
  connect( ui_defaultColors, SIGNAL( clicked() ),
           this, SLOT( loadDefaultColorsSLOT() ));
  
  connect( ui_fontBut, SIGNAL( clicked() ),
           this, SLOT( displayFontSLOT() ));
  
  loadDefaults();
  setDefaults();
  
  loadConfig();
}

PrefDlg::~PrefDlg()
{
  saveConfig();
  
  delete m_cfg;
}

int
PrefDlg::device() const
{
  return ui_device->currentItem();
}
/*
void
PrefDlg::show()
{
  loadDefaults();
  setDefaults();
  QDialog::show();
}
*/
void 
PrefDlg::applySLOT()
{
  DsoWid::chColor[0]     = ui_ch1Color->color();
  DsoWid::chColor[1]     = ui_ch2Color->color();
  DsoWid::addColor       = ui_addColor->color();
  DsoWid::subColor       = ui_subColor->color();
  DsoWid::timeColor      = ui_timeMarkerColor->color();
  DsoWid::bgColor        = ui_bgColor->color();
  DsoWid::gridColor      = ui_gridColor->color();
  DsoWid::borderColor    = ui_borderColor->color();
  DsoWid::s_lineWidth    = lineWidth();
  DsoWid::s_lineWidthFft = lineWidthFft();
  
  emit applyPrefs();
}

void
PrefDlg::okSLOT()
{
  applySLOT();
  accept();
}

void
PrefDlg::loadDefaultColorsSLOT()
{
  ui_ch1Color->setColor( s_chColorDef[0] );
  ui_ch2Color->setColor( s_chColorDef[1] );
  ui_addColor->setColor( s_addColorDef );
  ui_subColor->setColor( s_subColorDef );
  ui_timeMarkerColor->setColor( s_timeColorDef );
  ui_bgColor->setColor( s_bgColorDef );
  ui_gridColor->setColor( s_gridColorDef );
  ui_borderColor->setColor( s_borderColorDef );    
  setLineWidth( s_lineWidth );
}

void
PrefDlg::loadDefaults()
{
  m_bgColor      = DsoWid::bgColor;
  m_borderColor  = DsoWid::borderColor;
  m_gridColor    = DsoWid::gridColor;
  m_chColor[0]   = DsoWid::chColor[0];
  m_chColor[1]   = DsoWid::chColor[1];
  m_addColor     = DsoWid::addColor;
  m_subColor     = DsoWid::subColor;
  m_timeColor    = DsoWid::timeColor;
  m_lineWidth    = DsoWid::s_lineWidth;
  m_lineWidthFft = DsoWid::s_lineWidthFft;
}

void
PrefDlg::setDefaults()
{
  ui_ch1Color->setColor( m_chColor[0] );
  ui_ch2Color->setColor( m_chColor[1] );
  ui_addColor->setColor( m_addColor );
  ui_subColor->setColor( m_subColor );
  ui_timeMarkerColor->setColor( m_timeColor );
  ui_bgColor->setColor( m_bgColor );
  ui_gridColor->setColor( m_gridColor );
  ui_borderColor->setColor( m_borderColor );
  setLineWidth( m_lineWidth );
  setLineWidthFft( m_lineWidthFft );
}

void
PrefDlg::setLineWidth( int width )
{
  ui_lineWidth->setValue( width );
}

int 
PrefDlg::lineWidth() const
{
  return ui_lineWidth->value();
}

void
PrefDlg::setLineWidthFft( int width )
{
  if (0 == width)
  {
    ui_lineWidthFft->setValue( 1 );
  }
  else
  {
    ui_lineWidthFft->setValue( width );
  }
}

int 
PrefDlg::lineWidthFft() const
{
  return ( ui_lineWidthFft->value() == 1 ? 0 : ui_lineWidthFft->value() );
}

int
PrefDlg::fftBufferLength()
{
  return ui_averageBufferLength->currentText().toInt();
}

void
PrefDlg::displayFontSLOT()
{
  bool ok = false;
  
  QFont newFont = QFontDialog::getFont( &ok, ui_fontBut->font(), this );
  
  if (ok)
  {
    ui_fontBut->setFont( newFont );
  }
}

QFont
PrefDlg::displayFont() const
{
  return ui_fontBut->font();
}

DsoWid::FFTDrawMode
PrefDlg::fftDrawMode( int channel ) const
{
  if (0 == channel)
  {
    return (DsoWid::FFTDrawMode)ui_ch1Drawmode->currentItem();
  }
  
  return (DsoWid::FFTDrawMode)ui_ch2Drawmode->currentItem();
}

void
PrefDlg::loadConfig()
{
  m_cfg->load();
  
  m_bgColor     = QColor( m_cfg->getRGB( "display", "background", 
                          s_bgColorDef.rgb() ));
  m_gridColor   = QColor( m_cfg->getRGB( "display", "grid", 
                          s_gridColorDef.rgb() ));
  m_borderColor = QColor( m_cfg->getRGB( "display", "border", 
                          s_borderColorDef.rgb() ));
  m_chColor[0]  = QColor( m_cfg->getRGB( "display", "channel-1", 
                          s_chColorDef[0].rgb() ));
  m_chColor[1]  = QColor( m_cfg->getRGB( "display", "channel-2", 
                          s_chColorDef[1].rgb() ));
  m_addColor    = QColor( m_cfg->getRGB( "display", "add", 
                          s_addColorDef.rgb() ));
  m_subColor    = QColor( m_cfg->getRGB( "display", "sub", 
                          s_subColorDef.rgb() ));
  m_timeColor   = QColor( m_cfg->getRGB( "display", "time-marker", 
                          s_timeColorDef.rgb() ));
  
  m_lineWidth = m_cfg->getInt( "display", "line-width", s_lineWidth );
  m_lineWidthFft = m_cfg->getInt( "fft", "line-width", s_lineWidthFft );
}

void
PrefDlg::saveConfig()
{
  m_cfg->setRGB( "display", "background", m_bgColor.rgb() );
  m_cfg->setRGB( "display", "grid", m_gridColor.rgb() );
  m_cfg->setRGB( "display", "border", m_borderColor.rgb() );
  m_cfg->setRGB( "display", "channel-1", m_chColor[0].rgb() );
  m_cfg->setRGB( "display", "channel-2", m_chColor[1].rgb() );
  m_cfg->setRGB( "display", "add", m_addColor.rgb() );
  m_cfg->setRGB( "display", "sub", m_subColor.rgb() );
  m_cfg->setRGB( "display", "time-marker", m_timeColor.rgb() );
  
  m_cfg->setInt( "display", "line-width", s_lineWidth );
  m_cfg->setInt( "fft", "line-width", s_lineWidthFft );
  
  m_cfg->save();
}

