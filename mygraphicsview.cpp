#include "mygraphicsview.h"
#include <QTimeLine>
#include <QScrollBar>

MyGraphicsView::MyGraphicsView(QWidget *parent) :
    QGraphicsView(parent),
    _numScheduledScalings(0)
{
    _dx = 0;
    _dy = 0;
    setScene(new QGraphicsScene());
    currentItem = NULL;
}

void MyGraphicsView::setImage(QImage& image)
{
    if(currentItem)
    {
        scene()->removeItem(currentItem);
        delete currentItem;
    }
    currentItem = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    scene()->addItem(currentItem);
    QRectF rect = scene()->itemsBoundingRect();
    scene()->setSceneRect(rect);
}


const int animationLength = 350;
const int animationSteps = 5;
void MyGraphicsView::wheelEvent ( QWheelEvent * event )
{
    _wheelPoint = event->pos();// - geometry().center();
   qreal dx = horizontalScrollBar()->value();
   qreal dy = verticalScrollBar()->value();
   if(abs(dx - _dx) > 1) _dx = dx;
   if(abs(dy - _dy) > 1) _dy = dy;

    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15; // see QWheelEvent documentation
    _numScheduledScalings += numSteps;
    if (_numScheduledScalings * numSteps < 0) // if user moved the wheel in another direction, we reset previously scheduled scalings
        _numScheduledScalings = numSteps;

    QTimeLine *anim = new QTimeLine(animationLength, this);
    anim->setUpdateInterval(animationLength/animationSteps);

    connect(anim, SIGNAL (valueChanged(qreal)), SLOT (scalingTime(qreal)));
    connect(anim, SIGNAL (finished()), SLOT (animFinished()));
    anim->start();
}

void MyGraphicsView::scalingTime(qreal x)
{
    qreal added = qreal(_numScheduledScalings) / 300.0;
    QTransform t = transform();
    qreal factor =  (1.0 + added) * t.m11();
    setZoom(factor);
    horizontalScrollBar()->setValue(_dx = ((added + 1)*_dx + (added)*_wheelPoint.x()));
    verticalScrollBar()->setValue(_dy = ((added + 1)*_dy + (added)*_wheelPoint.y()));
}

void MyGraphicsView::animFinished()
{
    if (_numScheduledScalings > 0)
        _numScheduledScalings--;
    else
        _numScheduledScalings++;
    sender()->~QObject();
    emit zoomChanged(transform().m11());
    //qDebug() << "animfin nschsk" <<_numScheduledScalings;
}

void MyGraphicsView::setZoom(qreal factor)
{
    if(factor != transform().m11())
    {
        setTransform(QTransform(factor,0,0,factor,0,0));
        emit zoomChanged(factor);
    }
}
