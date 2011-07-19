// [Fog-Core]
//
// [License]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Global/Init_p.h>
#include <Fog/Core/Math/Math.h>
#include <Fog/Core/Memory/Alloc.h>
#include <Fog/Core/Memory/BSwap.h>
#include <Fog/Core/Memory/Ops.h>
#include <Fog/Core/Threading/Lock.h>
#include <Fog/Core/Threading/Thread.h>
#include <Fog/Core/Tools/Byte.h>
#include <Fog/Core/Tools/ByteArray.h>
#include <Fog/Core/Tools/ByteArrayTmp_p.h>
#include <Fog/Core/Tools/CharData.h>
#include <Fog/Core/Tools/String.h>
#include <Fog/Core/Tools/StringUtil.h>
#include <Fog/Core/Tools/TextCodec.h>
#include <Fog/Core/Tools/TextCodec_Page8Tables_p.h>

#if defined(FOG_OS_WINDOWS)
# include <locale.h>   // setlocale(LC_ALL, NULL)
#else
# include <langinfo.h> // nl_langinfo(CODESET)
#endif // FOG_OS_WINDOWS

namespace Fog {

// ============================================================================
// [Fog::TextCodecHandler - Construction / Destruction]
// ============================================================================

TextCodecHandler::~TextCodecHandler()
{
}

// Default implementation of TextCodecHandler which replaces the unencodable
// character by the escaping sequence "\uXXXX".
struct TextCodecDefaultHandler : public TextCodecHandler
{
  virtual ~TextCodecDefaultHandler() {}

  virtual err_t replaceCharacter(ByteArray& dst, uint32_t uc)
  {
    return dst.appendFormat("\\u%0.4X", uc);
  }
};

static Static<TextCodecDefaultHandler> _TextCodecHandler_default;

// ============================================================================
// [Fog::TextCodec - BuiltIn]
// ============================================================================

struct TextCodecItem;
typedef TextCodecData* (FOG_CDECL *TextCodecCreateFn)(const TextCodecItem& item);

struct TextCodecItem
{
  uint32_t code;
  uint32_t flags;

