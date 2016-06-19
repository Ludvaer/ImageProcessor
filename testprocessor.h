#ifndef TESTPROCESSOR_H
#define TESTPROCESSOR_H

#include <QObject>
#include <QtTest>
#include "myimageprocessing.h"


class TestProcessor : public QObject
{
    Q_OBJECT
public:
    explicit TestProcessor(QObject *parent = 0);

signals:

public slots:

private slots:
    void medianTest();

};

#endif // TESTPROCESSOR_H
