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
 * QQ       : lin.vladimir@gmail.com (2107437784)
 * URL      : http://qtucl.sourceforge.net/
 *
 * QtUCL acts as an interface between Qt and UCL library.
 * Please keep QtUCL as simple as possible.
 *
 * Copyright 2001 ~ 2016
 *
 ****************************************************************************/

#include <qtucl.h>

#include <uclconf.h>
#include <ucl.h>

QT_BEGIN_NAMESPACE

//////////////////////////////////////////////////////////////////////////////

#define IsNull(item)   ( NULL == (item) )
#define NotNull(item)  ( NULL != (item) )
#define NotEqual(a,b)  ( (a)  != (b)    )

#define UCL_MAX_UNUSED ( 8 * 1024       )

/* UCL magic file header for compressed files */
static const unsigned char uclmagic [ 8 ] = {
  0x00                                      ,
  0xe9                                      ,
  0x55                                      , // U
  0x43                                      , // C
  0x4c                                      , // L
  0xff                                      ,
  0x01                                      ,
  0x1a                                    } ;

#pragma pack(push,1)

typedef struct            {
  char       * buffer     ;
  int          bufferSize ;
  int          method     ;
  int          level      ;
  int          blockSize  ;
  int          flags      ;
  bool         fast       ;
  unsigned int checksum   ;
  bool         writing    ;
  bool         header     ;
  bool         tail       ;
} uclFile                 ;

#pragma pack(pop)

int ucl_compress                                   (
      int                                  method  ,
      const        ucl_bytep               src     ,
                   ucl_uint                src_len ,
                   ucl_bytep               dst     ,
                   ucl_uintp               dst_len ,
                   ucl_progress_callback_p cb      ,
      int                                  level   ,
      const struct ucl_compress_config_p   conf    ,
                   ucl_uintp               result  )
{
  switch ( method )                {
    case QtUCL::NRV2B              :
    return ::ucl_nrv2b_99_compress (
             src                   ,
             src_len               ,
             dst                   ,
             dst_len               ,
             cb                    ,
             level                 ,
             conf                  ,
             result              ) ;
    case QtUCL::NRV2D              :
    return ::ucl_nrv2d_99_compress (
             src                   ,
             src_len               ,
             dst                   ,
             dst_len               ,
             cb                    ,
             level                 ,
             conf                  ,
             result              ) ;
    case QtUCL::NRV2E              :
    return ::ucl_nrv2e_99_compress (
             src                   ,
             src_len               ,
             dst                   ,
             dst_len               ,
             cb                    ,
             level                 ,
             conf                  ,
             result              ) ;
  }                                ;
  return UCL_E_ERROR               ;
}

ucl_uint32 xread32(unsigned char * b)
{
  ucl_uint32 v                        ;
  v  = ( (ucl_uint32) b [ 3 ] )       ;
  v |= ( (ucl_uint32) b [ 2 ] ) <<  8 ;
  v |= ( (ucl_uint32) b [ 1 ] ) << 16 ;
  v |= ( (ucl_uint32) b [ 0 ] ) << 24 ;
  return     v                        ;
}

void xwrite32(QByteArray & body,ucl_uint32 v)
{
  unsigned char b [ 4 ]                       ;
  b [ 3 ] = (unsigned char) ( v      ) & 0xFF ;
  b [ 2 ] = (unsigned char) ( v >>  8) & 0xFF ;
  b [ 1 ] = (unsigned char) ( v >> 16) & 0xFF ;
  b [ 0 ] = (unsigned char) ( v >> 24) & 0xFF ;
  body . append ( (const char *) b , 4 )      ;
}

//////////////////////////////////////////////////////////////////////////////

QtUCL:: QtUCL     (void)
      : uclPacket (NULL)
{
}

QtUCL::~QtUCL (void)
{
  CleanUp ( ) ;
}

QString QtUCL::Version(void)
{
  const char * ucls = ucl_version_string ( ) ;
  if ( NULL == ucls ) return ""              ;
  return QString::fromUtf8 ( ucls )          ;
}

bool QtUCL::isUCL (QByteArray & header)
{
  if ( header . size ( ) < 8                ) return false  ;
  unsigned char * hxz = (unsigned char *) header . data ( ) ;
  if ( 0 == ::memcmp ( uclmagic , hxz , 8 ) ) return true   ;
  return false                                              ;
}

