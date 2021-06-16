/****************************************************************************
 *
 * Copyright (C) 2016 Neutrino International Inc.
 *
 * Author   : Brian Lin ( Vladimir Lin , Vladimir Forest )
 * E-mail   : lin.foxman@gmail.com
 *          : lin.vladimir@gmail.com
 *          : wolfram_lin@yahoo.com
 *          : wolfram_lin@sina.com
 *          : wolfram_lin@163.com
 * Skype    : wolfram_lin
 * WeChat   : 153-0271-7160
 * WhatsApp : 153-0271-7160
 * QQ       : lin.vladimir@gmail.com
 * URL      : http://qtucl.sourceforge.net/
 *
 * QtUCL acts as an interface between Qt and UCL library.
 * Please keep QtUCL as simple as possible.
 *
 * Copyright 2001 ~ 2016
 *
 ****************************************************************************/

#include <qtucl.h>

QT_BEGIN_NAMESPACE

ScriptableUCL:: ScriptableUCL ( QObject * parent )
              : QObject       (           parent )
              , QScriptable   (                  )
              , QtUCL         (                  )
{
}

ScriptableUCL::~ScriptableUCL (void)
{
}

bool ScriptableUCL::ToUCL (
       QString file       ,
       QString ucl        ,
       int     level      ,
       int     method     ,
       int     blocksize  ,
       bool    fast       )
{
  return FileToUCL ( file , ucl , level , method , blocksize, fast ) ;
}

bool ScriptableUCL::ToFile(QString ucl,QString file)
{
  return UCLToFile ( ucl , file ) ;
}

QScriptValue ScriptableUCL::Attachment(QScriptContext * context,QScriptEngine * engine)
{
  ScriptableUCL * ucl = new ScriptableUCL ( engine ) ;
  return engine -> newQObject             ( ucl    ) ;
}

QT_END_NAMESPACE
