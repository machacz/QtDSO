#ifndef CONVERTERHISTOGRAMWID_HH
#define CONVERTERHISTOGRAMWID_HH

#include <qwidget.h>
#include <qdatetime.h>

class Dso;

class ConverterHistogramWid : public QWidget
{
  Q_OBJECT
public:
  ConverterHistogramWid( QWidget *parent=0, const char *name=0 );
  virtual ~ConverterHistogramWid();
  
  double numSamples() const { return m_numSamples; }
  int max() const { return m_max; }
  int min() const { return m_min; }
  double sum() const { return m_sum; }
  double mean() const;
  double stdDeviation() const;
  int msec() const { return m_timer.elapsed(); }

  void addValues( Dso *, int );
    
public slots:
  void clearSLOT();

protected:
  double m_drawFac;
  double m_histogram[256];
  double m_numSamples;
  int    m_max;
  int    m_min;
  double m_sum;
  QTime  m_timer;
  
  void paintEvent( QPaintEvent * );
  
};

#endif // CONVERTERHISTOGRAMWID_HH