  TextCodecCreateFn create;
  const char* mime;
  const void* table;
};

// ============================================================================
// [Fog::TextCodec - Helpers]
// ============================================================================

static FOG_INLINE TextCodecData* TextCodec_ref(TextCodecData* d)
{
  d->refCount.inc();
  return d;
}

static FOG_INLINE void TextCodec_deref(TextCodecData* d)
{
  if (d->refCount.deref()) d->destroy(d);
}

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
# define _FOG_TEXTCODEC_IS_BYTESWAPPED(_Flags_) ((_Flags_ & TEXT_ENCODING_IS_BE) != 0)
#else
# define _FOG_TEXTCODEC_IS_BYTESWAPPED(_Flags_) ((_Flags_ & TEXT_ENCODING_IS_LE) != 0)
#endif // FOG_BYTE_ORDER

static FOG_INLINE bool TextCodec_isByteSwappedOnInit(const TextCodecData* d)
{
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
  return (d->flags & TEXT_ENCODING_IS_BE) != 0;
#else
  return (d->flags & TEXT_ENCODING_IS_LE) != 0;
#endif // FOG_BYTE_ORDER
}

static size_t TextCodec_addToState(TextCodecState* state, const uint8_t* cur, const uint8_t* end)
{
  FOG_ASSERT(state != NULL);
  FOG_ASSERT(state->_bufferLength <= 4);

  uint32_t len = (uint32_t)state->_bufferLength;
  uint32_t add = (uint32_t)Math::min((size_t)(end - cur), (size_t)(4 - len));

  memcpy(state->_buffer + len, cur, add);
  state->_bufferLength += (uint8_t)add;

  return state->_bufferLength;
}

// ============================================================================
// [Fog::TextCodec - Helpers - Encode]
// ============================================================================

#define _FOG_TEXTCODEC_ENCODE_INIT_VARS(_GrowBy_) \
  /* Length initialization and check. */ \
  if (srcLength == DETECT_LENGTH) srcLength = StringUtil::len(src); \
  if (srcLength == 0) return ERR_OK; \
  \
  /* Use default handler if not provided. */ \
  if (handler == NULL) handler = _TextCodecHandler_default.instancep(); \
  \
  /* Source buffer. */ \
  const Char* srcCur = src; \
  const Char* srcEnd = src + srcLength; \
  \
  /* Destination buffer */ \
  size_t initSize = dst.getLength(); \
  size_t growSize = _GrowBy_; \
  \
  err_t err = dst.grow(growSize); \
  if (FOG_IS_ERROR(err)) return err; \
  \
  uint8_t* dstCur = reinterpret_cast<uint8_t*>(dst.getDataX()) + initSize; \
  \
  /* Input characters. */ \
  uint32_t uc, ucSurrogate;

#define _FOG_TEXTCODEC_ENCODE_INIT_STATE() \
  /* Streaming */ \
  if (state != NULL && state->getBufferLength() == 2) \
  { \
    uc = reinterpret_cast<uint16_t*>(state->_buffer)[0]; \
    state->_bufferLength = 0; \
    goto _SurrogateTrail; \
  }

#define _FOG_TEXTCODEC_ENCODE_DO_SURROGATE(_DoEncode_) \
  /* Handle the incomplete surrogate pair. */ \
  if (FOG_UNLIKELY(srcCur == srcEnd)) \
  { \
    if (state != NULL) \
    { \
      /* Save the incomplete character to the state-buffer. */ \
      reinterpret_cast<uint16_t*>(state->_buffer)[0] = (uint16_t)uc; \
      state->_bufferLength = 2; \
      goto _End; \
    } \
    else \
    { \
      /* Input is truncated. */ \
      err = ERR_STRING_TRUNCATED; \
      goto _End; \
    } \
  } \
  \
  /* Read the surrogate-trail character and convert to one 32-bit code-point. */ \
_SurrogateTrail: \
  ucSurrogate = *srcCur++; \
  if (!Char::isLoSurrogate(ucSurrogate)) \
  { \
    err = ERR_STRING_INVALID_UTF16; \
    goto _End; \
  } \
  \
  if (_DoEncode_) \
  { \
    uc = Char::ucs4FromSurrogate(uc, ucSurrogate); \
  }

// ============================================================================
// [Fog::TextCodec - None]
// ============================================================================

static TextCodecData TextCodec_None_instance;

static err_t FOG_CDECL TextCodec_None_encode(const TextCodecData* d,
  ByteArray& dst, const Char* src, size_t srcLength, TextCodecState* state,
  TextCodecHandler* handler)
{
  return ERR_STRING_INVALID_CODEC;
}

static err_t FOG_CDECL TextCodec_None_decode(const TextCodecData* d,
  String& dst, const void* src, size_t srcSize, TextCodecState* state)
{
  return ERR_STRING_INVALID_CODEC;
}

static void FOG_CDECL TextCodec_None_destroy(TextCodecData* d)
{
}

static TextCodecData* TextCodec_None_create(const TextCodecItem& item)
{
  TextCodecData* d = &TextCodec_None_instance;

  d->refCount.init(1);
  d->destroy = TextCodec_None_destroy;
  d->encode = TextCodec_None_encode;
  d->decode = TextCodec_None_decode;

  d->code = item.code;
  d->flags = item.flags;
  d->mime = item.mime;
  d->page8 = NULL;

  return d;
}

// ============================================================================
// [Fog::TextCodec - ISO-8859-1]
// ============================================================================

static err_t FOG_CDECL TextCodec_ISO8859_1_encode(const TextCodecData* d,
  ByteArray& dst, const Char* src, size_t srcLength, TextCodecState* state,
  TextCodecHandler* handler)
{
  _FOG_TEXTCODEC_ENCODE_INIT_VARS(srcLength + 1)
  _FOG_TEXTCODEC_ENCODE_INIT_STATE()

  while (srcCur != srcEnd)
  {
    uc = *srcCur++;

    if (FOG_LIKELY(uc < 256))
    {
      *dstCur++ = (uint8_t)uc;
    }
    else
    {
      if (FOG_UNLIKELY(Char::isHiSurrogate(uc)))
      {
        _FOG_TEXTCODEC_ENCODE_DO_SURROGATE(true)
      }

      // Finalize dst and use handler to generate ASCII representation.
      dst.finishDataX(reinterpret_cast<char*>(dstCur));

      err = handler->replaceCharacter(dst, uc);
      if (FOG_IS_ERROR(err)) return err;

      // Prepare buffer for next characters.
      err = dst.reserve(dst.getLength() + (size_t)(srcEnd - srcCur));
      if (FOG_IS_ERROR(err)) return err;

      dstCur = reinterpret_cast<uint8_t*>(dst.getDataX()) + dst.getLength();
    }
  }

_End:
  dst.finishDataX(reinterpret_cast<char*>(dstCur));
  return err;
}

static err_t FOG_CDECL TextCodec_ISO8859_1_decode(const TextCodecData* d,
  String& dst, const void* src, size_t srcSize, TextCodecState* state)
{
  // Source Buffer.
  if (srcSize == DETECT_LENGTH) srcSize = strlen(reinterpret_cast<const char*>(src));

  const uint8_t* srcCur = reinterpret_cast<const uint8_t*>(src);
  const uint8_t* srcEnd = srcCur + srcSize;

  // Destination Buffer.
  size_t initSize = dst.getLength();
  size_t growSize = srcSize + 1;

  err_t err = dst.grow(growSize);
  if (FOG_IS_ERROR(err)) return err;

  Char* dstCur = dst.getDataX() + initSize;

  for (;;)
  {
    *dstCur++ = *srcCur++;
    if (srcCur == srcEnd) break;
  }

  dst.finishDataX(dstCur);
  return ERR_OK;
}

// ============================================================================
// [Fog::TextCodec - 8-Bit]
// ============================================================================

static const TextCodecPage8::Decode TextCodec_8Bit_empty =
{
  {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  }
};

static err_t FOG_CDECL TextCodec_8Bit_encode(const TextCodecData* d,
  ByteArray& dst, const Char* src, size_t srcLength, TextCodecState* state,
  TextCodecHandler* handler)
{
  // 8-bit tables.
  TextCodecPage8::Decode* const* table = d->page8->decode;
  uint8_t b;

  _FOG_TEXTCODEC_ENCODE_INIT_VARS(srcLength + 1)
  _FOG_TEXTCODEC_ENCODE_INIT_STATE()

  while (srcCur != srcEnd)
  {
    uc = *srcCur++;
    if (FOG_UNLIKELY(Char::isHiSurrogate(uc)))
    {
      _FOG_TEXTCODEC_ENCODE_DO_SURROGATE(true)
      goto _ReplaceCharacter;
    }
    else
    {
      b = table[uc >> 8]->uc[uc & 0xFF];
      if (FOG_LIKELY(b != 0))
      {
        *dstCur++ = b;
      }
      else
      {
_ReplaceCharacter:
        // Finalize dst and use handler to generate ASCII representation.
        dst.finishDataX(reinterpret_cast<char*>(dstCur));

        err = handler->replaceCharacter(dst, uc);
        if (FOG_IS_ERROR(err)) return err;

        // Prepare buffer for next characters.
        err = dst.reserve(dst.getLength() + (size_t)(srcEnd - srcCur));
        if (FOG_IS_ERROR(err)) return err;

        dstCur = reinterpret_cast<uint8_t*>(dst.getDataX()) + dst.getLength();
      }
    }
  }

_End:
  dst.finishDataX(reinterpret_cast<char*>(dstCur));
  return err;
}

static err_t FOG_CDECL TextCodec_8Bit_decode(const TextCodecData* d,
  String& dst, const void* src, size_t srcSize, TextCodecState* state)
{
  // Source Buffer.
  if (srcSize == DETECT_LENGTH) srcSize = strlen(reinterpret_cast<const char*>(src));

  const uint8_t* srcCur = reinterpret_cast<const uint8_t*>(src);
  const uint8_t* srcEnd = srcCur + srcSize;

  // Destination Buffer.
  size_t initSize = dst.getLength();
  size_t growSize = srcSize + 1;

  err_t err = dst.grow(growSize);
  if (FOG_IS_ERROR(err)) return err;

  Char* dstCur = dst.getDataX() + initSize;

  // Encode Table.
  const TextCodecPage8::Encode* table = d->page8->encode;

  for (;;)
  {
    uint16_t uc = *srcCur++;
    if (uc >= 128) uc = table->uc[uc - 128];

    *dstCur++ = uc;
    if (srcCur == srcEnd) break;
  }

  dst.finishDataX(dstCur);
  return ERR_OK;
}

static void FOG_CDECL TextCodec_8Bit_destroy(TextCodecData* d)
{
  TextCodecPage8* page = const_cast<TextCodecPage8*>(d->page8);

  if (page)
  {
    TextCodecPage8::Decode* const* tables = page->decode;

    for (uint i = 0; i != 256; i++)
    {
      if (tables[i] != &TextCodec_8Bit_empty)
        Memory::free(tables[i]);
    }

    Memory::free(page);
  }

  Memory::free(d);
}

static TextCodecData* TextCodec_8Bit_create(const TextCodecItem& item)
{
  TextCodecData* d = reinterpret_cast<TextCodecData*>(Memory::alloc(sizeof(TextCodecData)));
  if (FOG_IS_NULL(d)) return d;

  d->refCount.init(1);
  d->destroy = TextCodec_8Bit_destroy;

  if (item.code == TEXT_ENCODING_ISO8859_1)
  {
    // Optimized ISO-8859-1 (Latin1) encode/decode.
    d->encode = TextCodec_ISO8859_1_encode;
    d->decode = TextCodec_ISO8859_1_decode;
  }
  else
  {
    // Generic table-based encode/decode.
    d->encode = TextCodec_8Bit_encode;
    d->decode = TextCodec_8Bit_decode;
  }

  d->code = item.code;
  d->flags = item.flags;
  d->mime = item.mime;

  uint i;
  const uint16_t* encode = (const uint16_t*)item.table;

  TextCodecPage8* page8 = reinterpret_cast<TextCodecPage8 *>(Memory::alloc(sizeof(TextCodecPage8)));
  if (FOG_IS_NULL(page8)) goto _Fail;

  page8->encode = (TextCodecPage8::Encode*)encode;

  for (i = 0; i < 256; i++)
  {
    page8->decode[i] = (TextCodecPage8::Decode*)&TextCodec_8Bit_empty;
  }

  for (i = 0; i < 256; i++)
  {
    uint32_t uc = (i < 128) ? (uint32_t)i : encode[i-128];
    uint32_t ucPage = uc >> 8;
    uint32_t ucIndex = uc & 0xFF;

    TextCodecPage8::Decode* decode = (TextCodecPage8::Decode*)page8->decode[ucPage];

    if (decode == &TextCodec_8Bit_empty)
    {
      decode = reinterpret_cast<TextCodecPage8::Decode *>(
        Memory::calloc(sizeof(TextCodecPage8::Decode)));
      if (FOG_IS_NULL(decode)) goto _Fail;

      page8->decode[ucPage] = decode;
    }

    decode->uc[ucIndex] = (uint8_t)i;
  }

  d->page8 = page8;
  return d;

_Fail:
  // Silently destroy the created data and return NULL.
  if (page8 != NULL)
  {
    for (i = 0; i < 256; i++)
    {
      TextCodecPage8::Decode* decode = page8->decode[i];
      if (decode != &TextCodec_8Bit_empty)
        Memory::free(decode);
    }
    Memory::free(page8);
  }

  Memory::free(d);
  return NULL;
}

// ============================================================================
// [Fog::TextCodec - UTF-8]
// ============================================================================

static err_t FOG_CDECL TextCodec_UTF8_encode(const TextCodecData* d,
  ByteArray& dst, const Char* src, size_t srcLength, TextCodecState* state,
  TextCodecHandler* handler)
{
  _FOG_TEXTCODEC_ENCODE_INIT_VARS(srcLength * 2 + 4)
  size_t remain = dst.getCapacity() - initSize;
  _FOG_TEXTCODEC_ENCODE_INIT_STATE()

  while (srcCur != srcEnd)
  {
    uc = *srcCur++;
    if (FOG_UNLIKELY(Char::isHiSurrogate(uc)))
    {
      _FOG_TEXTCODEC_ENCODE_DO_SURROGATE(true)
    }

    // Get whether the destination buffer is large enough.
    if (remain < 4)
    {
      dst.finishDataX(reinterpret_cast<char*>(dstCur));
      initSize = dst.getLength();

      // If we failed to predict the ideal destination size, then assume
      // the worst, preventing another reallocation.
      err = dst.reserve(initSize + (size_t)(srcEnd - srcCur) * 4);
      if (FOG_IS_ERROR(err)) return err;

      dstCur = reinterpret_cast<uint8_t*>(dst.getDataX()) + initSize;
      remain = dst.getCapacity() - initSize;
    }

    if (uc < 0x80)
    {
      dstCur[0] = (uint8_t)uc;

      dstCur++;
      remain--;
    }
    else if (uc < 0x800)
    {
      dstCur[1] = (uint8_t)(0x80 | (uc & 0x3f)); uc = (uc >> 6) | 0xC0;
      dstCur[0] = (uint8_t)(uc);

      dstCur += 2;
      remain -= 2;
    }
    else if (uc < 0x10000)
    {
      dstCur[2] = (uint8_t)(0x80 | (uc & 0x3f)); uc = (uc >> 6) | 0x800;
      dstCur[1] = (uint8_t)(0x80 | (uc & 0x3f)); uc = (uc >> 6) | 0xC0;
      dstCur[0] = (uint8_t)(uc);

      dstCur += 3;
      remain -= 3;
    }
    else // (uc < 0x200000)
    {
      dstCur[3] = (uint8_t)(0x80 | (uc & 0x3f)); uc = (uc >> 6) | 0x10000;
      dstCur[2] = (uint8_t)(0x80 | (uc & 0x3f)); uc = (uc >> 6) | 0x800;
      dstCur[1] = (uint8_t)(0x80 | (uc & 0x3f)); uc = (uc >> 6) | 0xC0;
      dstCur[0] = (uint8_t)(uc);

      dstCur += 4;
      remain -= 4;
    }
  }

_End:
  dst.finishDataX(reinterpret_cast<char*>(dstCur));
  return err;
}

static err_t FOG_CDECL TextCodec_UTF8_decode(const TextCodecData* d,
  String& dst, const void* src, size_t srcSize, TextCodecState* state)
{
#define GET_UTF8_CHAR(_Buffer_) \
  switch (utf8Size) \
  { \
    /* Invalid UTF-8 Sequence */ \
    case 0: \
      err = ERR_STRING_INVALID_UTF8; \
      goto _End; \
    case 1: \
      break; \
    case 2: \
      uc = ((uc - 192U) << 6U) \
         | (uint32_t((_Buffer_)[1]) - 128U); \
      break; \
    case 3: \
      uc = ((uc - 224U) << 12U) \
         | ((uint32_t((_Buffer_)[1]) - 128U) << 6) \
         |  (uint32_t((_Buffer_)[2]) - 128U); \
      \
      /* Remove the UTF8-BOM - [EF][BB][BF] == \uFEFF. */ \
      if (FOG_UNLIKELY(uc == 0xFEFF)) goto _Continue; \
      \
      break; \
    case 4: \
      uc = ((uc - 240U) << 24U) \
         | ((uint32_t((_Buffer_)[1]) - 128U) << 12) \
         | ((uint32_t((_Buffer_)[2]) - 128U) << 6) \
         |  (uint32_t((_Buffer_)[3]) - 128U); \
      break; \
    default: \
      err = ERR_STRING_INVALID_UTF8; \
      goto _End; \
  }

  // Length initialization and check.
  if (srcSize == DETECT_LENGTH) srcSize = StringUtil::len(reinterpret_cast<const char*>(src));
  if (srcSize == 0) return ERR_OK;

  // Source Buffer.
  const uint8_t* srcCur = reinterpret_cast<const uint8_t*>(src);
  const uint8_t* srcEnd = srcCur + srcSize;

  // Destination Buffer.
  size_t oldStateSize = 0;
  size_t utf8Size;

  size_t initSize = dst.getLength();
  size_t growSize = srcSize + 1;

  err_t err = dst.reserve(growSize);
  if (FOG_IS_ERROR(err)) return err;

  Char* dstCur = dst.getDataX() + initSize;

  // Characters
  uint32_t uc;

  if (state && (oldStateSize = state->getBufferLength()))
  {
    const uint8_t* bufPtr = reinterpret_cast<uint8_t*>(state->_buffer);
    size_t bufSize = TextCodec_addToState(state, srcCur, srcEnd);

    uc = *bufPtr;
    utf8Size = Unicode::utf8GetSize(uc);

    // Incomplete Input, we are returning ERR_OK, because we know
    // that the state isn't NULL pointer. In all other cases TextCodec
    // should return ERR_STRING_TRUNCATED.
    if (FOG_UNLIKELY(bufSize < utf8Size)) return ERR_OK;

    srcCur -= oldStateSize;
    state->_bufferLength = 0;

    GET_UTF8_CHAR(bufPtr);
    goto _Code;
  }

_Loop:
  for (;;)
  {
    uc = *srcCur;
    utf8Size = Unicode::utf8GetSize(uc);

    // Incomplete Input
    if (FOG_UNLIKELY((size_t)(srcEnd - srcCur) < utf8Size)) goto _InputTruncated;

    GET_UTF8_CHAR(srcCur);

_Code:
    if (uc >= 0x10000U && uc <= UNICODE_MAX)
    {
      Char::ucs4ToSurrogate(&dstCur[0], &dstCur[1], uc);
      dstCur += 2;
    }
    else if (Char::isSurrogate(uc) && uc >= 0xFFFE)
    {
      err = ERR_STRING_INVALID_CHAR;
      goto _End;
    }
    else
    {
      *dstCur++ = (uint16_t)uc;
    }

_Continue:
    srcCur += utf8Size;
    if (srcCur == srcEnd) break;
  }
  goto _End;

_InputTruncated:
  // Different behavior if state is set or not.
  if (state)
  {
    size_t bufSize = (size_t)(srcEnd - srcCur);
    memcpy(state->_buffer, srcCur, bufSize);
    state->_bufferLength = (uint32_t)bufSize;
  }
  else
  {
    err = ERR_STRING_TRUNCATED;
  }

_End:
  dst.finishDataX(dstCur);
  return err;

#undef GET_UTF8_CHAR
}

static void FOG_CDECL TextCodec_UTF8_destroy(TextCodecData* d)
{
  Memory::free(d);
}

static TextCodecData* TextCodec_UTF8_create(const TextCodecItem& item)
{
  TextCodecData* d = reinterpret_cast<TextCodecData*>(Memory::alloc(sizeof(TextCodecData)));
  if (FOG_IS_NULL(d)) return d;

  d->refCount.init(1);
  d->destroy = TextCodec_UTF8_destroy;
  d->encode = TextCodec_UTF8_encode;
  d->decode = TextCodec_UTF8_decode;

  d->code = item.code;
  d->flags = item.flags;
  d->mime = item.mime;
  d->page8 = NULL;

  return d;
}

// ============================================================================
// [Fog::TextCodec - UTF-16]
// ============================================================================

static err_t FOG_CDECL TextCodec_UTF16_encode_native(const TextCodecData* d,
  ByteArray& dst, const Char* src, size_t srcLength, TextCodecState* state,
  TextCodecHandler* handler)
{
  _FOG_TEXTCODEC_ENCODE_INIT_VARS(srcLength * 2 + 2)
  _FOG_TEXTCODEC_ENCODE_INIT_STATE()

  while (srcCur != srcEnd)
  {
    uc = *srcCur++;
    if (FOG_UNLIKELY(Char::isHiSurrogate(uc)))
    {
      _FOG_TEXTCODEC_ENCODE_DO_SURROGATE(false)

      Memory::write_2a(dstCur + 0, (uint16_t)uc);
      Memory::write_2a(dstCur + 2, (uint16_t)ucSurrogate);
      dstCur += 4;
    }
    else
    {
      Memory::write_2a(dstCur + 0, (uint16_t)uc);
      dstCur += 2;
    }
  }

_End:
  dst.finishDataX(reinterpret_cast<char*>(dstCur));
  return err;
}

static err_t FOG_CDECL TextCodec_UTF16_encode_swapped(const TextCodecData* d,
  ByteArray& dst, const Char* src, size_t srcLength, TextCodecState* state,
  TextCodecHandler* handler)
{
  _FOG_TEXTCODEC_ENCODE_INIT_VARS(srcLength * 2 + 2)
  _FOG_TEXTCODEC_ENCODE_INIT_STATE()

  while (srcCur != srcEnd)
  {
    uc = *srcCur++;
    if (FOG_UNLIKELY(Char::isHiSurrogate(uc)))
    {
      _FOG_TEXTCODEC_ENCODE_DO_SURROGATE(false)

      Memory::write_2a(dstCur + 0, Memory::bswap16((uint16_t)uc));
      Memory::write_2a(dstCur + 2, Memory::bswap16((uint16_t)ucSurrogate));
      dstCur += 4;
    }
    else
    {
      Memory::write_2a(dstCur + 0, Memory::bswap16((uint16_t)uc));
      dstCur += 2;
    }
  }

_End:
  dst.finishDataX(reinterpret_cast<char*>(dstCur));
  return err;
}

static err_t FOG_CDECL TextCodec_UTF16_decode(const TextCodecData* d,
  String& dst, const void* src, size_t srcSize, TextCodecState* state)
{
  // Length initialization and check.
  if (srcSize == DETECT_LENGTH) srcSize = StringUtil::len(reinterpret_cast<const Char*>(src)) << 1;
  if (srcSize == 0) return ERR_OK;

  // Source Buffer.
  const uint8_t* srcCur = reinterpret_cast<const uint8_t*>(src);
  const uint8_t* srcEnd = srcCur + srcSize;
  const uint8_t* srcEndM2 = srcCur + srcSize - 2;

  // Destination Buffer.
  size_t oldStateSize = 0;

  size_t initSize = dst.getLength();
  size_t growSize = (srcSize >> 1) + 2;

  err_t err = dst.reserve(growSize);
  if (FOG_IS_ERROR(err)) return err;

  Char* dstCur = dst.getDataX() + initSize;

  // Characters.
  Char uc0;
  Char uc1;

  // Byte swapping.
  uint8_t isByteSwapped = TextCodec_isByteSwappedOnInit(d);

  if (state)
  {
    if (state->hasBom()) isByteSwapped = state->isByteSwapped();

    if (oldStateSize = state->getBufferLength())
    {
      const uint8_t* bufPtr = reinterpret_cast<uint8_t*>(state->_buffer);
      size_t bufSize = TextCodec_addToState(state, srcCur, srcEnd);

      if (state->getBufferLength() < 2) return ERR_OK;

      uc0 = reinterpret_cast<const uint16_t*>(bufPtr)[0];
      if (isByteSwapped) uc0.bswap();

      if (uc0.isHiSurrogate())
      {
        if (state->getBufferLength() < 4) return ERR_OK;

        uc1 = reinterpret_cast<const uint16_t*>(bufPtr)[1];
        if (isByteSwapped) uc1.bswap();

        srcCur -= oldStateSize;
        state->_bufferLength = 0;

        goto _SurrogatePair;
      }
      else
      {
        srcCur -= oldStateSize;
        state->_bufferLength = 0;

        goto _NotSurrogatePair;
      }
    }
  }

_Loop:
  for (;;)
  {
    if (srcCur > srcEndM2) break;

    uc0 = reinterpret_cast<const uint16_t*>(srcCur)[0];
    if (isByteSwapped) uc0.bswap();

    if (uc0.isHiSurrogate())
    {
      if (srcCur + 2 > srcEndM2) break;

      uc1 = reinterpret_cast<const uint16_t*>(srcCur)[1];
      if (isByteSwapped) uc1.bswap();

_SurrogatePair:
      if (!uc1.isLoSurrogate())
      {
        err = ERR_STRING_INVALID_UTF16;
        goto _End;
      }

      dstCur[0] = uc0;
      dstCur[1] = uc1;
      dstCur += 2;
      srcCur += 4;
    }
    else
    {
_NotSurrogatePair:
      // BOM support.
      if (uc0.isBomSwap())
      {
        isByteSwapped = !isByteSwapped;
      }
      else if (uc0.isLoSurrogate())
      {
        err = ERR_STRING_INVALID_UTF16;
        goto _End;
      }
      else
      {
        *dstCur++ = uc0;
      }

      srcCur += 2;
    }
  }

  // We need to check if the input was truncated or not. Ideally srcCur should
  // be same as srcEnd.
  if (srcCur != srcEnd)
  {
    // Different behavior if state is set or not.
    if (state)
    {
      size_t bufSize = (size_t)(srcEnd - srcCur);
      memcpy(state->_buffer, srcCur, bufSize);
      state->_bufferLength = (uint32_t)bufSize;
    }
    else
    {
      err = ERR_STRING_TRUNCATED;
    }
  }

_End:
  if (state) state->setByteSwapped(isByteSwapped);

  dst.finishDataX(dstCur);
  return err;
}

static void FOG_CDECL TextCodec_UTF16_destroy(TextCodecData* d)
{
  Memory::free(d);
}

static TextCodecData* TextCodec_UTF16_create(const TextCodecItem& item)
{
  TextCodecData* d = reinterpret_cast<TextCodecData*>(Memory::alloc(sizeof(TextCodecData)));
  if (FOG_IS_NULL(d)) return d;

  d->refCount.init(1);
  d->destroy = TextCodec_UTF16_destroy;
  d->encode = _FOG_TEXTCODEC_IS_BYTESWAPPED(item.flags)
    ? TextCodec_UTF16_encode_swapped
    : TextCodec_UTF16_encode_native;
  d->decode = TextCodec_UTF16_decode;

  d->code = item.code;
  d->flags = item.flags;
  d->mime = item.mime;
  d->page8 = NULL;

  return d;
}

// ============================================================================
// [Fog::TextCodec - UCS-2]
// ============================================================================

static err_t FOG_CDECL TextCodec_UCS2_encode_native(const TextCodecData* d,
  ByteArray& dst, const Char* src, size_t srcLength, TextCodecState* state,
  TextCodecHandler* handler)
{
  _FOG_TEXTCODEC_ENCODE_INIT_VARS(srcLength * 2 + 2)
  ByteArrayTmp<32> buf;
  _FOG_TEXTCODEC_ENCODE_INIT_STATE()

  while (srcCur != srcEnd)
  {
    uc = *srcCur++;
    if (FOG_UNLIKELY(Char::isHiSurrogate(uc)))
    {
      _FOG_TEXTCODEC_ENCODE_DO_SURROGATE(true)

      buf.clear();
      err = handler->replaceCharacter(buf, uc);
      if (FOG_IS_ERROR(err)) goto _End;

      // Need to finalize dst, append and reserve for next appending.
      dst.finishDataX(reinterpret_cast<char*>(dstCur));

      {
        size_t dl = dst.getLength();
        size_t bl = buf.getLength();

        err = dst.reserve(dl + bl + 128 + (size_t)(srcEnd - srcCur) * 2);
        if (FOG_IS_ERROR(err)) goto _End;

        dstCur = reinterpret_cast<uint8_t*>(dst.getDataX() + dl);

        for (size_t i = 0; i < bl; i++)
        {
          Memory::write_2a(dstCur, (uint16_t)buf.getAt(i));
          dstCur += 2;
        }
      }
    }
    else
    {
      Memory::write_2a(dstCur, (uint16_t)uc);
      dstCur += 2;
    }
  }

_End:
  dst.finishDataX(reinterpret_cast<char*>(dstCur));
  return err;
}

static err_t FOG_CDECL TextCodec_UCS2_encode_swapped(const TextCodecData* d,
  ByteArray& dst, const Char* src, size_t srcLength, TextCodecState* state,
  TextCodecHandler* handler)
{
  _FOG_TEXTCODEC_ENCODE_INIT_VARS(srcLength * 2 + 2)
  ByteArrayTmp<32> buf;
  _FOG_TEXTCODEC_ENCODE_INIT_STATE()

  while (srcCur != srcEnd)
  {
    uc = *srcCur++;
    if (FOG_UNLIKELY(Char::isHiSurrogate(uc)))
    {
      _FOG_TEXTCODEC_ENCODE_DO_SURROGATE(true)

      // Need to finalize dst, append and reserve for next appending.
      dst.finishDataX(reinterpret_cast<char*>(dstCur));

      buf.clear();
      err = handler->replaceCharacter(buf, uc);
      if (FOG_IS_ERROR(err)) return err;

      {
        size_t dl = dst.getLength();
        size_t bl = buf.getLength();

        err = dst.reserve(dl + bl + 128 + (size_t)(srcEnd - srcCur) * 2);
        if (FOG_IS_ERROR(err)) goto _End;

        dstCur = reinterpret_cast<uint8_t*>(dst.getDataX() + dl);

        for (size_t i = 0; i < bl; i++)
        {
          Memory::write_2a(dstCur, (uint16_t)buf.getAt(i) << 8);
          dstCur += 2;
        }
      }
    }
    else
    {
      Memory::write_2a(dstCur, Memory::bswap16((uint16_t)uc));
      dstCur += 2;
    }
  }

_End:
  dst.finishDataX(reinterpret_cast<char*>(dstCur));
  return err;
}

static err_t FOG_CDECL TextCodec_UCS2_decode(const TextCodecData* d,
  String& dst, const void* src, size_t srcSize, TextCodecState* state)
{
  // Length initialization and check.
  if (srcSize == DETECT_LENGTH) srcSize = StringUtil::len(reinterpret_cast<const Char*>(src)) << 1;
  if (srcSize == 0) return ERR_OK;

  // Source Buffer.
  const uint8_t* srcCur = reinterpret_cast<const uint8_t*>(src);
  const uint8_t* srcEnd = srcCur + srcSize;
  const uint8_t* srcEndM2 = srcCur + srcSize - 2;

  // Destination Buffer.
  size_t oldStateSize = 0;
  size_t growSize = (srcSize >> 1) + 2;

  err_t err = dst.reserve(dst.getLength() + growSize);
  if (FOG_IS_ERROR(err)) return err;

  Char* dstCur = dst.getDataX() + dst.getLength();

  // Characters.
  Char uc0;

  // Byte Swapping.
  uint8_t isByteSwapped = TextCodec_isByteSwappedOnInit(d);

  if (state)
  {
    if (state->hasBom()) isByteSwapped = state->isByteSwapped();

    if ((oldStateSize = state->getBufferLength()))
    {
      const uint8_t* bufPtr = reinterpret_cast<uint8_t*>(state->_buffer);
      size_t bufSize = TextCodec_addToState(state, srcCur, srcEnd);

      if (state->getBufferLength() < 2) return ERR_OK;

      uc0 = reinterpret_cast<const uint16_t*>(bufPtr)[0];
      if (isByteSwapped) uc0.bswap();

      srcCur -= oldStateSize;
      state->_bufferLength = 0;

      goto _ProcessChar;
    }
  }

_Loop:
  for (;;)
  {
    if (srcCur > srcEndM2) break;

    uc0 = reinterpret_cast<const uint16_t*>(srcCur)[0];
    if (isByteSwapped) uc0.bswap();

_ProcessChar:
    if (uc0.isSurrogate())
    {
      err = ERR_STRING_INVALID_UCS2;
      goto _End;
    }

    // BOM support.
    if (uc0.isBomSwap())
    {
      isByteSwapped = !isByteSwapped;
    }
    else
    {
      *dstCur++ = uc0;
    }

    srcCur += 2;
  }

  // We need to check if input was truncated or not. Ideally srcCur should
  // be srcEnd.
  if (srcCur != srcEnd)
  {
    // Different behavior if state is set or not.
    if (state)
    {
      size_t bufSize = (size_t)(srcEnd - srcCur);
      memcpy(state->_buffer, srcCur, bufSize);
      state->_bufferLength = (uint32_t)bufSize;
    }
    else
    {
      err = ERR_STRING_TRUNCATED;
    }
  }

_End:
  if (state) state->setByteSwapped(isByteSwapped);

  dst.finishDataX(dstCur);
  return err;
}

static void FOG_CDECL TextCodec_UCS2_destroy(TextCodecData* d)
{
  Memory::free(d);
}

static TextCodecData* TextCodec_UCS2_create(const TextCodecItem& item)
{
  TextCodecData* d = reinterpret_cast<TextCodecData*>(Memory::alloc(sizeof(TextCodecData)));
  if (FOG_IS_NULL(d)) return d;

  d->refCount.init(1);
  d->destroy = TextCodec_UCS2_destroy;
  d->encode = _FOG_TEXTCODEC_IS_BYTESWAPPED(item.flags)
    ? TextCodec_UCS2_encode_swapped
    : TextCodec_UCS2_encode_native;
  d->decode = TextCodec_UCS2_decode;

  d->code = item.code;
  d->flags = item.flags;
  d->mime = item.mime;
  d->page8 = NULL;

  return d;
}

// ============================================================================
// [Fog::TextCodec - UTF-32]
// ============================================================================

static err_t FOG_CDECL TextCodec_UTF32_encode_native(const TextCodecData* d,
  ByteArray& dst, const Char* src, size_t srcLength, TextCodecState* state,
  TextCodecHandler* handler)
{
  _FOG_TEXTCODEC_ENCODE_INIT_VARS(srcLength * 4 + 4)
  _FOG_TEXTCODEC_ENCODE_INIT_STATE()

  while (srcCur != srcEnd)
  {
    uc = *srcCur++;
    if (FOG_UNLIKELY(Char::isHiSurrogate(uc)))
    {
      _FOG_TEXTCODEC_ENCODE_DO_SURROGATE(true)
    }

    Memory::write_4a(dstCur, uc);
    dstCur += 4;
  }

_End:
  dst.finishDataX(reinterpret_cast<char*>(dstCur));
  return err;
}

static err_t FOG_CDECL TextCodec_UTF32_encode_swapped(const TextCodecData* d,
  ByteArray& dst, const Char* src, size_t srcLength, TextCodecState* state,
  TextCodecHandler* handler)
{
  _FOG_TEXTCODEC_ENCODE_INIT_VARS(srcLength * 4 + 4)
  _FOG_TEXTCODEC_ENCODE_INIT_STATE()

  while (srcCur != srcEnd)
  {
    uc = *srcCur++;
    if (FOG_UNLIKELY(Char::isHiSurrogate(uc)))
    {
      _FOG_TEXTCODEC_ENCODE_DO_SURROGATE(true)
    }

    Memory::write_4a(dstCur, Memory::bswap32(uc));
    dstCur += 4;
  }

_End:
  dst.finishDataX(reinterpret_cast<char*>(dstCur));
  return err;
}

static err_t FOG_CDECL TextCodec_UTF32_decode(const TextCodecData* d,
  String& dst, const void* src, size_t srcSize, TextCodecState* state)
{
  // Length initialization and check.
  if (srcSize == DETECT_LENGTH) srcSize = StringUtil::len(reinterpret_cast<const uint32_t*>(src)) << 3;
  if (srcSize == 0) return ERR_OK;

  // Source Buffer.
  const uint8_t* srcCur = reinterpret_cast<const uint8_t*>(src);
  const uint8_t* srcEnd = srcCur + srcSize;
  const uint8_t* srcEndM4 = srcCur + srcSize - 4;

  // Destination Buffer.
  size_t oldStateSize = 0;

  size_t initSize = dst.getLength();
  size_t growSize = (srcSize >> 1) + 2;

  err_t err = dst.grow(growSize);
  if (FOG_IS_ERROR(err)) return err;

  Char* dstCur = dst.getDataX() + initSize;

  // Characters.
  uint32_t uc;

  // Byte Swapping.
  uint8_t isByteSwapped = TextCodec_isByteSwappedOnInit(d);

  if (state)
  {
    if (state->hasBom()) isByteSwapped = state->isByteSwapped();

    if ((oldStateSize = state->getBufferLength()))
    {
      const uint8_t* bufPtr = reinterpret_cast<uint8_t*>(state->_buffer);
      size_t bufSize = TextCodec_addToState(state, srcCur, srcEnd);

      if (state->getBufferLength() < 4) return ERR_OK;

      uc = reinterpret_cast<const uint32_t*>(bufPtr)[0];
      if (isByteSwapped) uc = Memory::bswap32(uc);

      srcCur -= oldStateSize;
      state->_bufferLength = 0;

      goto _ProcessChar;
    }
  }

_Loop:
  for (;;)
  {
    if (srcCur > srcEndM4) break;

    uc = reinterpret_cast<const uint32_t*>(srcCur)[0];
    if (isByteSwapped) uc = Memory::bswap32(uc);

_ProcessChar:
    if (uc > UNICODE_MAX || (uc <= 0xFFFF && Char::isSurrogate((uint16_t)uc)))
    {
      err = ERR_STRING_INVALID_CHAR;
      goto _End;
    }

    // BOM support.
    if (uc == UTF32_BOM_SWAPPED)
    {
      isByteSwapped = !isByteSwapped;
    }
    else if (uc >= 0x10000)
    {
      Char::ucs4ToSurrogate(&dstCur[0], &dstCur[1], uc);
      dstCur += 2;
    }
    else
    {
      *dstCur++ = uc;
    }

    srcCur += 4;
  }

  // We need to check if input was truncated or not. Ideally srcCur should
  // be srcEnd.
  if (srcCur != srcEnd)
  {
    // Different behavior if state is set or not.
    if (state)
    {
      size_t bufSize = (size_t)(srcEnd - srcCur);
      memcpy(state->_buffer, srcCur, bufSize);
      state->_bufferLength = (uint32_t)bufSize;
    }
    else
    {
      err = ERR_STRING_TRUNCATED;
    }
  }

_End:
  if (state) state->setByteSwapped(isByteSwapped);

  dst.finishDataX(dstCur);
  return err;
}

static void FOG_CDECL TextCodec_UTF32_destroy(TextCodecData* d)
{
  Memory::free(d);
}

static TextCodecData* TextCodec_UTF32_create(const TextCodecItem& item)
{
  TextCodecData* d = reinterpret_cast<TextCodecData*>(Memory::alloc(sizeof(TextCodecData)));
  if (FOG_IS_NULL(d)) return d;

  d->refCount.init(1);
  d->destroy = TextCodec_UTF32_destroy;
  d->encode = _FOG_TEXTCODEC_IS_BYTESWAPPED(item.flags)
    ? TextCodec_UTF32_encode_swapped
    : TextCodec_UTF32_encode_native;
  d->decode = TextCodec_UTF32_decode;

  d->code = item.code;
  d->flags = item.flags;
  d->mime = item.mime;
  d->page8 = NULL;

  return d;
}

// ============================================================================
// [Fog::TextCodec - BuiltIn]
// ============================================================================

#define _INIT_NONE(_Code_, _Mime_) \
  { \
    TEXT_ENCODING_##_Code_, \
    NO_FLAGS, \
    TextCodec_None_create, \
    _Mime_, \
    NULL \
  }

#define _INIT_8BIT(_Code_, _Mime_) \
  { \
    TEXT_ENCODING_##_Code_, \
    TEXT_ENCODING_IS_TABLE | TEXT_ENCODING_IS_8BIT, \
    TextCodec_8Bit_create, \
    _Mime_, \
    TextCodec_Table_##_Code_ \
  }

#define _INIT_UTF8(_Code_, _Mime_) \
  { \
    TEXT_ENCODING_##_Code_, \
    TEXT_ENCODING_IS_UNICODE | TEXT_ENCODING_IS_8BIT | TEXT_ENCODING_IS_VARLEN, \
    TextCodec_UTF8_create, \
    _Mime_, \
    NULL \
  }

#define _INIT_UTF16(_Code_, _Mime_, _Flags_) \
  { \
    TEXT_ENCODING_##_Code_, \
    TEXT_ENCODING_IS_UNICODE | TEXT_ENCODING_IS_16BIT | TEXT_ENCODING_IS_VARLEN | _Flags_, \
    TextCodec_UTF16_create, \
    _Mime_, \
    NULL \
  }

#define _INIT_UTF32(_Code_, _Mime_, _Flags_) \
  { \
    TEXT_ENCODING_##_Code_, \
    TEXT_ENCODING_IS_UNICODE | TEXT_ENCODING_IS_32BIT | _Flags_, \
    TextCodec_UTF32_create, \
    _Mime_, \
    NULL \
  }

#define _INIT_UCS2(_Code_, _Mime_, _Flags_) \
  { \
    TEXT_ENCODING_##_Code_, \
    TEXT_ENCODING_IS_UNICODE | TEXT_ENCODING_IS_16BIT | _Flags_, \
    TextCodec_UCS2_create, \
    _Mime_, \
    NULL \
  }

// ${TEXT_ENCODING:BEGIN}
static const TextCodecItem TextCodec_items[] =
{
  _INIT_NONE(NONE       , "NONE\0"),

  // --------------------------------------------------------------------------
  // [Table Codecs]
  // --------------------------------------------------------------------------

  _INIT_8BIT(ISO8859_1  , "ISO-8859-1\0" "8859-1\0" "LATIN1\0" "L1\0" "ISO-IR-100\0" "CP819\0" "IBM819\0" "819\0"),
  _INIT_8BIT(ISO8859_2  , "ISO-8859-2\0" "8859-2\0" "LATIN2\0" "L2\0" "ISO-IR-101\0"),
  _INIT_8BIT(ISO8859_3  , "ISO-8859-3\0" "8859-3\0" "LATIN3\0" "L3\0" "ISO-IR-109\0"),
  _INIT_8BIT(ISO8859_4  , "ISO-8859-4\0" "8859-4\0" "LATIN4\0" "L4\0" "ISO-IR-110\0"),
  _INIT_8BIT(ISO8859_5  , "ISO-8859-5\0" "8859-5\0" "CYRILLIC\0" "ISO-IR-144\0"),
  _INIT_8BIT(ISO8859_6  , "ISO-8859-6\0" "8859-6\0" "ISO-8859-6-I\0" "ECMA-114\0" "ASMO-708\0" "ARABIC\0" "ISO-IR-127\0"),
  _INIT_8BIT(ISO8859_7  , "ISO-8859-7\0" "8859-7\0" "ECMA-118\0" "GREEK\0" "ISO-IR-126\0"),
  _INIT_8BIT(ISO8859_8  , "ISO-8859-8\0" "8859-8\0" "ISO-8859-8-I\0" "ISO-IR-138\0" "HEBREW\0"),
  _INIT_8BIT(ISO8859_9  , "ISO-8859-9\0" "8859-9\0" "ISO-IR-148\0" "LATIN5\0" "L5\0"),
  _INIT_8BIT(ISO8859_10 , "ISO-8859-10\0" "8859-10\0" "ISO-IR-157\0" "LATIN6\0" "L6\0" "ISO-8859-10:1992\0"),
  _INIT_8BIT(ISO8859_11 , "ISO-8859-11\0" "8859-11\0" "TIS-620\0"),
  _INIT_8BIT(ISO8859_13 , "ISO-8859-13\0" "8859-13\0"),
  _INIT_8BIT(ISO8859_14 , "ISO-8859-14\0" "8859-14\0" "ISO-IR-199\0" "LATIN8\0" "L8\0" "ISO-CELTIC\0"),
  _INIT_8BIT(ISO8859_15 , "ISO-8859-15\0" "8859-15\0" "ISO-IR-203\0" "LATIN9\0" "L9\0"),
  _INIT_8BIT(ISO8859_16 , "ISO-8859-16\0" "8859-16\0" "ISO-IR-226\0" "LATIN10\0" "L10\0"),
  _INIT_8BIT(CP850      , "CP850\0" "IBM850\0" "850\0"),
  _INIT_8BIT(CP866      , "CP866\0" "IBM866\0" "866\0"),
  _INIT_8BIT(CP874      , "CP874\0" "IBM874\0" "874\0"),
  _INIT_8BIT(CP1250     , "CP1250\0" "WINDOWS-1250\0" "1250\0"),
  _INIT_8BIT(CP1251     , "CP1251\0" "WINDOWS-1251\0" "1251\0"),
  _INIT_8BIT(CP1252     , "CP1252\0" "WINDOWS-1252\0" "1252\0"),
  _INIT_8BIT(CP1253     , "CP1253\0" "WINDOWS-1253\0" "1253\0"),
  _INIT_8BIT(CP1254     , "CP1254\0" "WINDOWS-1254\0" "1254\0"),
  _INIT_8BIT(CP1255     , "CP1255\0" "WINDOWS-1255\0" "1255\0"),
  _INIT_8BIT(CP1256     , "CP1256\0" "WINDOWS-1256\0" "1256\0"),
  _INIT_8BIT(CP1257     , "CP1257\0" "WINDOWS-1257\0" "1257\0"),
  _INIT_8BIT(CP1258     , "CP1258\0" "WINDOWS-1258\0" "1258\0"),
  _INIT_8BIT(MAC_ROMAN  , "MAC-ROMAN\0" "MACINTOSH\0" "MAC\0"),
  _INIT_8BIT(KOI8R      , "KOI8-R\0"),
  _INIT_8BIT(KOI8U      , "KOI8-U\0" "KOI8-RU\0"),
  _INIT_8BIT(WINSAMI2   , "WINSAMI2\0" "WS2\0"),
  _INIT_8BIT(ROMAN8     , "ROMAN8\0" "HP-ROMAN8\0"),
  _INIT_8BIT(ARMSCII8   , "ARMSCII-8\0"),
  _INIT_8BIT(GEORGIAN_ACADEMY, "GEORGIAN-ACADEMY\0"),
  _INIT_8BIT(GEORGIAN_PS, "GEORGIAN-PS\0"),

  // --------------------------------------------------------------------------
  // [Unicode Codecs]
  // --------------------------------------------------------------------------

  _INIT_UTF8(UTF8       , "UTF-8\0"),

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
  _INIT_UTF16(UTF16_LE  , "UTF-16LE\0" "UTF-16\0", TEXT_ENCODING_IS_LE),
  _INIT_UTF16(UTF16_BE  , "UTF-16BE\0"           , TEXT_ENCODING_IS_BE),
  _INIT_UTF32(UTF32_LE  , "UTF-32LE\0" "UTF-32\0", TEXT_ENCODING_IS_LE),
  _INIT_UTF32(UTF32_BE  , "UTF-32BE\0"           , TEXT_ENCODING_IS_BE),
  _INIT_UCS2 (UCS2_LE   , "UCS-2LE\0" "UCS-2\0"  , TEXT_ENCODING_IS_LE),
  _INIT_UCS2 (UCS2_BE   , "UCS-2BE\0"            , TEXT_ENCODING_IS_BE),
#endif // FOG_LITTLE_ENDIAN

#if FOG_BYTE_ORDER == FOG_BIG_ENDIAN
  _INIT_UTF16(UTF16_LE  , "UTF-16LE\0"           , TEXT_ENCODING_IS_LE),
  _INIT_UTF16(UTF16_BE  , "UTF-16BE\0" "UTF-16\0", TEXT_ENCODING_IS_BE),
  _INIT_UTF32(UTF32_LE  , "UTF-32LE\0"           , TEXT_ENCODING_IS_LE),
  _INIT_UTF32(UTF32_BE  , "UTF-32BE\0" "UTF-32\0", TEXT_ENCODING_IS_BE),
  _INIT_UCS2 (UCS2_LE   , "UCS-2LE\0"            , TEXT_ENCODING_IS_LE),
  _INIT_UCS2 (UCS2_BE   , "UCS-2BE\0" "UCS-2\0"  , TEXT_ENCODING_IS_BE),
#endif // FOG_BIG_ENDIAN
};
// ${TEXT_ENCODING:END}

// ============================================================================
// [Fog::TextCodec - Search]
// ============================================================================

// MIME searching
//
// First string in mime is encoding name, all other strings are aliases
// zero terminator means that string ends, but two zero terminators means
// that aliases list ends.
//
// Mime compare function that ignores case, ' ', '-' and '_'.
static bool TextCodec_mimeCompare(const char* mime, const char* str, size_t length)
{
  const char* strCur;
  const char* strEnd = str + length;

_RepeatAll:
  strCur = str;
  for (;;)
  {
    char ch;
    char mime_ch;

    // Get string character
_RepeatString:
    ch = (strCur == strEnd) ? 0 : (char)Byte::toLower(*strCur++);
    if (ch == '_'|| ch == '-' || ch == ' ') goto _RepeatString;

    // Get mime character
_RepeatMime:
    mime_ch = Byte::toLower(*mime++);
    if (mime_ch == '_'|| mime_ch == '-' || mime_ch == ' ') goto _RepeatMime;

    if (ch != mime_ch)
    {
      // NOT EQUAL - Go to end of mime string
      while (*mime) mime++;

      // Not two zero terminators => Try next alias
      if (*++mime) goto _RepeatAll;

      // Not found in this MIME and ALIASes
      return false;
    }

    if (!ch)
      // EQUAL
      return true;
  }
}

// ============================================================================
// [Fog::TextCodec - GetCodeset]
// ============================================================================

static const char* TextCodec_getCodeset(void)
{
#if defined(FOG_OS_WINDOWS)
  // Windows hasn't nl_langinto(CODESET), but we can use setlocale()
  // that returns "Language_Country.CODEPAGE
  const char* codeset = setlocale(LC_ALL, NULL); // get locale...
  const char* chr = strchr(codeset, '.');
  if (chr)
  {
    // Text codec knows 1250, 1251, ...
    return chr + 1;
  }
  else
  {
    Debug::dbgFunc("Fog::TextCodec", "Fog::TextCodec::getCodeset", "Can't get system codeset.\n");
    return "ISO-8859-1";
  }
#else
  // Must be called after setlocale("") that's called by:
  //   Fog/Core/Tools/Locale.cpp.
  return nl_langinfo(CODESET);
#endif // FOG_OS_WINDOWS
}

// ============================================================================
// [Fog::TextCodec - Cache]
// ============================================================================

static TextCodecData* TextCodec_d[TEXT_ENCODING_COUNT];

static TextCodecData* TextCodec_create(uint32_t code)
{
  if (code >= TEXT_ENCODING_COUNT) code = TEXT_ENCODING_NONE;

  TextCodecData** p = &TextCodec_d[code];
  TextCodecData*  v = AtomicCore<TextCodecData*>::get(p);

  enum { CREATING_NOW = 1 };

  // Cached.
  if ((size_t)v > (size_t)CREATING_NOW) goto _End;

  // Create.
  if (AtomicCore<TextCodecData*>::cmpXchg(p, (TextCodecData*)NULL, (TextCodecData*)CREATING_NOW))
  {
    v = TextCodec_items[code].create(TextCodec_items[code]);
    AtomicCore<TextCodecData*>::set(p, v);
    goto _End;
  }

  // Race.
  // This is very rare situation, but it can happen!
  while ((v = AtomicCore<TextCodecData*>::get(p)) == (TextCodecData*)CREATING_NOW)
  {
    Thread::yield();
  }
  if (FOG_IS_NULL(v)) return NULL;

_End:
  v->refCount.inc();
  return v;
}

// ============================================================================
// [Fog::TextCodec - Construction / Destruction]
// ============================================================================

TextCodec::TextCodec() :
  _d(TextCodec_ref(((TextCodec*)_cache)[TEXT_CODEC_CACHE_NONE]._d))
{
}

TextCodec::TextCodec(const TextCodec& other) :
  _d(TextCodec_ref(other._d))
{
}

TextCodec::~TextCodec()
{
  TextCodec_deref(_d);
}

// ============================================================================
// [Fog::TextCodec - Create]
// ============================================================================

err_t TextCodec::createFromCode(uint32_t code)
{
  if (code >= TEXT_ENCODING_COUNT)
    return ERR_RT_INVALID_ARGUMENT;

  TextCodecData* newd = TextCodec_create(code);
  if (FOG_IS_NULL(newd)) return ERR_RT_OUT_OF_MEMORY;

  TextCodec_deref(atomicPtrXchg(&_d, newd));
  return ERR_OK;
}

err_t TextCodec::createFromMime(const char* mime)
{
  uint32_t i;
  size_t mimeLength = strlen(mime);

  for (i = 0; i != FOG_ARRAY_SIZE(TextCodec_items); i++)
  {
    if (TextCodec_mimeCompare(TextCodec_items[i].mime, mime, mimeLength))
      return createFromCode(i);
  }

  return ERR_STRING_INVALID_CODEC;
}

err_t TextCodec::createFromMime(const String& mime)
{
  if (mime.getLength() > 31) return ERR_STRING_INVALID_CODEC;

  char mime8[32];
  StringUtil::unicodeToLatin1(mime8, mime.getData(), mime.getLength());
  mime8[mime.getLength()] = '\0';

  return createFromMime(mime8);
}

struct TextCodecBom
{
  uint16_t code;
  uint16_t size;
  uint8_t data[4];
};

err_t TextCodec::createFromBom(const void* data, size_t length)
{
  static const TextCodecBom bomData[] =
  {
    { TEXT_ENCODING_UTF8    , 3, { 0xEF, 0xBB, 0xBF       } },
    { TEXT_ENCODING_UTF16_BE, 2, { 0xFE, 0xFF             } },
    { TEXT_ENCODING_UTF16_LE, 2, { 0xFF, 0xFE             } },
    { TEXT_ENCODING_UTF32_BE, 4, { 0x00, 0x00, 0xFE, 0xFF } },
    { TEXT_ENCODING_UTF32_LE, 4, { 0xFF, 0xFE, 0x00, 0x00 } }
  };

  for (size_t i = 0; i != FOG_ARRAY_SIZE(bomData); i++)
  {
    if (length >= bomData[i].size && memcmp(data, bomData[i].data, bomData[i].size) == 0)
      return createFromCode(bomData[i].code);
  }

  return ERR_STRING_INVALID_CODEC;
}

// ============================================================================
// [Fog::TextCodec - Reset]
// ============================================================================

void TextCodec::reset()
{
  TextCodec_deref(
    atomicPtrXchg(&_d, TextCodec_ref(((TextCodec *)_cache)[TEXT_CODEC_CACHE_NONE]._d))
  );
}

// ============================================================================
// [Fog::TextCodec - Encode / Decode]
// ============================================================================

err_t TextCodec::encode(ByteArray& dst, const Utf16& src, TextCodecState* state,
  TextCodecHandler* handler, uint32_t cntOp) const
{
  if (cntOp == CONTAINER_OP_REPLACE) dst.clear();
  return _d->encode(_d, dst, src.getData(), src.getLength(), state, handler);
}

err_t TextCodec::encode(ByteArray& dst, const String& src, TextCodecState* state,
  TextCodecHandler* handler, uint32_t cntOp) const
{
  if (cntOp == CONTAINER_OP_REPLACE) dst.clear();
  return _d->encode(_d, dst, src.getData(), src.getLength(), state, handler);
}

err_t TextCodec::decode(String& dst, const Stub8& src, TextCodecState* state,
  uint32_t cntOp) const
{
  if (cntOp == CONTAINER_OP_REPLACE) dst.clear();
  return _d->decode(_d, dst, src.getData(), src.getLength(), state);
}

err_t TextCodec::decode(String& dst, const ByteArray& src, TextCodecState* state,
  uint32_t cntOp) const
{
  if (cntOp == CONTAINER_OP_REPLACE) dst.clear();
  return _d->decode(_d, dst, src.getData(), src.getLength(), state);
}

// ============================================================================
// [Fog::TextCodec - Operator Overload]
// ============================================================================

TextCodec& TextCodec::operator=(const TextCodec& other)
{
  TextCodec_deref(atomicPtrXchg(&_d, TextCodec_ref(other._d)));
  return *this;
}

// ============================================================================
// [Fog::TextCodec - Statics]
// ============================================================================

uint8_t TextCodec::_cache[sizeof(void*) * TEXT_CODEC_CACHE_COUNT];

// ============================================================================
// [Init / Fini]
// ============================================================================

FOG_NO_EXPORT void TextCodec_init(void)
{
  // Create the default text-codec handler.
  _TextCodecHandler_default.init();

  // Initialize the 'd' cache.
  Memory::zero(TextCodec_d, sizeof(void*) * TEXT_ENCODING_COUNT);

  // Initialize the cached codecs.
  TextCodec* cache = (TextCodec*)TextCodec::_cache;

  // LOCAL codec is initially initialized to LATIN-1.
  cache[TEXT_CODEC_CACHE_NONE ]._d = TextCodec_create(TEXT_ENCODING_NONE     );
  cache[TEXT_CODEC_CACHE_ASCII]._d = TextCodec_create(TEXT_ENCODING_ISO8859_1);
  cache[TEXT_CODEC_CACHE_LOCAL]._d = TextCodec_create(TEXT_ENCODING_ISO8859_1);
  cache[TEXT_CODEC_CACHE_UTF8 ]._d = TextCodec_create(TEXT_ENCODING_UTF8     );
  cache[TEXT_CODEC_CACHE_UTF16]._d = TextCodec_create(TEXT_ENCODING_UTF16    );
  cache[TEXT_CODEC_CACHE_UTF32]._d = TextCodec_create(TEXT_ENCODING_UTF32    );

  // Initialize the local 8-bit text codec.
  cache[TEXT_CODEC_CACHE_LOCAL].createFromMime(TextCodec_getCodeset());
}

FOG_NO_EXPORT void TextCodec_fini(void)
{
  TextCodec* cache = (TextCodec*)TextCodec::_cache;

  // Destroy the cached codecs.
  cache[TEXT_CODEC_CACHE_NONE ].~TextCodec();
  cache[TEXT_CODEC_CACHE_ASCII].~TextCodec();
  cache[TEXT_CODEC_CACHE_LOCAL].~TextCodec();
  cache[TEXT_CODEC_CACHE_UTF8 ].~TextCodec();
  cache[TEXT_CODEC_CACHE_UTF16].~TextCodec();
  cache[TEXT_CODEC_CACHE_UTF32].~TextCodec();

  // Destroy the 'd' cache.
  for (size_t i = 0; i != FOG_ARRAY_SIZE(TextCodec_d); i++)
  {
    if (TextCodec_d[i]) TextCodec_deref(TextCodec_d[i]);
  }

  // Destroy the default text-codec handler.
  _TextCodecHandler_default.destroy();
}

} // Fog namespace