bool QtUCL::IsCorrect(int returnCode)
{
  if ( UCL_E_OK == returnCode ) return true ;
  return false                              ;
}

bool QtUCL::IsEnd(int returnCode)
{
  return ( returnCode > UCL_E_OK ) ;
}

bool QtUCL::IsFault(int returnCode)
{
  return ( returnCode < UCL_E_OK ) ;
}

void QtUCL::CleanUp(void)
{
  if ( NULL == uclPacket ) return ;
  uclFile * uf                    ;
  uf = (uclFile *) uclPacket      ;
  if ( NULL != uf -> buffer )     {
    ::free ( uf -> buffer )       ;
    uf -> buffer = NULL           ;
  }                               ;
  ::free ( uf )                   ;
  uclPacket = NULL                ;
}

bool QtUCL::CompressHeader(QByteArray & Compressed)
{
  if ( NULL == uclPacket ) return false                ;
  uclFile * uf = (uclFile *) uclPacket                 ;
  //////////////////////////////////////////////////////
  Compressed . append ( (const char *) uclmagic , 8  ) ;
  xwrite32            ( Compressed , uf -> flags     ) ;
  Compressed . append ( (char)       uf -> method    ) ;
  Compressed . append ( (char)       uf -> level     ) ;
  xwrite32            ( Compressed , uf -> blockSize ) ;
  return true                                          ;
}

bool QtUCL::CompressTail(unsigned int checksum,QByteArray & Compressed)
{
  xwrite32 ( Compressed , 0        ) ;
  xwrite32 ( Compressed , checksum ) ;
  return true                        ;
}

int QtUCL::DecompressHeader(const QByteArray & header)
{
  if ( IsNull ( uclPacket )                 ) return -1      ;
  int len = header . size ( )                                ;
  if ( len  < 8                             ) return -1      ;
  unsigned char * hxz = (unsigned char *) header . data ( )  ;
  if ( 0 != ::memcmp ( uclmagic , hxz , 8 ) ) return -1      ;
  uclFile * uf = (uclFile *) uclPacket                       ;
  if ( uf -> writing                        ) return -1      ;
  ////////////////////////////////////////////////////////////
  int sid         = 8                                        ;
  hxz            += sid                                      ;
  if ( len < 12                             ) return -1      ;
  uf -> flags     = (int) xread32 ( hxz )                    ;
  uf -> fast      = ( 0 == uf -> flags )                     ;
  hxz            += 4                                        ;
  sid            += 4                                        ;
  if ( len < 13                             ) return -1      ;
  uf -> method = (int) (*hxz)                                ;
  hxz            += 1                                        ;
  sid            += 1                                        ;
  if ( len < 14                             ) return -1      ;
  uf -> level  = (int) (*hxz)                                ;
  hxz            += 1                                        ;
  sid            += 1                                        ;
  if ( len < 18                             ) return -1      ;
  uf -> blockSize = (int) xread32 ( hxz )                    ;
  hxz            += 4                                        ;
  sid            += 4                                        ;
  ////////////////////////////////////////////////////////////
  if ( NULL != uf -> buffer ) ::free ( uf -> buffer )        ;
  uf -> bufferSize  = ( uf -> blockSize / 8 )                ;
  uf -> bufferSize +=   256                                  ;
  uf -> bufferSize +=   uf -> blockSize                      ;
  uf -> buffer      = (char *) ::malloc ( uf -> bufferSize ) ;
  ////////////////////////////////////////////////////////////
  return sid                                                 ;
}

int QtUCL::BeginCompress(QVariantList arguments)
{
  int  level     = 10                                                       ;
  int  blocksize = 8192                                                     ;
  int  method    = NRV2B                                                    ;
  bool fast      = false                                                    ;
  if ( arguments . count ( ) > 0 ) level     = arguments [ 0 ] . toInt  ( ) ;
  if ( arguments . count ( ) > 1 ) blocksize = arguments [ 1 ] . toInt  ( ) ;
  if ( arguments . count ( ) > 2 ) method    = arguments [ 2 ] . toInt  ( ) ;
  if ( arguments . count ( ) > 3 ) fast      = arguments [ 3 ] . toBool ( ) ;
  return BeginCompress ( level , method , blocksize , fast )                ;
}

