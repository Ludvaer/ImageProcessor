#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QScrollBar>
#include <QFileDialog>
#include <QDebug>
#include <QFileInfo>
#include <QTime>
#include <QVariant>
#include <QVector>
#include "myimageprocessing.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    fileName(""),
    currentImage(NULL)
{
    //progressDialog.set


    ui->setupUi(this);
    //connect buttons
    connect(ui->openFileButton, SIGNAL (released()), this, SLOT (openFile()));
    connect(ui->reopenButton, SIGNAL (released()), this, SLOT (resetImage()));
    connect(ui->applyButton, SIGNAL (released()), this, SLOT (processImage()));
    connect(ui->saveCombinedButton, SIGNAL (released()), this, SLOT (saveCombined()));
    connect(ui->saveCombinedCutButton, SIGNAL (released()), this, SLOT (saveCombinedCut()));
    connect(ui->saveProcessedButton, SIGNAL (released()), this, SLOT (saveProcessed()));
    connect(ui->saveProcessedCutButton, SIGNAL (released()), this, SLOT (saveProcessedCut()));

    //sync graphic views
    QScrollBar* horizontalScrollBar1 = ui->processedImageView->horizontalScrollBar();
    QScrollBar* horizontalScrollBar2 = ui->originalImageView->horizontalScrollBar();
    QScrollBar* verticalScrollBar1 = ui->processedImageView->verticalScrollBar();
    QScrollBar* verticalScrollBar2 = ui->originalImageView->verticalScrollBar();
    connect(horizontalScrollBar1, SIGNAL(valueChanged(int)), horizontalScrollBar2, SLOT(setValue(int)));
    connect(horizontalScrollBar2, SIGNAL(valueChanged(int)), horizontalScrollBar1, SLOT(setValue(int)));
    connect(verticalScrollBar1, SIGNAL(valueChanged(int)), verticalScrollBar2, SLOT(setValue(int)));
    connect(verticalScrollBar2, SIGNAL(valueChanged(int)), verticalScrollBar1, SLOT(setValue(int)));
    connect(ui->originalImageView, SIGNAL(zoomChanged(qreal)),ui->processedImageView, SLOT (setZoom(qreal)) );
    connect(ui->processedImageView, SIGNAL(zoomChanged(qreal)),ui->originalImageView, SLOT (setZoom(qreal)) );
    connect(ui->iterationSlider, SIGNAL(valueChanged(int)), ui->iterationLabel, SLOT(setNum(int)));


    QVector<MyImageProcessing::ImageProcessor*>& vector(MyImageProcessing::filterList);
    for(int i = 0; i < vector.size(); i++)
       ui->filtersBox->insertItem(i,vector.at(i)->name, QVariant(i));//, qVariantFromValue(filter));
 }

MainWindow::~MainWindow()
{
    if(currentImage) delete currentImage;
    delete ui;
}


void MainWindow::openFile()
{
    QString prevFileName = fileName;
    fileName = QFileDialog::getOpenFileName(this, "Open Image", fileName,tr("Image Files (*.png *.jpg *.jpeg *.bmp *.pbm *.pgm *.ppm *.xbm *.xpm);; All Files (*)"));
    if (!setCurrentImage()) fileName = prevFileName;
}

void MainWindow::resetImage()
{
    setCurrentImage();
}

QString MainWindow::cutImageNameModify(const QString& filename)
{
    QRectF rect = ui->originalImageView->mapToScene(ui->originalImageView->viewport()->geometry()).boundingRect();
    return QString("%1_%2_%3_%4_%5.png").arg(filename, QString::number(rect.left()), QString::number(rect.top()), QString::number(rect.width()), QString::number(rect.height()));
}

template<typename ImageType>
void MainWindow::saveImage(const QString& name,const ImageType& image)
{
    if(image.save(name))
        statusBar()->showMessage(QString("saved %1").arg(name));
    else
        statusBar()->showMessage(QString("not saved"));
}

#define COMBINE(drawMethod)\
{\
    QImage resultImage(image1.width() + image2.width(), image1.height(), currentImage->format());\
    QPainter painter(&resultImage);\
    painter.drawMethod(0,0, image1);\
    painter.drawMethod(image1.width(),0, image2);\
    painter.end();\
    return resultImage;\
}

QImage MainWindow::combineImage(const QPixmap& image1,const QPixmap& image2)
COMBINE(drawPixmap)

QImage MainWindow::combineImage(const QImage& image1,const QImage& image2)
COMBINE(drawImage)

#define IMAGE_CHECK_NULL \
if(!currentImage)\
{\
     statusBar()->showMessage(QString("no image to save"));\
    return;\
}

void MainWindow::saveCombined()
{
    IMAGE_CHECK_NULL
    QImage originalImage;
    originalImage.load(fileName);
    saveImage(fileName+"combined.png", combineImage(originalImage, *currentImage));
}


void MainWindow::saveCombinedCut()
{
    IMAGE_CHECK_NULL
    //QPixmap::grabWidget(ui->originalImageView->viewport());
    QPixmap left(QPixmap::grabWidget(ui->originalImageView->viewport()));
    QPixmap right(QPixmap::grabWidget(ui->processedImageView->viewport()));
    saveImage(cutImageNameModify(fileName+"combined_"), combineImage(left,right));
}

void MainWindow::saveProcessed()
{
    IMAGE_CHECK_NULL
    saveImage(fileName+"processed.png",*currentImage);
}

void MainWindow::saveProcessedCut()
{
    IMAGE_CHECK_NULL
    saveImage((cutImageNameModify(fileName)), QPixmap::grabWidget(ui->processedImageView->viewport()));
}

bool MainWindow::setCurrentImage()
{
    QFileInfo check_file(fileName);
    if(!(check_file.exists() && check_file.isFile())) return false;
    if(currentImage) delete currentImage;
    currentImage = new QImage();
    currentImage->load(fileName);
    ui->processedImageView->setImage(*currentImage);
    ui->originalImageView->setImage(*currentImage);
    return true;
}

void MainWindow::processImage()
{
    if(!currentImage)
    {
        statusBar()->showMessage(("no image to save"));
        return;
    }

    MyImageProcessing::ImageProcessor& filter = *(MyImageProcessing::filterList.at(ui->filtersBox->currentIndex()));
    int max = ui->iterationSlider->value();
    QProgressDialog progressDialog("Processing...", "Stop it now", 0, 1, this);
    progressDialog.setMinimumDuration(500);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setWindowFlags( progressDialog.windowFlags() | Qt::FramelessWindowHint);
    progressDialog.setModal(true);
    progressDialog.setAutoFillBackground(false);

    progressDialog.setMaximum(max);
    QTime myTimer;
    myTimer.start();

    for(int i = 0; i < max && !progressDialog.wasCanceled(); i++)
    {
        progressDialog.setValue(i);
        progressDialog.setGeometry(geometry().x(), geometry().y()+(height()-progressDialog.height())/2, width(), progressDialog.height());
        QImage *temp = currentImage;
        currentImage = filter.apply(*temp);
        delete temp;
    }
    statusBar()->showMessage(QString("%2 completed in %1ms").arg(QString::number(myTimer.elapsed()),filter.name),15000);
    ui->processedImageView->setImage(*currentImage);
    progressDialog.setValue(max);
    progressDialog.reset();
}
