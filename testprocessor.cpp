#include "testprocessor.h"


TestProcessor::TestProcessor(QObject *parent) :
    QObject(parent)
{
}

void TestProcessor::medianTest()
{
    uchar singleNumberArray[12] = {127};
    QCOMPARE(MyImageProcessing::median(singleNumberArray,singleNumberArray,singleNumberArray), (uchar)127);

   // uchar testArray1[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9 , 10, 11, 12};
}