int QtUCL::BeginCompress(int level,int method,int blocksize,bool fast)
{
  CleanUp ( )                                                  ;
  //////////////////////////////////////////////////////////////
  uclFile * uf = NULL                                          ;
  if (level < 1 ) level = 1                                    ;
  if (level > 10) level = 10                                   ;
  //////////////////////////////////////////////////////////////
  if ( UCL_E_OK != ::ucl_init ( ) ) return UCL_E_ERROR         ;
  uf = (uclFile *) ::malloc ( sizeof(uclFile) )                ;
  if ( IsNull ( uf )              ) return UCL_E_OUT_OF_MEMORY ;
  //////////////////////////////////////////////////////////////
  ::memset ( uf , 0 , sizeof(uclFile) )                        ;
  uf -> bufferSize  = ( blocksize / 8 )                        ;
  uf -> bufferSize +=   256                                    ;
  uf -> bufferSize +=   blocksize                              ;
  uf -> method      = method                                   ;
  uf -> level       = level                                    ;
  uf -> blockSize   = blocksize                                ;
  uf -> flags       = fast ? 0 : 1                             ;
  uf -> fast        = fast                                     ;
  uf -> checksum    = ::ucl_adler32 ( 0 , NULL , 0 )           ;
  uf -> writing     = true                                     ;
  uf -> header      = false                                    ;
  uf -> tail        = false                                    ;
  uf -> buffer      = (char *) ::malloc ( uf -> bufferSize )   ;
  //////////////////////////////////////////////////////////////
  uclPacket         = uf                                       ;
  //////////////////////////////////////////////////////////////
  return UCL_E_OK                                              ;
}

int QtUCL::doCompress(const QByteArray & Source,QByteArray & Compressed)
{
  if ( IsNull ( uclPacket ) ) return UCL_E_OUT_OF_MEMORY              ;
  uclFile * uf = (uclFile *) uclPacket                                ;
  if ( ! uf -> writing ) return UCL_E_ERROR                           ;
  /////////////////////////////////////////////////////////////////////
  Compressed . clear ( )                                              ;
  if ( Source . size ( ) <= 0 ) return UCL_E_OK                       ;
  /////////////////////////////////////////////////////////////////////
  if ( ! uf -> header )                                               {
    CompressHeader ( Compressed )                                     ;
    uf -> header = true                                               ;
  }                                                                   ;
  /////////////////////////////////////////////////////////////////////
  int             r                                                   ;
  ucl_uint        in_len                                              ;
  ucl_uint        out_len                                             ;
  ucl_bytep       inp                                                 ;
  ucl_bytep       outp                                                ;
  unsigned char * pp   = (unsigned char *) Source . data ( )          ;
  qint64          len  = Source . size ( )                            ;
  qint64          sid  = 0                                            ;
  qint64          dlen = 0                                            ;
  if ( NULL == pp ) return UCL_E_ERROR                                ;
  /////////////////////////////////////////////////////////////////////
  while ( sid < len )                                                 {
    dlen = len - sid                                                  ;
    if ( dlen > uf -> blockSize ) dlen = uf -> blockSize              ;
    if ( ! uf -> fast )                                               {
      uf -> checksum = ::ucl_adler32 ( uf -> checksum , pp , in_len ) ;
    }                                                                 ;
    in_len  = (ucl_uint) dlen                                         ;
    out_len = 0                                                       ;
    inp     = (ucl_bytep) pp                                          ;
    outp    = (ucl_bytep) uf -> buffer                                ;
    r       = UCL_E_ERROR                                             ;
    r       = ::ucl_compress                                          (
                uf -> method                                          ,
                inp                                                   ,
                in_len                                                ,
                outp                                                  ,
                &out_len                                              ,
                0                                                     ,
                uf -> level                                           ,
                NULL                                                  ,
                NULL                                                ) ;
    if ( UCL_E_OK != r              ) return UCL_E_ERROR              ;
    if ( out_len > uf -> bufferSize ) return UCL_E_ERROR              ;
    if                      ( out_len > 0                )            {
      xwrite32              ( Compressed       , in_len  )            ;
      if                    ( out_len < in_len           )            {
        xwrite32            ( Compressed       , out_len )            ;
        Compressed . append ( uf -> buffer     , out_len )            ;
      } else                                                          {
        xwrite32            ( Compressed       , in_len  )            ;
        Compressed . append ( (const char *)pp , in_len  )            ;
      }                                                               ;
    }                                                                 ;
    sid += dlen                                                       ;
    pp  += dlen                                                       ;
  }                                                                   ;
  /////////////////////////////////////////////////////////////////////
  CompressDone ( Compressed )                                         ;
  return UCL_E_OK                                                     ;
}

