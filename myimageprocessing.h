#ifndef MYIMAGEPROCESSING_H
#define MYIMAGEPROCESSING_H

#include <QImage>
#include <QVector>

#define FUNCTOR(FunctorName, RETURN_TYPE, ARG_LIST)\
    struct FunctorName\
    {\
    public:\
        inline RETURN_TYPE operator()ARG_LIST const;

#define DEFINE_FILTER(filterName)  \
static QImage* filterName(QImage& input); \
class T##filterName: ImageProcessor \
{ \
    public: \
        const static T##filterName instance; \
        T##filterName(); \
        QImage* apply(QImage& input); \
};
#define IMP_FUNCTOR(FunctorName,RETURN_TYPE,ARG_LIST) RETURN_TYPE MyImageProcessing::FunctorName::operator() ARG_LIST const

#define IMPLEMENT_FILTER(filterName)  \
const MyImageProcessing::T##filterName MyImageProcessing::T##filterName::instance = MyImageProcessing::T##filterName(); \
MyImageProcessing::T##filterName::T##filterName(): \
    MyImageProcessing::ImageProcessor(#filterName) \
{ \
    filterList.append(this); \
} \
QImage* MyImageProcessing::T##filterName::apply(QImage& input) \
{ \
    return filterName(input); \
} \
QImage* MyImageProcessing::filterName(QImage& input)

class MyImageProcessing
{
public:
    class ImageProcessor
    {
    public:
        QString name;
        ImageProcessor(const QString& name);
        virtual QImage* apply(QImage& input) = 0;
    };

    static QVector<ImageProcessor*> filterList;

    DEFINE_FILTER(straightenBourders)
    DEFINE_FILTER(straightenBourdersSharper)
    DEFINE_FILTER(narrowBourders)
    DEFINE_FILTER(narrowBourdersWay8)
    DEFINE_FILTER(median)

    static inline uchar median(uchar* const left, uchar * const right, uchar* const mid);

private:        


//define functor for procwssing besed on work with 8(+1) ajacent pixels
#define SQR3_ARGLIST (const uchar& topleft,const  uchar& left,const  uchar& leftbot, \
        const uchar& top, const uchar& center, const uchar& bot,  const uchar& righttop,  const uchar& right, const uchar& botright)
#define SQR3_FUNCTOR(FunctorName) FUNCTOR(FunctorName, uchar, SQR3_ARGLIST)
#define IMP_SQR3_FUNCTOR(FunctorName)  IMP_FUNCTOR(FunctorName,uchar,SQR3_ARGLIST)
//define conway functor for alternative conway rules
#define CONWAY_FUNCTOR_ARGLIST (const uchar& count, const bool& alive)
#define CONWAY_FUNCTOR(FunctorName) FUNCTOR(FunctorName, bool, CONWAY_FUNCTOR_ARGLIST)
#define IMP_CONWAY_FUNCTOR(FunctorName)  IMP_FUNCTOR(FunctorName, bool, CONWAY_FUNCTOR_ARGLIST)
//make filter for each new conway
#define CONWAY_FUNCTOR_FILTER(Name) \
    public: DEFINE_FILTER(conway##Name) \
    private: CONWAY_FUNCTOR(Conway##Name)};


#define IMP_CONWAY_FUNCTOR_FILTER(Name) \
         IMPLEMENT_FILTER(conway##Name)\
         {\
            return filterSqr3(Conway<Conway##Name>(Conway##Name()), input);\
         }\
         IMP_FUNCTOR(Conway##Name, bool, CONWAY_FUNCTOR_ARGLIST)




    template <bool SHARPER>
    SQR3_FUNCTOR(StraightenBourders)};
    template <bool WAY8>
    SQR3_FUNCTOR(NarrowBourders)};
    SQR3_FUNCTOR(MedianFilter)};

    CONWAY_FUNCTOR_FILTER(Classic)
    CONWAY_FUNCTOR_FILTER(HighLife)
    CONWAY_FUNCTOR_FILTER(FreeOrDie)
    CONWAY_FUNCTOR_FILTER(My)
    template <typename F>
    SQR3_FUNCTOR(Conway)
       const F& f;
       Conway(const F& f);
    };


    template <typename F>
    static QImage* filterSqr3(const F& f, QImage& input);
};

#endif // MYIMAGEPROCESSING_H
