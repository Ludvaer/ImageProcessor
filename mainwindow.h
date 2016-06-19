#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QImage>
#include <QPixmap>
#include <QProgressDialog>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void openFile();
    void resetImage();
    void processImage();

    void saveCombined();
    void saveCombinedCut();
    void saveProcessed();
    void saveProcessedCut();

private:
    void deleteItem();
    bool setCurrentImage();
    QString cutImageNameModify(const QString& filename);

    template<typename ImageType>
    void saveImage(const QString& filename,const ImageType& image);


    QImage combineImage(const QImage& image1,const QImage& image2);
    QImage combineImage(const QPixmap& image1,const QPixmap& image2);

    Ui::MainWindow *ui;
    QString fileName;
    QImage* currentImage;
    QProgressDialog progressDialog;
};

#endif // MAINWINDOW_H