int QtUCL::doSection(QByteArray & Source,QByteArray & Compressed)
{
  if ( IsNull ( uclPacket )   ) return UCL_E_OUT_OF_MEMORY            ;
  uclFile * uf = (uclFile *) uclPacket                                ;
  if ( ! uf -> writing        ) return UCL_E_ERROR                    ;
  /////////////////////////////////////////////////////////////////////
  Compressed . clear ( )                                              ;
  if ( Source . size ( ) <= 0 ) return UCL_E_OK                       ;
  /////////////////////////////////////////////////////////////////////
  if ( ! uf -> header )                                               {
    CompressHeader ( Compressed )                                     ;
    uf -> header = true                                               ;
  }                                                                   ;
  /////////////////////////////////////////////////////////////////////
  int             r       = UCL_E_ERROR                               ;
  unsigned char * pp      = (unsigned char *) Source . data ( )       ;
  qint64          len     = Source . size ( )                         ;
  ucl_uint        in_len                                              ;
  ucl_uint        out_len = 0                                         ;
  ucl_bytep       inp     = (ucl_bytep) pp                            ;
  ucl_bytep       outp    = (ucl_bytep) uf -> buffer                  ;
  /////////////////////////////////////////////////////////////////////
  if ( NULL == pp ) return UCL_E_ERROR                                ;
  if ( len > uf -> blockSize )                                        {
    in_len = uf -> blockSize                                          ;
  } else                                                              {
    in_len = len                                                      ;
  }                                                                   ;
  /////////////////////////////////////////////////////////////////////
  if ( ! uf -> fast )                                                 {
    uf -> checksum = ::ucl_adler32 ( uf -> checksum , pp , in_len )   ;
  }                                                                   ;
  r       = ::ucl_compress                                            (
              uf -> method                                            ,
              inp                                                     ,
              in_len                                                  ,
              outp                                                    ,
              &out_len                                                ,
              0                                                       ,
              uf -> level                                             ,
              NULL                                                    ,
              NULL                                                  ) ;
  if ( UCL_E_OK != r              ) return UCL_E_ERROR                ;
  if ( out_len > uf -> bufferSize ) return UCL_E_ERROR                ;
  if                      ( out_len > 0                )              {
    xwrite32              ( Compressed       , in_len  )              ;
    if                    ( out_len < in_len           )              {
      xwrite32            ( Compressed       , out_len )              ;
      Compressed . append ( uf -> buffer     , out_len )              ;
    } else                                                            {
      xwrite32            ( Compressed       , in_len  )              ;
      Compressed . append ( (const char *)pp , in_len  )              ;
    }                                                                 ;
    Source . remove       ( 0 , in_len                 )              ;
  }                                                                   ;
  /////////////////////////////////////////////////////////////////////
  return UCL_E_ERROR                                                  ;
}

int QtUCL::CompressDone(QByteArray & Compressed)
{
  if (IsNull(uclPacket)) return UCL_E_OUT_OF_MEMORY ;
  uclFile * uf = (uclFile *)uclPacket               ;
  ///////////////////////////////////////////////////
  if ( ! uf -> writing ) return UCL_E_ERROR         ;
  if (   uf -> tail    ) return UCL_E_OK            ;
  CompressTail ( uf -> checksum , Compressed )      ;
  uf -> tail = true                                 ;
  ///////////////////////////////////////////////////
  return UCL_E_OK                                   ;
}

