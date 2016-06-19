#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QImage>
#include <QGraphicsPixmapItem>
#include <QWheelEvent>


class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MyGraphicsView(QWidget *parent = 0);
    void setImage(QImage& image);
    void wheelEvent ( QWheelEvent * event );//override

signals:
    void zoomChanged(qreal factor);
public slots:
    void setZoom(qreal factor);
    void scalingTime(qreal x);
    void animFinished();

private:
    QGraphicsPixmapItem *currentItem;
    int _numScheduledScalings;

    QPointF _wheelPoint;
    qreal _dx;
    qreal _dy;

};

#endif // MYGRAPHICSVIEW_H
