#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
#include <QMessageBox>
#include <QMetaEnum>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  setGeometry(400, 300, 640, 480);

  setupSimpleDemo(ui->customPlot);

  setWindowTitle("Sound data");
  statusBar()->clearMessage();
  ui->customPlot->replot();

  // Uncomment to make screenshot and exit
  //QTimer::singleShot(4000, this, SLOT(screenShot()));
}

void MainWindow::setupSimpleDemo(QCustomPlot *customPlot)
{
  size_t result = 0;
  uint16_t value = 0;

  // add two new graph:
  customPlot->addGraph();
  customPlot->graph(0)->setPen(QPen(Qt::blue));
  customPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20)));

  QVector<double> x(1), y(1);
  int i = 0;
  while (true)
  {
    result = fread(&value, sizeof(uint16_t), 1, stdin);

    if (result != 1)
        break;

    x.push_back(i);
    y.push_back((double)value / 1000);

    i++;
  }

  customPlot->xAxis2->setVisible(true);
  customPlot->xAxis2->setTickLabels(false);
  customPlot->yAxis2->setVisible(true);
  customPlot->yAxis2->setTickLabels(false);

  connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
  connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

  customPlot->graph(0)->setData(x, y);

  customPlot->graph(0)->rescaleAxes();
  customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}

void MainWindow::setupPlayground(QCustomPlot *customPlot)
{
  Q_UNUSED(customPlot)
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::screenShot()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  QPixmap pm = QPixmap::grabWindow(qApp->desktop()->winId(), this->x()+2, this->y()+2, this->frameGeometry().width()-4, this->frameGeometry().height()-4);
#else
  QPixmap pm = qApp->primaryScreen()->grabWindow(qApp->desktop()->winId(), this->x()+2, this->y()+2, this->frameGeometry().width()-4, this->frameGeometry().height()-4);
#endif
  QString fileName = "SoundData.png";
  fileName.replace(" ", "");
  pm.save("./screenshots/"+fileName);
  qApp->quit();
}