int QtUCL::BeginDecompress(void)
{
  CleanUp ( )                                                  ;
  if ( UCL_E_OK != ::ucl_init ( ) ) return UCL_E_ERROR         ;
  //////////////////////////////////////////////////////////////
  uclFile * uf = NULL                                          ;
  uf = (uclFile *) ::malloc ( sizeof(uclFile) )                ;
  if ( IsNull ( uf )              ) return UCL_E_OUT_OF_MEMORY ;
  //////////////////////////////////////////////////////////////
  ::memset ( uf , 0 , sizeof(uclFile) )                        ;
  uf -> bufferSize  = 0                                        ;
  uf -> method      = NRV2B                                    ;
  uf -> level       = 0                                        ;
  uf -> blockSize   = 0                                        ;
  uf -> flags       = 0                                        ;
  uf -> fast        = false                                    ;
  uf -> checksum    = ::ucl_adler32 ( 0 , NULL , 0 )           ;
  uf -> writing     = false                                    ;
  uf -> header      = false                                    ;
  uf -> tail        = false                                    ;
  uf -> buffer      = NULL                                     ;
  //////////////////////////////////////////////////////////////
  uclPacket         = uf                                       ;
  //////////////////////////////////////////////////////////////
  return UCL_E_OK                                              ;
}

