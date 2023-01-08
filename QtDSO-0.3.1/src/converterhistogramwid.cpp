#include <converterhistogramwid.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <stdlib.h>
#include <dso.h>
#include <math.h>

ConverterHistogramWid::ConverterHistogramWid( QWidget *parent, const char *name ) :
  QWidget( parent, name )
{
  setBackgroundMode( QWidget::NoBackground );
  
  clearSLOT();
}

ConverterHistogramWid::~ConverterHistogramWid()
{
}

void
ConverterHistogramWid::paintEvent( QPaintEvent * )
{
  QPixmap buf( size() );
  //buf.fill( colorGroup().base() );
  buf.fill( Qt::black );
  
  QPainter p;
  p.begin( &buf );
  p.setPen( Qt::green );
  
  for (int i=0; i<256; ++i)
  {
    p.drawLine( i, height()-1, i, height()-1-(int)(m_drawFac*m_histogram[i]) );
  }
  
  p.end();
  
  p.begin( this );
  p.drawPixmap( 0, 0, buf );
  p.end();
}

void
ConverterHistogramWid::addValues( Dso *dso, int channel )
{
  double max = 0.0;
  int shift = dso->bits()-8;
  
  if (shift >= 0)
  {
    for (int i=0; i<4096; ++i)
    {     
      int sample = dso->data( channel, i ) >> shift;

      m_histogram[sample]++;

      max = QMAX( max, m_histogram[sample] );
      m_max = QMAX( m_max, sample );
      m_min = QMIN( m_min, sample );
      m_numSamples++;
      m_sum += sample;
    }
  }
  else
  {
    for (int i=0; i<4096; ++i)
    {     
      int sample = dso->data( channel, i ) << (-shift);

      m_histogram[sample]++;

      max = QMAX( max, m_histogram[sample] );
      m_max = QMAX( m_max, sample );
      m_min = QMIN( m_min, sample );
      m_numSamples++;
      m_sum += sample;
    }
  }
  
  m_drawFac = (double)height() / max;
  
  update();
}

void
ConverterHistogramWid::clearSLOT()
{
  for (int i=0; i<256; ++i)
  {
    m_histogram[i] = 0.0;
  }
  m_drawFac = 0.0;
  m_numSamples = 0.0;
  m_max = 0;
  m_sum = 0.0;
  m_min = 10000000;
  
  m_timer.start();
}

double 
ConverterHistogramWid::mean() const
{
  if (m_numSamples != 0.0)
  {
    return m_sum / m_numSamples;
  }
  
  return 0.0;
}

double 
ConverterHistogramWid::stdDeviation() const
{
  double m = mean(); 
  double sum = 0.0;
  
  for (int i=0; i<256; ++i)
  {
    const double delta = m_histogram[i] - m;
    sum += delta*delta;
  }
  
  return sqrt( sum / 256.0 );
}
