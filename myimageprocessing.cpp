#include "myimageprocessing.h"
#include "qmath.h"
#include <QDebug>
#define CENTER_WEIGHT 1.414213562

QVector<MyImageProcessing::ImageProcessor*> MyImageProcessing::filterList = QVector<MyImageProcessing::ImageProcessor*>();

MyImageProcessing::ImageProcessor::ImageProcessor(const QString& name):
    name(name)
{
}





#define INIT_GRADIENT_VECTOR(top,left) \
    int top##left##left = center + top - top##left - left;\
    int top##left##top = center + left - top##left - top;\
    long top##left##Abs = (top##left##left * top##left##left + top##left##top * top##left##top );

//this macros calculates weight of ajacent pixel
#define GET_WEIGHT(right,top,left)   \
{\
    if(top##left##Abs > 0 && right##top##Abs > 0 && (top##left##top+right##top##top)*(top##left##left + right##top##right) > 0) \
    {   \
        int dotProduct = (top##left##top*right##top##top - top##left##left*right##top##right);\
        if(SHARPER)\
            dotProduct = abs(dotProduct);\
        qreal topWeight = 1 - (dotProduct / (qreal)sqrt((qreal)top##left##Abs*right##top##Abs));  \
        sum += top*topWeight;  \
        weight += topWeight;  \
    }\
}
template <bool SHARPER>
IMP_SQR3_FUNCTOR(StraightenBourders<SHARPER>)
{

    INIT_GRADIENT_VECTOR(top,left)
    INIT_GRADIENT_VECTOR(left,bot)
    INIT_GRADIENT_VECTOR(bot,right)
    INIT_GRADIENT_VECTOR(right,top)

    qreal weight = 1;
    qreal sum = center*weight;

    GET_WEIGHT(right, top, left)
    GET_WEIGHT(top, left, bot)
    GET_WEIGHT(left, bot, right)
    GET_WEIGHT(bot, right, top)


    qreal result = sum/weight + 0.5;//499999;
#ifdef QT_DEBUG
    if((uchar)result > 255 || (uchar)result < 0)
        qDebug() << QString("got bad pixel value %1 with sum = %2 and weight = %3").arg(QString::number(result),QString::number(sum),QString::number(weight)); //not really possible to happen
#endif
    return (uchar)(result);
}

#define CHOOSE_CLOSEST_VALUE(a1,b1) \
{ \
    uchar a = a1; uchar b = b1; \
    uchar r1 = center; \
    ++weight; \
    if((center >= a) && (center >= b)) \
        xmin = qMax(qMax(a,b),xmin); \
    else\
    {\
        if((center <= a) && (center <= b)) \
            xmax = qMin(qMin(a,b),xmax); \
        else\
        {\
           xmin = qMin(qMax(a,b),xmin); \
           xmax = qMin(qMax(a,b),xmax); \
           if(abs(center - a) < abs(center - b)) \
               r1 = a; \
           else \
               if(abs(center - b) < abs(center - a)) \
                     r1 = b; \
        }\
    } \
    sum += r1; \
}


template <bool WAY8>
IMP_SQR3_FUNCTOR(NarrowBourders<WAY8>)
{
    uchar xmax = 255;
    uchar xmin = 0;

    int sum = 0;
    int weight = 0;

    CHOOSE_CLOSEST_VALUE(left,right)
    CHOOSE_CLOSEST_VALUE(top,bot)
    if(WAY8)
    {
        CHOOSE_CLOSEST_VALUE(topleft,botright)
        CHOOSE_CLOSEST_VALUE(leftbot,righttop)
    }

    int result = (sum + weight/2)/weight;

    if(result < xmin) result = xmin;
    if(result > xmax) result = xmax;
    return (uchar)result;
}

#define FOR_EACH_8_AJACENT(PIXEL_WORK) \
PIXEL_WORK(topleft) \
PIXEL_WORK(left) \
PIXEL_WORK(leftbot) \
PIXEL_WORK(top) \
PIXEL_WORK(bot) \
PIXEL_WORK(righttop) \
PIXEL_WORK(right) \
PIXEL_WORK(botright)


template <typename F>
MyImageProcessing::Conway<F>::Conway(const F& f): f(f){}

#define COPY_PIXEL(pix_value)\
    uchar c##pix_value(pix_value);

#define CALC_PIXEL(pix_value)\
    count += c##pix_value % 2;\
    c##pix_value /= 2;

template <typename F>
IMP_SQR3_FUNCTOR(Conway<F>)
{
    uchar result = 0;
    uchar pow = 1;
    FOR_EACH_8_AJACENT(COPY_PIXEL)
    uchar c_center(center);
    for(int i = 0; i < 8; i++)
    {
        uchar count = 0;
        FOR_EACH_8_AJACENT(CALC_PIXEL)
        if (f(count,c_center % 2))  result += pow;
        c_center /= 2;
        pow *= 2;
    }
    return result;
}


 //   qDebug()<< pix_value << #pix_value ;
#define PARTITION_STEP(pix_value) \
if (pix_value < center) \
{ \
    *i = pix_value; \
    ++i; \
} \
else if (pix_value > center) \
{ \
    *j = pix_value; \
    --j; \
}

uchar MyImageProcessing::median(uchar* const left, uchar * const right, uchar* const mid)
{
    uchar divider = *left;
    if (left == right) return divider;
    uchar* c = left + 1;
    if (mid == left) //experiment showed that this way is faster
    {
        for(;c<=right;c++) if(divider > *c) divider = *c;
        return divider;
    }
    if (mid == right)
    {
        for(;c<=right;c++) if(divider < *c) divider = *c;
        return divider;
    }
    uchar* eq = left;
    uchar* r = right;
    while (c <= r)
    {
        uchar current = *c;
        if(current < divider)
        {
            *eq = current;
            ++eq;
            //current = divider; // not needed, not doing sort, not going use
            ++c;
        }
        else if (current > divider)
        {
            *c = *r;
            *r = current;
            --r;
        }
        else
        {
            //current == divider - go next element
            ++c;
        }
    }
    if(r < mid) return median(r+1 ,right,mid );
    if(eq > mid) return median(left,eq - 1,mid);
    return divider;
}

#define CHECK_ANS_PIXEL(pixel_value)   if(pixel_value < ans) ++counta; if(pixel_value > ans) ++ countb;



IMP_SQR3_FUNCTOR(MedianFilter)
{
    uchar arr[8];
    uchar* i = arr;
    uchar* j = arr + 7;
    //qDebug() << "center " << center;
    FOR_EACH_8_AJACENT(PARTITION_STEP)
    uchar ans;

    if(i > (arr + 4))
    {
        //return center - 1;
        ans = median(arr, i - 1, arr+  4);
        //qDebug() << "med " << ans;

    }
    else  if(j < (arr + 3))
    {
        //return center + 1;
        ans = median(j + 1, arr + 7, arr + 3 );
        // qDebug() << "med " << ans;
    }
    else
    {
       //     qDebug() << "med " << center;
        ans = center;
     }

#ifdef QT_DEBUG
    int counta = 0;
    int countb = 0;
    if(center < ans) ++counta;
    if(center > ans) ++countb;
    FOR_EACH_8_AJACENT(CHECK_ANS_PIXEL)
    if(counta > 4)
    {
        qDebug() << "median error";
    }
    if(countb > 4)
    {
        qDebug() << "median error";
    }
#endif


    return ans;
}


template <typename F>
QImage* MyImageProcessing::filterSqr3(const F& f, QImage& input)
{
    int width = input.width();
    int height = input.height();
    int depth = input.depth()/8;

    QImage* output = new QImage(width,height,input.format());

    int bytesPerLine = input.bytesPerLine();
    int bytesPerLineOut = output->bytesPerLine();

    uchar* currentLinePtr0 = input.scanLine(0);
    uchar* nextLinePtr0 = input.scanLine(0);
    uchar* endLines = output->scanLine(0) + (height-1)*bytesPerLineOut;
    uchar* outputLine0;
    for(  outputLine0 = output->scanLine(0); outputLine0 < endLines; outputLine0 += bytesPerLineOut)
    {

        uchar* prevLinePtr = currentLinePtr0;
        uchar* currentPtr = (currentLinePtr0 = nextLinePtr0);
        uchar* nextLinePtr = (nextLinePtr0 +=  bytesPerLine);

//-----start defines----
//define pixel columns
#define LEFT_COL_PTRS prevLinePtr[-depth], currentPtr[-depth], nextLinePtr[-depth]
#define CENTR_COL_PTRS *prevLinePtr, *currentPtr, *nextLinePtr
#define RIGHT_COL_PTRS prevLinePtr[depth], currentPtr[depth], nextLinePtr[depth]
//define increment
#define INCREMENT_PTRS ++prevLinePtr;++currentPtr;++nextLinePtr;
//define line process
#define SCAN_LINE                        \
        uchar* outputLine = outputLine0; \
        uchar* lineEnd = outputLine + depth;\
        for(; outputLine < lineEnd; outputLine++) \
        { \
            *outputLine = f(CENTR_COL_PTRS, CENTR_COL_PTRS, RIGHT_COL_PTRS); \
            INCREMENT_PTRS \
        } \
        lineEnd = outputLine +(width-2)*depth; \
        for(; outputLine < lineEnd; outputLine++) \
        { \
            *outputLine = f(LEFT_COL_PTRS, CENTR_COL_PTRS, RIGHT_COL_PTRS); \
            INCREMENT_PTRS \
        } \
        lineEnd = outputLine + depth; \
        for(; outputLine < lineEnd; outputLine++) \
        { \
            *outputLine = f(LEFT_COL_PTRS, CENTR_COL_PTRS, CENTR_COL_PTRS); \
            INCREMENT_PTRS \
        }
//-----end defines----

        SCAN_LINE
    }
    {
        uchar* prevLinePtr = currentLinePtr0;
        uchar* currentPtr = nextLinePtr0;
        uchar* nextLinePtr = nextLinePtr0;
        SCAN_LINE
    }

    return output;
}






IMPLEMENT_FILTER (straightenBourdersSharper)
{

    return filterSqr3(StraightenBourders<true>(), input);
}
IMPLEMENT_FILTER(straightenBourders)
{
    return filterSqr3(StraightenBourders<true>(), input);
}
IMPLEMENT_FILTER(narrowBourders)
{

    return filterSqr3(NarrowBourders<false>(), input);
}
IMPLEMENT_FILTER(narrowBourdersWay8)
{
    return filterSqr3(NarrowBourders<true>(), input);
}
IMPLEMENT_FILTER(median)
{
    return filterSqr3(MedianFilter(), input);
}

IMP_CONWAY_FUNCTOR_FILTER(Classic)
{
    return (count == 3 || (alive && (count == 2)));
}

IMP_CONWAY_FUNCTOR_FILTER(HighLife)
{
    return (count == 3 || (alive ? count == 2 : count == 6));
}

IMP_CONWAY_FUNCTOR_FILTER(FreeOrDie)
{
    return (alive ? count == 0 : count == 2);
}

IMP_CONWAY_FUNCTOR_FILTER(My)
{
    return ( (count == 3 || count == 6 || count == 7 || count == 8 || (!alive && (count == 4)))  );
}