int QtUCL::doDecompress(const QByteArray & Source,QByteArray & Decompressed)
{
  if ( IsNull ( uclPacket ) ) return UCL_E_OUT_OF_MEMORY                     ;
  uclFile * uf = (uclFile *) uclPacket                                       ;
  if (   uf -> writing ) return UCL_E_ERROR                                  ;
  if (   uf -> tail    ) return UCL_E_OK                                     ;
  ////////////////////////////////////////////////////////////////////////////
  qint64 sid = 0                                                             ;
  if ( ! uf -> header  )                                                     {
    sid          = DecompressHeader ( Source )                               ;
    uf -> header = true                                                      ;
  }                                                                          ;
  if ( sid < 0         ) return UCL_E_ERROR                                  ;
  ////////////////////////////////////////////////////////////////////////////
  unsigned char * pp = (unsigned char *) Source . data ( )                   ;
  if ( NULL == pp      ) return UCL_E_ERROR                                  ;
  pp += sid                                                                  ;
  ////////////////////////////////////////////////////////////////////////////
  int       r   = UCL_E_ERROR                                                ;
  int       len = Source . size ( )                                          ;
  ucl_bytep inp                                                              ;
  ucl_bytep outp                                                             ;
  ucl_uint  in_len                                                           ;
  ucl_uint  out_len                                                          ;
  ucl_uint  chksum                                                           ;
  while ( true )                                                             {
    if   ( ( sid + 4 ) > len          ) return UCL_E_ERROR                   ;
    out_len  = xread32 ( pp )                                                ;
    pp      += 4                                                             ;
    sid     += 4                                                             ;
    //////////////////////////////////////////////////////////////////////////
    if ( 0 == out_len )                                                      {
      if ( ( sid + 4 ) > len          ) return UCL_E_ERROR                   ;
      if ( ! uf -> fast )                                                    {
        chksum = xread32 ( pp )                                              ;
        if ( chksum != uf -> checksum ) return UCL_E_EOF_NOT_FOUND           ;
      }                                                                      ;
      uf -> tail = true                                                      ;
      return UCL_E_OK                                                        ;
    }                                                                        ;
    //////////////////////////////////////////////////////////////////////////
    if   ( ( sid + 4 ) > len        ) return UCL_E_ERROR                     ;
    in_len   = xread32 ( pp )                                                ;
    pp      += 4                                                             ;
    sid     += 4                                                             ;
    inp      = (ucl_bytep) pp                                                ;
    outp     = (ucl_bytep) uf -> buffer                                      ;
    if ( ( sid + in_len ) > len     ) return UCL_E_INPUT_OVERRUN             ;
    //////////////////////////////////////////////////////////////////////////
    if ( in_len < out_len )                                                  {
      ucl_uint new_len = out_len                                             ;
      switch ( uf -> method )                                                {
        case NRV2B                                                           :
          if ( uf -> fast )                                                  {
            r = ::ucl_nrv2b_decompress_8                                     (
                  inp                                                        ,
                  in_len                                                     ,
                  outp                                                       ,
                  &new_len                                                   ,
                  NULL                                                     ) ;
          } else                                                             {
            r = ::ucl_nrv2b_decompress_safe_8                                (
                  inp                                                        ,
                  in_len                                                     ,
                  outp                                                       ,
                  &new_len                                                   ,
                  NULL                                                     ) ;
          }                                                                  ;
        break                                                                ;
        case NRV2D                                                           :
          if ( uf -> fast )                                                  {
            r = ::ucl_nrv2d_decompress_8                                     (
                  inp                                                        ,
                  in_len                                                     ,
                  outp                                                       ,
                  &new_len                                                   ,
                  NULL                                                     ) ;
          } else                                                             {
            r = ::ucl_nrv2d_decompress_safe_8                                (
                  inp                                                        ,
                  in_len                                                     ,
                  outp                                                       ,
                  &new_len                                                   ,
                  NULL                                                     ) ;
          }                                                                  ;
        break                                                                ;
        case NRV2E                                                           :
          if ( uf -> fast )                                                  {
            r = ::ucl_nrv2e_decompress_8                                     (
                  inp                                                        ,
                  in_len                                                     ,
                  outp                                                       ,
                  &new_len                                                   ,
                  NULL                                                     ) ;
          } else                                                             {
            r = ::ucl_nrv2e_decompress_safe_8                                (
                  inp                                                        ,
                  in_len                                                     ,
                  outp                                                       ,
                  &new_len                                                   ,
                  NULL                                                     ) ;
          }                                                                  ;
        break                                                                ;
      }                                                                      ;
      if ( UCL_E_OK != r      ) return UCL_E_ERROR                           ;
      if ( new_len != out_len ) return UCL_E_ERROR                           ;
      Decompressed . append ( (const char *) outp , out_len )                ;
      if ( ! uf -> fast )                                                    {
        uf -> checksum = ::ucl_adler32 ( uf -> checksum , outp , out_len )   ;
      }                                                                      ;
    } else                                                                   {
      Decompressed . append ( (const char *) pp   , in_len  )                ;
      if ( ! uf -> fast )                                                    {
        uf -> checksum = ::ucl_adler32 ( uf -> checksum , pp , in_len )      ;
      }                                                                      ;
    }                                                                        ;
    //////////////////////////////////////////////////////////////////////////
    pp  += in_len                                                            ;
    sid += in_len                                                            ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  return UCL_E_ERROR                                                         ;
}

int QtUCL::undoSection(QByteArray & Source,QByteArray & Decompressed)
{
  if ( IsNull ( uclPacket ) ) return UCL_E_OUT_OF_MEMORY                     ;
  uclFile * uf = (uclFile *) uclPacket                                       ;
  if (   uf -> writing ) return UCL_E_ERROR                                  ;
  if (   uf -> tail    ) return UCL_E_OK                                     ;
  ////////////////////////////////////////////////////////////////////////////
  qint64 sid = 0                                                             ;
  if ( ! uf -> header  )                                                     {
    sid          = DecompressHeader ( Source )                               ;
    uf -> header = true                                                      ;
  }                                                                          ;
  if ( sid < 0         ) return UCL_E_ERROR                                  ;
  ////////////////////////////////////////////////////////////////////////////
  unsigned char * pp = (unsigned char *) Source . data ( )                   ;
  if ( NULL == pp      ) return UCL_E_ERROR                                  ;
  pp += sid                                                                  ;
  ////////////////////////////////////////////////////////////////////////////
  int       r   = UCL_E_ERROR                                                ;
  int       len = Source . size ( )                                          ;
  ucl_bytep inp                                                              ;
  ucl_bytep outp                                                             ;
  ucl_uint  in_len                                                           ;
  ucl_uint  out_len                                                          ;
  ucl_uint  chksum                                                           ;
  ////////////////////////////////////////////////////////////////////////////
  if   ( ( sid + 4 ) > len          ) return UCL_E_ERROR                     ;
  out_len  = xread32 ( pp )                                                  ;
  pp      += 4                                                               ;
  sid     += 4                                                               ;
  ////////////////////////////////////////////////////////////////////////////
  if ( 0 == out_len )                                                        {
    if ( ( sid + 4 ) > len          ) return UCL_E_ERROR                     ;
    if ( ! uf -> fast )                                                      {
      chksum = xread32 ( pp )                                                ;
      if ( chksum != uf -> checksum ) return UCL_E_EOF_NOT_FOUND             ;
    }                                                                        ;
    sid       += 4                                                           ;
    uf -> tail = true                                                        ;
    Source . remove ( 0 , sid )                                              ;
    return UCL_E_OK                                                          ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  if   ( ( sid + 4 ) > len          ) return UCL_E_ERROR                     ;
  in_len   = xread32 ( pp )                                                  ;
  pp      += 4                                                               ;
  sid     += 4                                                               ;
  inp      = (ucl_bytep) pp                                                  ;
  outp     = (ucl_bytep) uf -> buffer                                        ;
  if   ( ( sid + in_len ) > len     ) return UCL_E_INPUT_OVERRUN             ;
  ////////////////////////////////////////////////////////////////////////////
  if ( in_len < out_len )                                                    {
    ucl_uint new_len = out_len                                               ;
    switch ( uf -> method )                                                  {
      case NRV2B                                                             :
        if ( uf -> fast )                                                    {
          r = ::ucl_nrv2b_decompress_8                                       (
                inp                                                          ,
                in_len                                                       ,
                outp                                                         ,
                &new_len                                                     ,
                NULL                                                       ) ;
        } else                                                               {
          r = ::ucl_nrv2b_decompress_safe_8                                  (
                inp                                                          ,
                in_len                                                       ,
                outp                                                         ,
                &new_len                                                     ,
                NULL                                                       ) ;
        }                                                                    ;
      break                                                                  ;
      case NRV2D                                                             :
        if ( uf -> fast )                                                    {
          r = ::ucl_nrv2d_decompress_8                                       (
                inp                                                          ,
                in_len                                                       ,
                outp                                                         ,
                &new_len                                                     ,
                NULL                                                       ) ;
        } else                                                               {
          r = ::ucl_nrv2d_decompress_safe_8                                  (
                inp                                                          ,
                in_len                                                       ,
                outp                                                         ,
                &new_len                                                     ,
                NULL                                                       ) ;
        }                                                                    ;
      break                                                                  ;
      case NRV2E                                                             :
        if ( uf -> fast )                                                    {
          r = ::ucl_nrv2e_decompress_8                                       (
                inp                                                          ,
                in_len                                                       ,
                outp                                                         ,
                &new_len                                                     ,
                NULL                                                       ) ;
        } else                                                               {
          r = ::ucl_nrv2e_decompress_safe_8                                  (
                inp                                                          ,
                in_len                                                       ,
                outp                                                         ,
                &new_len                                                     ,
                NULL                                                       ) ;
        }                                                                    ;
      break                                                                  ;
    }                                                                        ;
    if ( UCL_E_OK != r      ) return UCL_E_ERROR                             ;
    if ( new_len != out_len ) return UCL_E_ERROR                             ;
    Decompressed . append ( (const char *) outp , out_len )                  ;
    if ( ! uf -> fast )                                                      {
      uf -> checksum = ::ucl_adler32 ( uf -> checksum , outp , out_len )     ;
    }                                                                        ;
  } else                                                                     {
    Decompressed . append ( (const char *) pp   , in_len  )                  ;
    if ( ! uf -> fast )                                                      {
      uf -> checksum = ::ucl_adler32 ( uf -> checksum , pp , in_len )        ;
    }                                                                        ;
  }                                                                          ;
  ////////////////////////////////////////////////////////////////////////////
  pp  += in_len                                                              ;
  sid += in_len                                                              ;
  Source . remove ( 0 , sid )                                                ;
  ////////////////////////////////////////////////////////////////////////////
  return UCL_E_OK                                                            ;
}

int QtUCL::DecompressDone(void)
{
  if ( IsNull ( uclPacket ) ) return UCL_E_OUT_OF_MEMORY ;
  uclFile * uf = (uclFile *) uclPacket                   ;
  if (   uf -> writing ) return UCL_E_ERROR              ;
  if (   uf -> tail    ) return UCL_E_OK                 ;
  ////////////////////////////////////////////////////////
  uf -> tail = true                                      ;
  ////////////////////////////////////////////////////////
  return UCL_E_OK                                        ;
}

bool QtUCL::IsTail(QByteArray & header)
{
  if ( NULL == uclPacket     ) return false                     ;
  if ( header . size ( ) < 8 ) return false                     ;
  unsigned char * footer  = (unsigned char *) header . data ( ) ;
  bool            correct = true                                ;
  return correct                                                ;
}

//////////////////////////////////////////////////////////////////////////////

bool ToUCL ( const QByteArray & data      ,
                   QByteArray & ucl       ,
             int                level     ,
             int                method    ,
             int                blocksize ,
             bool               fast      )
{
  if ( data . size ( ) <= 0 ) return false ;
  //////////////////////////////////////////
  QtUCL        L                           ;
  int          r                           ;
  QVariantList v                           ;
  v << level                               ;
  v << method                              ;
  v << blocksize                           ;
  v << fast                                ;
  r = L . BeginCompress ( v )              ;
  if ( L . IsCorrect ( r ) )               {
    L . doCompress   ( data , ucl )        ;
    L . CompressDone (        ucl )        ;
  }                                        ;
  //////////////////////////////////////////
  return ( ucl . size ( ) > 0 )            ;
}

//////////////////////////////////////////////////////////////////////////////

bool FromUCL(const QByteArray & ucl,QByteArray & data)
{
  if ( ucl . size ( ) <= 0 ) return false ;
  /////////////////////////////////////////
  QtUCL L                                 ;
  int   r                                 ;
  r = L . BeginDecompress ( )             ;
  if ( L . IsCorrect ( r ) )              {
    L . doDecompress   ( ucl , data )     ;
    L . DecompressDone (            )     ;
  }                                       ;
  /////////////////////////////////////////
  return ( data . size ( ) > 0 )          ;
}

//////////////////////////////////////////////////////////////////////////////

bool SaveUCL ( QString      filename  ,
               QByteArray & data      ,
               int          level     ,
               int          method    ,
               int          blocksize ,
               bool         fast      )
{
  if ( data . size ( ) <= 0 ) return false                          ;
  QByteArray ucl                                                    ;
  if ( level < 1  ) level = 1                                       ;
  if ( level > 10 ) level = 10                                      ;
  if ( ! ToUCL ( data , ucl , level , method , blocksize , fast ) ) {
    return false                                                    ;
  }                                                                 ;
  if ( ucl . size ( ) <= 0  ) return false                          ;
  QFile F ( filename )                                              ;
  if ( ! F . open ( QIODevice::WriteOnly | QIODevice::Truncate ) )  {
    return false                                                    ;
  }                                                                 ;
  F . write ( ucl )                                                 ;
  F . close (     )                                                 ;
  return true                                                       ;
}

//////////////////////////////////////////////////////////////////////////////

bool LoadUCL (QString filename,QByteArray & data)
{
  QFile F ( filename )                                   ;
  if ( ! F . open ( QIODevice::ReadOnly ) ) return false ;
  QByteArray ucl                                         ;
  ucl = F . readAll ( )                                  ;
  F . close         ( )                                  ;
  if ( ucl . size ( ) <= 0 ) return false                ;
  if ( ! FromUCL ( ucl , data ) ) return false           ;
  return ( data . size ( ) > 0 )                         ;
}

//////////////////////////////////////////////////////////////////////////////

bool FileToUCL ( QString filename  ,
                 QString ucl       ,
                 int     level     ,
                 int     method    ,
                 int     blocksize ,
                 bool    fast      )
{
  QFile F ( filename )                                              ;
  if ( ! F . open ( QIODevice::ReadOnly ) ) return false            ;
  QByteArray data                                                   ;
  data = F . readAll ( )                                            ;
  F . close ( )                                                     ;
  if ( data . size ( ) <= 0 ) return false                          ;
  return SaveUCL ( ucl , data , level , method , blocksize , fast ) ;
}

//////////////////////////////////////////////////////////////////////////////

bool UCLToFile (QString ucl,QString filename)
{
  QByteArray data                                        ;
  if ( ! LoadUCL ( ucl , data ) ) return false           ;
  if ( data . size ( ) <=0      ) return false           ;
  QFile F ( filename )                                   ;
  if ( ! F . open ( QIODevice::WriteOnly                 |
                    QIODevice::Truncate ) ) return false ;
  F . write ( data )                                     ;
  F . close (      )                                     ;
  return true                                            ;
}

QT_END_NAMESPACE
