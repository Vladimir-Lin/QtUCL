/****************************************************************************
 *
 * Copyright (C) 2001~2016 Neutrino International Inc.
 *
 * Author   : Brian Lin ( Foxman , Vladimir Lin , Vladimir Forest )
 * E-mail   : lin.foxman@gmail.com
 *          : lin.vladimir@gmail.com
 *          : wolfram_lin@yahoo.com
 *          : wolfram_lin@sina.com
 *          : wolfram_lin@163.com
 * Skype    : wolfram_lin
 * WeChat   : 153-0271-7160
 * WhatsApp : 153-0271-7160
 * QQ       : lin.vladimir@gmail.com (2107437784)
 * URL      : http://qtucl.sourceforge.net/
 *
 * QtUCL acts as an interface between Qt and UCL library.
 * Please keep QtUCL as simple as possible.
 *
 * Copyright 2001 ~ 2016
 *
 ****************************************************************************/

#ifndef QT_UCL_H
#define QT_UCL_H

#include <QtCore>
#include <QtScript>

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#    if defined(QT_BUILD_QTUCL_LIB)
#      define Q_UCL_EXPORT Q_DECL_EXPORT
#    else
#      define Q_UCL_EXPORT  Q_DECL_IMPORT
#    endif
#else
#    define Q_UCL_EXPORT
#endif

#define QT_UCL_LIB 1

class Q_UCL_EXPORT QtUCL         ;
class Q_UCL_EXPORT ScriptableUCL ;

// Lempel-Ziv-Oberhumer

class Q_UCL_EXPORT QtUCL
{
  public:

    typedef enum   {
      NRV2B = 0x2B ,
      NRV2D = 0x2D ,
      NRV2E = 0x2E ,
    } UclMethods   ;

    explicit        QtUCL              (void) ;
    virtual        ~QtUCL              (void) ;

    static  QString Version            (void) ;

    virtual bool    isUCL              (QByteArray & header) ;

    virtual void    CleanUp            (void) ;

    virtual bool    IsCorrect          (int returnCode) ;
    virtual bool    IsEnd              (int returnCode) ;
    virtual bool    IsFault            (int returnCode) ;

    // Compression functions

    virtual int     BeginCompress      (int level     = 10      ,
                                        int method    = NRV2B   ,
                                        int blocksize = 8192    ,
                                        bool fast     = false ) ;
    virtual int     BeginCompress      (QVariantList arguments = QVariantList() ) ;
    virtual int     doCompress         (const QByteArray & Source      ,
                                              QByteArray & Compressed) ;
    virtual int     doSection          (      QByteArray & Source      ,
                                              QByteArray & Compressed) ;
    virtual int     CompressDone       (QByteArray & Compressed) ;

    // Decompression functions

    virtual int     BeginDecompress    (void) ;
    virtual int     doDecompress       (const QByteArray & Source        ,
                                              QByteArray & Decompressed) ;
    virtual int     undoSection        (      QByteArray & Source        ,
                                              QByteArray & Decompressed) ;
    virtual int     DecompressDone     (void) ;

    virtual bool    IsTail             (QByteArray & header) ;

  protected:

    void * uclPacket ;

    virtual bool    CompressHeader     (QByteArray & Compressed) ;
    virtual bool    CompressTail       (unsigned int checksum,QByteArray & Compressed) ;
    virtual int     DecompressHeader   (const QByteArray & Source) ;

  private:

};

class Q_UCL_EXPORT ScriptableUCL : public QObject
                                 , public QScriptable
                                 , public QtUCL
{
  Q_OBJECT
  public:

    static QScriptValue Attachment    (QScriptContext * context,QScriptEngine * engine) ;

    explicit            ScriptableUCL (QObject * parent) ;
    virtual            ~ScriptableUCL (void) ;

  protected:

  private:

  public slots:

    virtual bool        ToUCL         (QString file                     ,
                                       QString ucl                      ,
                                       int     level     = 10           ,
                                       int     method    = QtUCL::NRV2B ,
                                       int     blocksize = 8192         ,
                                       bool    fast      = false      ) ;
    virtual bool        ToFile        (QString ucl                      ,
                                       QString file                   ) ;

  protected slots:

  private slots:

  signals:

} ;

Q_UCL_EXPORT bool ToUCL     (const QByteArray & data                     ,
                                   QByteArray & ucl                      ,
                             int                level     = 10           ,
                             int                method    = QtUCL::NRV2B ,
                             int                blocksize = 8192         ,
                             bool               fast      = false      ) ;
Q_UCL_EXPORT bool FromUCL   (const QByteArray & ucl                      ,
                                   QByteArray & data                   ) ;
Q_UCL_EXPORT bool SaveUCL   (QString            filename                 ,
                             QByteArray       & data                     ,
                             int                level     = 10           ,
                             int                method    = QtUCL::NRV2B ,
                             int                blocksize = 8192         ,
                             bool               fast      = false      ) ;
Q_UCL_EXPORT bool LoadUCL   (QString            filename                 ,
                             QByteArray       & data                   ) ;
Q_UCL_EXPORT bool FileToUCL (QString            file                     ,
                             QString            ucl                      ,
                             int                level     = 10           ,
                             int                method    = QtUCL::NRV2B ,
                             int                blocksize = 8192         ,
                             bool               fast      = false      ) ;
Q_UCL_EXPORT bool UCLToFile (QString            ucl                      ,
                             QString            file                   ) ;

QT_END_NAMESPACE

#endif
