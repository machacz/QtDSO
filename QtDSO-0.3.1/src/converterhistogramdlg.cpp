#include <converterhistogramdlg.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <converterhistogramwid.h>
#include <qstring.h>
#include <qcombobox.h>

ConverterHistogramDlg::ConverterHistogramDlg( QWidget *parent, const char *name ) :
  UIConverterHistogramDlg( parent, name )
{
  connect( ui_clear, SIGNAL( clicked() ),
           ui_histogram, SLOT( clearSLOT() ));
  
  adjustSize();
  
  startTimer( 300 );
}

ConverterHistogramDlg::~ConverterHistogramDlg()
{
}

void
ConverterHistogramDlg::addValues( Dso *dso )
{
  ui_histogram->addValues( dso, ui_channel->currentItem() );
}

void
ConverterHistogramDlg::timerEvent( QTimerEvent * )
{
  QString txt;
  
  if (ui_histogram->numSamples() != 0.0)
  {
    txt.sprintf( "%lu", (unsigned long)ui_histogram->numSamples() );
    ui_numSamples->setText( txt );

    txt.sprintf( "%d", ui_histogram->min() );
    ui_minSample->setText( txt );

    txt.sprintf( "%d", ui_histogram->max() );
    ui_maxSample->setText( txt );

    txt.sprintf( "%g", ui_histogram->mean() );/// ui_histogram->numSamples() * 256.0 );
    ui_meanSample->setText( txt );

    txt.sprintf( "%.4f", ui_histogram->stdDeviation() / ui_histogram->numSamples() * 100.0 );
    ui_stdDeviationSample->setText( txt );
  }
  else
  {
    ui_numSamples->setText( "-" );
    ui_minSample->setText( "-" );
    ui_maxSample->setText( "-" );
    ui_meanSample->setText( "-" );
    ui_stdDeviationSample->setText( "-" );
  }
  
  unsigned sec = ui_histogram->msec() / 1000;
  
  if (sec < 60)
  {
    txt.sprintf( "%ds", sec );
  }
  else
  {
    unsigned min = sec / 60;
    sec = sec % 60;
    
    if (min < 60)
    {
      txt.sprintf( "%dmin %02ds", min, sec );
    }
    else
    {
      unsigned hour = min / 60;
      min = min % 60;
      
      txt.sprintf( "%dh %02dmin %02ds", hour, min, sec );
    }
  }
    
  ui_time->setText( txt );
}

