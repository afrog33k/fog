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
#include <Fog/Core/Memory/MemMgr.h>
#include <Fog/Core/Tools/Char.h>
#include <Fog/Core/Tools/Date.h>
#include <Fog/Core/Tools/Hash.h>
#include <Fog/Core/Tools/List.h>
#include <Fog/Core/Tools/Locale.h>
#include <Fog/Core/Tools/Range.h>
#include <Fog/Core/Tools/RegExp.h>
#include <Fog/Core/Tools/String.h>
#include <Fog/Core/Tools/StringUtil.h>
#include <Fog/Core/Tools/Stub.h>
#include <Fog/Core/Tools/Time.h>
#include <Fog/Core/Tools/Var.h>
#include <Fog/Core/Tools/VarId.h>
#include <Fog/G2d/Geometry/Arc.h>
#include <Fog/G2d/Geometry/Box.h>
#include <Fog/G2d/Geometry/CBezier.h>
#include <Fog/G2d/Geometry/Chord.h>
#include <Fog/G2d/Geometry/Circle.h>
#include <Fog/G2d/Geometry/Ellipse.h>
#include <Fog/G2d/Geometry/Line.h>
#include <Fog/G2d/Geometry/Path.h>
#include <Fog/G2d/Geometry/Pie.h>
#include <Fog/G2d/Geometry/Point.h>
#include <Fog/G2d/Geometry/Polygon.h>
#include <Fog/G2d/Geometry/QBezier.h>
#include <Fog/G2d/Geometry/Rect.h>
#include <Fog/G2d/Geometry/Round.h>
#include <Fog/G2d/Geometry/Size.h>
#include <Fog/G2d/Geometry/Transform.h>
#include <Fog/G2d/Geometry/Triangle.h>
#include <Fog/G2d/Imaging/Image.h>
#include <Fog/G2d/Imaging/ImagePalette.h>
#include <Fog/G2d/Source/Argb.h>
#include <Fog/G2d/Source/Color.h>
#include <Fog/G2d/Source/ColorStop.h>
#include <Fog/G2d/Source/Gradient.h>
#include <Fog/G2d/Source/Texture.h>
#include <Fog/G2d/Tools/Matrix.h>
#include <Fog/G2d/Tools/Region.h>

namespace Fog {

// ============================================================================
// [Fog::Var - Global]
// ============================================================================

static Static<VarData> Var_dNull;
static Static<Var> Var_oNull;

// ============================================================================
// [Fog::Var - DataSize]
// ============================================================================

// ${VAR_TYPE:BEGIN}
static const uint8_t Var_dataSize[] =
{
  /* 0000: VAR_TYPE_NULL                 */ 0,

  /* 0001: VAR_TYPE_BOOL                 */ sizeof(bool),
  /* 0002: VAR_TYPE_CHAR                 */ sizeof(CharW),
  /* 0003: VAR_TYPE_INT32                */ sizeof(int32_t),
  /* 0004: VAR_TYPE_UINT32               */ sizeof(uint32_t),
  /* 0005: VAR_TYPE_INT64                */ sizeof(int64_t),
  /* 0006: VAR_TYPE_UINT64               */ sizeof(uint64_t),
  /* 0007: VAR_TYPE_FLOAT                */ sizeof(float),
  /* 0008: VAR_TYPE_DOUBLE               */ sizeof(double),

  /* 0009: VAR_TYPE_STRINGA              */ 0,
  /* 0010: VAR_TYPE_STRINGW              */ 0,

  /* 0011: VAR_TYPE_LIST_STRINGA         */ 0,
  /* 0012: VAR_TYPE_LIST_STRINGW         */ 0,
  /* 0013: VAR_TYPE_LIST_VAR             */ 0,

  /* 0014: VAR_TYPE_HASH_STRINGA_STRINGA */ 0,
  /* 0015: VAR_TYPE_HASH_STRINGA_VAR     */ 0,
  /* 0016: VAR_TYPE_HASH_STRINGW_STRINGW */ 0,
  /* 0017: VAR_TYPE_HASH_STRINGW_VAR     */ 0,

  /* 0018: VAR_TYPE_REGEXPA              */ 0,
  /* 0019: VAR_TYPE_REGEXPW              */ 0,

  /* 0020: VAR_TYPE_LOCALE               */ 0,

  /* 0021: VAR_TYPE_DATE                 */ sizeof(Date),
  /* 0022: VAR_TYPE_TIME                 */ sizeof(Time),

  /* 0023:                               */ 0,
  /* 0024:                               */ 0,
  /* 0025:                               */ 0,
  /* 0026:                               */ 0,
  /* 0027:                               */ 0,
  /* 0028:                               */ 0,
  /* 0029:                               */ 0,
  /* 0030:                               */ 0,
  /* 0031:                               */ 0,

  /* 0032: VAR_TYPE_POINTI               */ sizeof(PointI),
  /* 0033: VAR_TYPE_POINTF               */ sizeof(PointF),
  /* 0034: VAR_TYPE_POINTD               */ sizeof(PointD),
  /* 0035: VAR_TYPE_SIZEI                */ sizeof(SizeI),
  /* 0036: VAR_TYPE_SIZEF                */ sizeof(SizeF),
  /* 0037: VAR_TYPE_SIZED                */ sizeof(SizeD),
  /* 0038: VAR_TYPE_BOXI                 */ sizeof(BoxI),
  /* 0039: VAR_TYPE_BOXF                 */ sizeof(BoxF),
  /* 0040: VAR_TYPE_BOXD                 */ sizeof(BoxD),
  /* 0041: VAR_TYPE_RECTI                */ sizeof(RectI),
  /* 0042: VAR_TYPE_RECTF                */ sizeof(RectF),
  /* 0043: VAR_TYPE_RECTD                */ sizeof(RectD),
  /* 0044:                               */ 0,
  /* 0045: VAR_TYPE_LINEF                */ sizeof(LineF),
  /* 0046: VAR_TYPE_LINED                */ sizeof(LineD),
  /* 0047: VAR_TYPE_QBEZIERF             */ sizeof(QBezierF),
  /* 0048: VAR_TYPE_QBEZIERD             */ sizeof(QBezierD),
  /* 0049: VAR_TYPE_CBEZIERF             */ sizeof(CBezierF),
  /* 0050: VAR_TYPE_CBEZIERD             */ sizeof(CBezierD),
  /* 0051: VAR_TYPE_TRIANGLEF            */ sizeof(TriangleF),
  /* 0052: VAR_TYPE_TRIANGLED            */ sizeof(TriangleD),
  /* 0053: VAR_TYPE_ROUNDF               */ sizeof(RoundF),
  /* 0054: VAR_TYPE_ROUNDD               */ sizeof(RoundD),
  /* 0055: VAR_TYPE_CIRCLEF              */ sizeof(CircleF),
  /* 0056: VAR_TYPE_CIRCLED              */ sizeof(CircleD),
  /* 0057: VAR_TYPE_ELLIPSEF             */ sizeof(EllipseF),
  /* 0058: VAR_TYPE_ELLIPSED             */ sizeof(EllipseD),
  /* 0059: VAR_TYPE_ARCF                 */ sizeof(ArcF),
  /* 0060: VAR_TYPE_ARCD                 */ sizeof(ArcD),
  /* 0061: VAR_TYPE_CHORDF               */ sizeof(ChordF),
  /* 0062: VAR_TYPE_CHORDD               */ sizeof(ChordD),
  /* 0063: VAR_TYPE_PIEF                 */ sizeof(PieF),
  /* 0064: VAR_TYPE_PIED                 */ sizeof(PieD),
  /* 0065: VAR_TYPE_POLYGONF             */ 0,
  /* 0066: VAR_TYPE_POLYGOND             */ 0,
  /* 0067: VAR_TYPE_PATHF                */ 0,
  /* 0068: VAR_TYPE_PATHD                */ 0,
  /* 0069: VAR_TYPE_REGION               */ 0,
  /* 0070: VAR_TYPE_TRANSFORMF           */ sizeof(TransformF),
  /* 0071: VAR_TYPE_TRANSFORMD           */ sizeof(TransformD),
  /* 0072: VAR_TYPE_                     */ 0,
  /* 0073: VAR_TYPE_                     */ 0,
  /* 0074: VAR_TYPE_                     */ 0,
  /* 0075: VAR_TYPE_                     */ 0,
  /* 0076: VAR_TYPE_                     */ 0,
  /* 0077: VAR_TYPE_                     */ 0,
  /* 0078: VAR_TYPE_                     */ 0,
  /* 0079: VAR_TYPE_                     */ 0
};
// ${VAR_TYPE:END}

static FOG_INLINE size_t Var_getDataSize(uint32_t vType)
{
  FOG_ASSERT(vType < FOG_ARRAY_SIZE(Var_dataSize));
  size_t size = Var_dataSize[vType];

  FOG_ASSERT(size != 0);
  return size;
}

// ============================================================================
// [Fog::Var - Construction / Destruction]
// ============================================================================

static void FOG_CDECL Var_ctor(Var* self)
{
  self->_d = &Var_dNull;
}

static void FOG_CDECL Var_ctorCopy(Var* self, const Var* other)
{
  self->_d = other->_d->addRef();
}

static void FOG_CDECL Var_ctorType(Var* self, uint32_t vType, const void* vData)
{
  switch (vType)
  {
    case VAR_TYPE_NULL:
_CreateNull:
      self->_d = &Var_dNull;
      return;

    case VAR_TYPE_BOOL:
    case VAR_TYPE_CHAR:
    case VAR_TYPE_INT32:
    case VAR_TYPE_UINT32:
    case VAR_TYPE_INT64:
    case VAR_TYPE_UINT64:
    case VAR_TYPE_FLOAT:
    case VAR_TYPE_DOUBLE:
      goto _CreateSimple;

    case VAR_TYPE_STRINGA:
    case VAR_TYPE_STRINGW:
    case VAR_TYPE_LIST_STRINGA:
    case VAR_TYPE_LIST_STRINGW:
    case VAR_TYPE_LIST_VAR:
    case VAR_TYPE_HASH_STRINGA_STRINGA:
    case VAR_TYPE_HASH_STRINGA_VAR:
    case VAR_TYPE_HASH_STRINGW_STRINGW:
    case VAR_TYPE_HASH_STRINGW_VAR:
    case VAR_TYPE_REGEXPA:
    case VAR_TYPE_REGEXPW:
    case VAR_TYPE_LOCALE:
      self->_d = reinterpret_cast<const VarData*>(vData)->addRef();
      return;

    case VAR_TYPE_DATE:
    case VAR_TYPE_TIME:
      goto _CreateSimple;

    case VAR_TYPE_POINTI:
    case VAR_TYPE_POINTF:
    case VAR_TYPE_POINTD:
    case VAR_TYPE_SIZEI:
    case VAR_TYPE_SIZEF:
    case VAR_TYPE_SIZED:
    case VAR_TYPE_BOXI:
    case VAR_TYPE_BOXF:
    case VAR_TYPE_BOXD:
    case VAR_TYPE_RECTI:
    case VAR_TYPE_RECTF:
    case VAR_TYPE_RECTD:
    case VAR_TYPE_LINEF:
    case VAR_TYPE_LINED:
    case VAR_TYPE_QBEZIERF:
    case VAR_TYPE_QBEZIERD:
    case VAR_TYPE_CBEZIERF:
    case VAR_TYPE_CBEZIERD:
    case VAR_TYPE_TRIANGLEF:
    case VAR_TYPE_TRIANGLED:
    case VAR_TYPE_ROUNDF:
    case VAR_TYPE_ROUNDD:
    case VAR_TYPE_CIRCLEF:
    case VAR_TYPE_CIRCLED:
    case VAR_TYPE_ELLIPSEF:
    case VAR_TYPE_ELLIPSED:
    case VAR_TYPE_ARCF:
    case VAR_TYPE_ARCD:
    case VAR_TYPE_CHORDF:
    case VAR_TYPE_CHORDD:
    case VAR_TYPE_PIEF:
    case VAR_TYPE_PIED:
      goto _CreateSimple;

    case VAR_TYPE_POLYGONF:
      // TODO: Var.
      return;

    case VAR_TYPE_POLYGOND:
      // TODO: Var.
      return;

    case VAR_TYPE_PATHF:
    case VAR_TYPE_PATHD:
    case VAR_TYPE_REGION:
      self->_d = reinterpret_cast<const VarData*>(vData)->addRef();
      return;

    case VAR_TYPE_TRANSFORMF:
    case VAR_TYPE_TRANSFORMD:
      goto _CreateSimple;

    case VAR_TYPE_COLOR:
      goto _CreateSimple;

    case VAR_TYPE_TEXTURE:
    case VAR_TYPE_GRADIENTF:
    case VAR_TYPE_GRADIENTD:
    case VAR_TYPE_PATTERN:
      // TODO: Var
      return;

    case VAR_TYPE_COLOR_STOP:
      goto _CreateSimple;

    case VAR_TYPE_IMAGE:
    case VAR_TYPE_IMAGE_PALETTE:
    case VAR_TYPE_FONT:
      // TODO: Var
      return;

    default:
      goto _CreateNull;
  }

_CreateSimple:
  {
    size_t vSize = Var_getDataSize(vType);

    VarData* d = _api.var.dCreate(vSize);
    if (FOG_IS_NULL(d))
      goto _CreateNull;

    d->vType = vType;
    MemOps::copy_small(d->getData(), vData, vSize);

    self->_d = d;
  }
}

static void FOG_CDECL Var_dtor(Var* self)
{
  self->_d->release();
}

// ============================================================================
// [Fog::Var - GetReference]
// ============================================================================

static size_t FOG_CDECL Var_getReference(const Var* self)
{
  VarData* d = self->_d;
  uint32_t vType = d->vType & VAR_TYPE_MASK;

  if (vType == VAR_TYPE_NULL)
    return 1;

  if (vType != VAR_TYPE_OBJECT_REF)
  {
    return AtomicCore<size_t>::get(&d->unknown.reference);
  }
  else
  {
    // TODO: Var
    return 0;
  }
}

// ============================================================================
// [Fog::Var - Type]
// ============================================================================

static uint32_t FOG_CDECL Var_getTypeId(const Var* self)
{
  return self->_d->vType & VAR_TYPE_MASK;
}

// ============================================================================
// [Fog::Var - Reset]
// ============================================================================

static void FOG_CDECL Var_reset(Var* self)
{
  _api.var.dRelease(atomicPtrXchg(&self->_d, &Var_dNull));
}

// ============================================================================
// [Fog::Var - Copy]
// ============================================================================

static void FOG_CDECL Var_copy(Var* self, const Var* other)
{
  atomicPtrXchg(&self->_d, other->_d->addRef())->release();
}

// ============================================================================
// [Fog::Var - Get]
// ============================================================================

static err_t FOG_CDECL Var_getI32(const Var* self, int32_t* dst)
{
  VarData* d = self->_d;
  uint32_t vType = d->vType & VAR_TYPE_MASK;

  switch (vType)
  {
    case VAR_TYPE_BOOL:
    {
      *dst = (int32_t)*reinterpret_cast<const bool*>(d->getData());
      return ERR_OK;
    }

    case VAR_TYPE_CHAR:
    {
      *dst = (int32_t)reinterpret_cast<const CharW*>(d->getData())->getInt();
      return ERR_OK;
    }

    case VAR_TYPE_INT32:
    {
      *dst = *reinterpret_cast<const int32_t*>(d->getData());
      return ERR_OK;
    }

    case VAR_TYPE_UINT32:
    {
      uint32_t val = *reinterpret_cast<const uint32_t*>(d->getData());

      if (val > INT32_MAX)
      {
        *dst = INT32_MAX;
        return ERR_RT_OVERFLOW;
      }

      *dst = (int32_t)val;
      return ERR_OK;
    }

    case VAR_TYPE_INT64:
    {
      int64_t val = *reinterpret_cast<const int64_t*>(d->getData());

      if (val < INT32_MIN)
      {
        *dst = INT32_MIN;
        return ERR_RT_OVERFLOW;
      }

      if (val > INT32_MAX)
      {
        *dst = INT32_MAX;
        return ERR_RT_OVERFLOW;
      }

      *dst = (int32_t)val;
      return ERR_OK;
    }

    case VAR_TYPE_UINT64:
    {
      uint64_t val = *reinterpret_cast<const uint64_t*>(d->getData());

      if (val > INT32_MAX)
      {
        *dst = INT32_MAX;
        return ERR_RT_OVERFLOW;
      }

      *dst = (int32_t)val;
      return ERR_OK;
    }

    case VAR_TYPE_FLOAT:
    {
      float val = *reinterpret_cast<const float*>(d->getData());

      if (!Math::isFinite(val))
        return ERR_RT_NOT_A_NUMBER;

      if (val < (float)INT32_MIN)
      {
        *dst = INT32_MIN;
        return ERR_RT_OVERFLOW;
      }

      if (val > (float)INT32_MAX)
      {
        *dst = INT32_MAX;
        return ERR_RT_OVERFLOW;
      }

      *dst = (int32_t)val;
      return ERR_OK;
    }

    case VAR_TYPE_DOUBLE:
    {
      double val = *reinterpret_cast<const double*>(d->getData());

      if (!Math::isFinite(val))
        return ERR_RT_NOT_A_NUMBER;

      if (val < (double)INT32_MIN)
      {
        *dst = INT32_MIN;
        return ERR_RT_OVERFLOW;
      }

      if (val > (double)INT32_MAX)
      {
        *dst = INT32_MAX;
        return ERR_RT_OVERFLOW;
      }

      *dst = (int32_t)val;
      return ERR_OK;
    }

    case VAR_TYPE_STRINGA:
    {
      const StringA* val = reinterpret_cast<const StringA*>(self);
      return val->parseI32(dst);
    }

    case VAR_TYPE_STRINGW:
    {
      const StringW* val = reinterpret_cast<const StringW*>(self);
      return val->parseI32(dst);
    }

    default:
      return ERR_RT_NOT_COMPATIBLE;
  }
}

static err_t FOG_CDECL Var_getI32Bound(const Var* self, int32_t* dst, int32_t min, int32_t max)
{
  if (min > max)
    return ERR_RT_INVALID_ARGUMENT;

  int32_t value;
  FOG_RETURN_ON_ERROR(_api.var.getI32(self, &value));

  if (value < min || value > max)
  {
    *dst = min;
    return ERR_RT_OVERFLOW;
  }
  else
  {
    *dst = value;
    return ERR_OK;
  }
}

static err_t FOG_CDECL Var_getU32(const Var* self, uint32_t* dst)
{
  VarData* d = self->_d;
  uint32_t vType = d->vType & VAR_TYPE_MASK;

  switch (vType)
  {
    case VAR_TYPE_BOOL:
    {
      *dst = (uint32_t)*reinterpret_cast<const bool*>(d->getData());
      return ERR_OK;
    }

    case VAR_TYPE_CHAR:
    {
      *dst = (uint32_t)reinterpret_cast<const CharW*>(d->getData())->getInt();
      return ERR_OK;
    }

    case VAR_TYPE_INT32:
    {
      int32_t val = *reinterpret_cast<const int32_t*>(d->getData());

      if (val < 0)
      {
        *dst = 0;
        return ERR_RT_OVERFLOW;
      }

      *dst = (uint32_t)val;
      return ERR_OK;
    }

    case VAR_TYPE_UINT32:
    {
      *dst = *reinterpret_cast<const uint32_t*>(d->getData());
      return ERR_OK;
    }

    case VAR_TYPE_INT64:
    {
      int64_t val = *reinterpret_cast<const int64_t*>(d->getData());

      if (val < 0)
      {
        *dst = 0;
        return ERR_RT_OVERFLOW;
      }

      if (val > UINT32_MAX)
      {
        *dst = UINT32_MAX;
        return ERR_RT_OVERFLOW;
      }

      *dst = (uint32_t)(uint64_t)val;
      return ERR_OK;
    }

    case VAR_TYPE_UINT64:
    {
      uint64_t val = *reinterpret_cast<const uint64_t*>(d->getData());

      if (val > UINT32_MAX)
      {
        *dst = UINT32_MAX;
        return ERR_RT_OVERFLOW;
      }

      *dst = (uint32_t)val;
      return ERR_OK;
    }

    case VAR_TYPE_FLOAT:
    {
      float val = *reinterpret_cast<const float*>(d->getData());

      if (!Math::isFinite(val))
        return ERR_RT_NOT_A_NUMBER;

      if (val < 0.0f)
      {
        *dst = 0;
        return ERR_RT_OVERFLOW;
      }

      if (val > (float)UINT32_MAX)
      {
        *dst = UINT32_MAX;
        return ERR_RT_OVERFLOW;
      }

      *dst = (uint32_t)val;
      return ERR_OK;
    }

    case VAR_TYPE_DOUBLE:
    {
      double val = *reinterpret_cast<const double*>(d->getData());

      if (!Math::isFinite(val))
        return ERR_RT_NOT_A_NUMBER;

      if (val < 0.0)
      {
        *dst = 0;
        return ERR_RT_OVERFLOW;
      }

      if (val > (double)UINT32_MAX)
      {
        *dst = UINT32_MAX;
        return ERR_RT_OVERFLOW;
      }

      *dst = (uint32_t)val;
      return ERR_OK;
    }

    case VAR_TYPE_STRINGA:
    {
      const StringA* val = reinterpret_cast<const StringA*>(self);
      return val->parseU32(dst);
    }

    case VAR_TYPE_STRINGW:
    {
      const StringW* val = reinterpret_cast<const StringW*>(self);
      return val->parseU32(dst);
    }

    default:
      return ERR_RT_NOT_COMPATIBLE;
  }
}

static err_t FOG_CDECL Var_getU32Bound(const Var* self, uint32_t* dst, uint32_t min, uint32_t max)
{
  if (min > max)
    return ERR_RT_INVALID_ARGUMENT;

  uint32_t value;
  FOG_RETURN_ON_ERROR(_api.var.getU32(self, &value));

  if (value < min || value > max)
  {
    *dst = min;
    return ERR_RT_OVERFLOW;
  }
  else
  {
    *dst = value;
    return ERR_OK;
  }
}

static err_t FOG_CDECL Var_getI64(const Var* self, int64_t* dst)
{
  VarData* d = self->_d;
  uint32_t vType = d->vType & VAR_TYPE_MASK;

  switch (vType)
  {
    case VAR_TYPE_BOOL:
    {
      *dst = (int64_t)*reinterpret_cast<const bool*>(d->getData());
      return ERR_OK;
    }

    case VAR_TYPE_CHAR:
    {
      *dst = (int64_t)reinterpret_cast<const CharW*>(d->getData())->getInt();
      return ERR_OK;
    }

    case VAR_TYPE_INT32:
    {
      *dst = (int64_t)*reinterpret_cast<const int32_t*>(d->getData());
      return ERR_OK;
    }

    case VAR_TYPE_UINT32:
    {
      *dst = (int64_t)*reinterpret_cast<const uint32_t*>(d->getData());
      return ERR_OK;
    }

    case VAR_TYPE_INT64:
    {
      *dst = *reinterpret_cast<const int64_t*>(d->getData());
      return ERR_OK;
    }

    case VAR_TYPE_UINT64:
    {
      uint64_t val = *reinterpret_cast<const uint64_t*>(d->getData());

      if (val > INT64_MAX)
      {
        *dst = INT64_MAX;
        return ERR_RT_OVERFLOW;
      }

      *dst = (int64_t)val;
      return ERR_OK;
    }

    case VAR_TYPE_FLOAT:
    {
      float val = *reinterpret_cast<const float*>(d->getData());

      if (!Math::isFinite(val))
        return ERR_RT_NOT_A_NUMBER;

      if (val < (float)INT64_MIN)
      {
        *dst = INT64_MIN;
        return ERR_RT_OVERFLOW;
      }

      if (val > (float)INT64_MAX)
      {
        *dst = INT64_MAX;
        return ERR_RT_OVERFLOW;
      }

      *dst = (int64_t)val;
      return ERR_OK;
    }

    case VAR_TYPE_DOUBLE:
    {
      double val = *reinterpret_cast<const double*>(d->getData());

      if (!Math::isFinite(val))
        return ERR_RT_NOT_A_NUMBER;

      if (val < (double)INT64_MIN)
      {
        *dst = INT64_MIN;
        return ERR_RT_OVERFLOW;
      }

      if (val > (double)INT64_MAX)
      {
        *dst = INT64_MAX;
        return ERR_RT_OVERFLOW;
      }

      *dst = (int64_t)val;
      return ERR_OK;
    }

    case VAR_TYPE_STRINGA:
    {
      const StringA* val = reinterpret_cast<const StringA*>(self);
      return val->parseI64(dst);
    }

    case VAR_TYPE_STRINGW:
    {
      const StringW* val = reinterpret_cast<const StringW*>(self);
      return val->parseI64(dst);
    }

    default:
      return ERR_RT_NOT_COMPATIBLE;
  }
}

static err_t FOG_CDECL Var_getI64Bound(const Var* self, int64_t* dst, int64_t min, int64_t max)
{
  if (min > max)
    return ERR_RT_INVALID_ARGUMENT;

  int64_t value;
  FOG_RETURN_ON_ERROR(_api.var.getI64(self, &value));

  if (value < min || value > max)
  {
    *dst = min;
    return ERR_RT_OVERFLOW;
  }
  else
  {
    *dst = value;
    return ERR_OK;
  }
}

static err_t FOG_CDECL Var_getU64(const Var* self, uint64_t* dst)
{
  VarData* d = self->_d;
  uint32_t vType = d->vType & VAR_TYPE_MASK;

  switch (vType)
  {
    case VAR_TYPE_BOOL:
    {
      *dst = (uint64_t)*reinterpret_cast<const bool*>(d->getData());
      return ERR_OK;
    }

    case VAR_TYPE_CHAR:
    {
      *dst = (uint64_t)reinterpret_cast<const CharW*>(d->getData())->getInt();
      return ERR_OK;
    }

    case VAR_TYPE_INT32:
    {
      int32_t val = *reinterpret_cast<const int32_t*>(d->getData());

      if (val < 0)
      {
        *dst = 0;
        return ERR_RT_OVERFLOW;
      }

      *dst = (uint64_t)(uint32_t)val;
      return ERR_OK;
    }

    case VAR_TYPE_UINT32:
    {
      *dst = (uint64_t)*reinterpret_cast<const uint32_t*>(d->getData());
      return ERR_OK;
    }

    case VAR_TYPE_INT64:
    {
      int64_t val = *reinterpret_cast<const int64_t*>(d->getData());

      if (val < 0)
      {
        *dst = 0;
        return ERR_RT_OVERFLOW;
      }

      *dst = (uint64_t)val;
      return ERR_OK;
    }

    case VAR_TYPE_UINT64:
    {
      *dst = *reinterpret_cast<const uint64_t*>(d->getData());
      return ERR_OK;
    }

    case VAR_TYPE_FLOAT:
    {
      float val = *reinterpret_cast<const float*>(d->getData());

      if (!Math::isFinite(val))
        return ERR_RT_NOT_A_NUMBER;

      if (val < 0.0f)
      {
        *dst = 0;
        return ERR_RT_OVERFLOW;
      }

      if (val > (float)UINT64_MAX)
      {
        *dst = UINT64_MAX;
        return ERR_RT_OVERFLOW;
      }

      *dst = (uint64_t)val;
      return ERR_OK;
    }

    case VAR_TYPE_DOUBLE:
    {
      double val = *reinterpret_cast<const double*>(d->getData());

      if (!Math::isFinite(val))
        return ERR_RT_NOT_A_NUMBER;

      if (val < 0.0)
      {
        *dst = 0;
        return ERR_RT_OVERFLOW;
      }

      if (val > (double)UINT64_MAX)
      {
        *dst = UINT64_MAX;
        return ERR_RT_OVERFLOW;
      }

      *dst = (uint64_t)val;
      return ERR_OK;
    }

    case VAR_TYPE_STRINGA:
    {
      const StringA* val = reinterpret_cast<const StringA*>(self);
      return val->parseU64(dst);
    }

    case VAR_TYPE_STRINGW:
    {
      const StringW* val = reinterpret_cast<const StringW*>(self);
      return val->parseU64(dst);
    }

    default:
      return ERR_RT_NOT_COMPATIBLE;
  }
}

static err_t FOG_CDECL Var_getU64Bound(const Var* self, uint64_t* dst, uint64_t min, uint64_t max)
{
  if (min > max)
    return ERR_RT_INVALID_ARGUMENT;

  uint64_t value;
  FOG_RETURN_ON_ERROR(_api.var.getU64(self, &value));

  if (value < min || value > max)
  {
    *dst = min;
    return ERR_RT_OVERFLOW;
  }
  else
  {
    *dst = value;
    return ERR_OK;
  }
}

static err_t FOG_CDECL Var_getFloat(const Var* self, float* dst)
{
  VarData* d = self->_d;
  uint32_t vType = d->vType & VAR_TYPE_MASK;

  switch (vType)
  {
    case VAR_TYPE_BOOL:
    {
      *dst = float((int)*reinterpret_cast<const bool*>(d->getData()));
      return ERR_OK;
    }

    case VAR_TYPE_CHAR:
    {
      *dst = float(reinterpret_cast<const CharW*>(d->getData())->getInt());
      return ERR_OK;
    }

    case VAR_TYPE_INT32:
    {
      *dst = float(*reinterpret_cast<const int32_t*>(d->getData()));
      return ERR_OK;
    }

    case VAR_TYPE_UINT32:
    {
      *dst = float(*reinterpret_cast<const uint32_t*>(d->getData()));
      return ERR_OK;
    }

    case VAR_TYPE_INT64:
    {
      *dst = float(*reinterpret_cast<const int64_t*>(d->getData()));
      return ERR_OK;
    }

    case VAR_TYPE_UINT64:
    {
      *dst = float(*reinterpret_cast<const uint64_t*>(d->getData()));
      return ERR_OK;
    }

    case VAR_TYPE_FLOAT:
    {
      *dst = *reinterpret_cast<const float*>(d->getData());
      return ERR_OK;
    }

    case VAR_TYPE_DOUBLE:
    {
      double val = *reinterpret_cast<const double*>(d->getData());

      if (val < double(FLOAT_MIN))
      {
        *dst = FLOAT_MIN;
        return ERR_RT_OVERFLOW;
      }

      if (val > double(FLOAT_MAX))
      {
        *dst = FLOAT_MAX;
        return ERR_RT_OVERFLOW;
      }

      *dst = float(val);
      return ERR_OK;
    }

    case VAR_TYPE_STRINGA:
    {
      const StringA* val = reinterpret_cast<const StringA*>(self);
      return val->parseReal(dst);
    }

    case VAR_TYPE_STRINGW:
    {
      const StringW* val = reinterpret_cast<const StringW*>(self);
      return val->parseReal(dst);
    }

    default:
      return ERR_RT_NOT_COMPATIBLE;
  }
}

static err_t FOG_CDECL Var_getFloatBound(const Var* self, float* dst, float min, float max)
{
  if (!Math::isFinite(min) || !Math::isFinite(max) || min > max)
    return ERR_RT_INVALID_ARGUMENT;

  float value;
  FOG_RETURN_ON_ERROR(_api.var.getFloat(self, &value));

  if (!Math::isFinite(value) || value < min || value > max)
  {
    *dst = min;
    return ERR_RT_OVERFLOW;
  }
  else
  {
    *dst = value;
    return ERR_OK;
  }
}

static err_t FOG_CDECL Var_getDouble(const Var* self, double* dst)
{
  VarData* d = self->_d;
  uint32_t vType = d->vType & VAR_TYPE_MASK;

  switch (vType)
  {
    case VAR_TYPE_BOOL:
    {
      *dst = double((int)*reinterpret_cast<const bool*>(d->getData()));
      return ERR_OK;
    }

    case VAR_TYPE_CHAR:
    {
      *dst = double(reinterpret_cast<const CharW*>(d->getData())->getInt());
      return ERR_OK;
    }

    case VAR_TYPE_INT32:
    {
      *dst = double(*reinterpret_cast<const int32_t*>(d->getData()));
      return ERR_OK;
    }

    case VAR_TYPE_UINT32:
    {
      *dst = double(*reinterpret_cast<const uint32_t*>(d->getData()));
      return ERR_OK;
    }

    case VAR_TYPE_INT64:
    {
      *dst = double(*reinterpret_cast<const int64_t*>(d->getData()));
      return ERR_OK;
    }

    case VAR_TYPE_UINT64:
    {
      *dst = double(*reinterpret_cast<const uint64_t*>(d->getData()));
      return ERR_OK;
    }

    case VAR_TYPE_FLOAT:
    {
      *dst = double(*reinterpret_cast<const float*>(d->getData()));
      return ERR_OK;
    }

    case VAR_TYPE_DOUBLE:
    {
      *dst = *reinterpret_cast<const double*>(d->getData());
      return ERR_OK;
    }

    case VAR_TYPE_STRINGA:
    {
      const StringA* val = reinterpret_cast<const StringA*>(self);
      return val->parseReal(dst);
    }

    case VAR_TYPE_STRINGW:
    {
      const StringW* val = reinterpret_cast<const StringW*>(self);
      return val->parseReal(dst);
    }

    default:
      return ERR_RT_NOT_COMPATIBLE;
  }
}

static err_t FOG_CDECL Var_getDoubleBound(const Var* self, double* dst, double min, double max)
{
  if (!Math::isFinite(min) || !Math::isFinite(max) || min > max)
    return ERR_RT_INVALID_ARGUMENT;

  double value;
  FOG_RETURN_ON_ERROR(_api.var.getDouble(self, &value));

  if (!Math::isFinite(value) || value < min || value > max)
  {
    *dst = min;
    return ERR_RT_OVERFLOW;
  }
  else
  {
    *dst = value;
    return ERR_OK;
  }
}

static err_t FOG_CDECL Var_getType(const Var* self, uint32_t vType, void* vData)
{
  // TODO:
  return ERR_RT_NOT_IMPLEMENTED;
}

// ============================================================================
// [Fog::Var - Set]
// ============================================================================

static err_t FOG_CDECL Var_setType(Var* self, uint32_t vType, const void* vData)
{
  // TODO:
  return ERR_RT_NOT_IMPLEMENTED;
}

// ============================================================================
// [Fog::Var - Eq]
// ============================================================================

static bool FOG_CDECL Var_eq(const Var* a, const Var* b)
{
  // TODO:
  return false;
}

// ============================================================================
// [Fog::Var - Compare]
// ============================================================================

static int FOG_CDECL Var_compare(const Var* a, const Var* b)
{
  // TODO:
  return 0;
}

// ============================================================================
// [Fog::Var - Data]
// ============================================================================

static VarData* FOG_CDECL Var_dCreate(size_t dataSize)
{
  size_t memSize = sizeof(VarData) + dataSize;
  if (memSize < sizeof(VarSimpleData)) memSize = sizeof(VarSimpleData);

  VarData* d = reinterpret_cast<VarData*>(MemMgr::alloc(memSize));
  if (FOG_IS_NULL(d))
    return NULL;

  d->unknown.reference = 1;
  d->vType = VAR_TYPE_NULL;
  FOG_PADDING_ZERO_64(d->padding_0_32);

  return d;
}

static VarData* FOG_CDECL Var_dAddRef(VarData* d)
{
  uint32_t vType = d->vType & VAR_TYPE_MASK;

  // NULL type has no reference.
  if (vType == VAR_TYPE_NULL)
  {
    return d;
  }

  if (vType != VAR_TYPE_OBJECT_REF)
  {
    AtomicCore<size_t>::inc(&d->unknown.reference);
    return d;
  }
  else
  {
    // TODO: Var
    return d;
  }
}

static void FOG_CDECL Var_dRelease(VarData* d)
{
  uint32_t vType = d->vType & VAR_TYPE_MASK;

  switch (vType)
  {
    case VAR_TYPE_NULL:
      return;

    case VAR_TYPE_BOOL:
    case VAR_TYPE_CHAR:
    case VAR_TYPE_INT32:
    case VAR_TYPE_UINT32:
    case VAR_TYPE_INT64:
    case VAR_TYPE_UINT64:
    case VAR_TYPE_FLOAT:
    case VAR_TYPE_DOUBLE:
      goto _ReleaseSimple;

    case VAR_TYPE_STRINGA:
      reinterpret_cast<StringDataA*>(d)->release();
      return;

    case VAR_TYPE_STRINGW:
      reinterpret_cast<StringDataW*>(d)->release();
      return;

    case VAR_TYPE_LIST_STRINGA:
      if (reinterpret_cast<ListUntypedData*>(d)->reference.deref())
        _api.list.unknown.dFree(reinterpret_cast<ListUntypedData*>(d), _api.list.stringa.vTable);
      return;

    case VAR_TYPE_LIST_STRINGW:
      if (reinterpret_cast<ListUntypedData*>(d)->reference.deref())
        _api.list.unknown.dFree(reinterpret_cast<ListUntypedData*>(d), _api.list.stringw.vTable);
      return;

    case VAR_TYPE_LIST_VAR:
      if (reinterpret_cast<ListUntypedData*>(d)->reference.deref())
        _api.list.unknown.dFree(reinterpret_cast<ListUntypedData*>(d), _api.list.var.vTable);
      return;

    case VAR_TYPE_HASH_STRINGA_STRINGA:
      if (reinterpret_cast<HashUntypedData*>(d)->reference.deref())
        _api.hash.stringa_stringa.dFree(reinterpret_cast<HashUntypedData*>(d));
      return;

    case VAR_TYPE_HASH_STRINGA_VAR:
      if (reinterpret_cast<HashUntypedData*>(d)->reference.deref())
        _api.hash.stringa_var.dFree(reinterpret_cast<HashUntypedData*>(d));
      return;

    case VAR_TYPE_HASH_STRINGW_STRINGW:
      if (reinterpret_cast<HashUntypedData*>(d)->reference.deref())
        _api.hash.stringw_stringw.dFree(reinterpret_cast<HashUntypedData*>(d));
      return;

    case VAR_TYPE_HASH_STRINGW_VAR:
      if (reinterpret_cast<HashUntypedData*>(d)->reference.deref())
        _api.hash.stringw_var.dFree(reinterpret_cast<HashUntypedData*>(d));
      return;

    case VAR_TYPE_REGEXPA:
      reinterpret_cast<RegExpDataA*>(d)->release();
      return;

    case VAR_TYPE_REGEXPW:
      reinterpret_cast<RegExpDataW*>(d)->release();
      return;

    case VAR_TYPE_LOCALE:
      reinterpret_cast<LocaleData*>(d)->release();
      return;

    case VAR_TYPE_DATE:
    case VAR_TYPE_TIME:
      goto _ReleaseSimple;

    case VAR_TYPE_POINTI:
    case VAR_TYPE_POINTF:
    case VAR_TYPE_POINTD:
    case VAR_TYPE_SIZEI:
    case VAR_TYPE_SIZEF:
    case VAR_TYPE_SIZED:
    case VAR_TYPE_BOXI:
    case VAR_TYPE_BOXF:
    case VAR_TYPE_BOXD:
    case VAR_TYPE_RECTI:
    case VAR_TYPE_RECTF:
    case VAR_TYPE_RECTD:
    case VAR_TYPE_LINEF:
    case VAR_TYPE_LINED:
    case VAR_TYPE_QBEZIERF:
    case VAR_TYPE_QBEZIERD:
    case VAR_TYPE_CBEZIERF:
    case VAR_TYPE_CBEZIERD:
    case VAR_TYPE_TRIANGLEF:
    case VAR_TYPE_TRIANGLED:
    case VAR_TYPE_ROUNDF:
    case VAR_TYPE_ROUNDD:
    case VAR_TYPE_CIRCLEF:
    case VAR_TYPE_CIRCLED:
    case VAR_TYPE_ELLIPSEF:
    case VAR_TYPE_ELLIPSED:
    case VAR_TYPE_ARCF:
    case VAR_TYPE_ARCD:
    case VAR_TYPE_CHORDF:
    case VAR_TYPE_CHORDD:
    case VAR_TYPE_PIEF:
    case VAR_TYPE_PIED:
      goto _ReleaseSimple;

    case VAR_TYPE_POLYGONF:
      // TODO: Var.
      return;

    case VAR_TYPE_POLYGOND:
      // TODO: Var.
      return;

    case VAR_TYPE_PATHF:
      reinterpret_cast<PathDataF*>(d)->release();
      return;

    case VAR_TYPE_PATHD:
      reinterpret_cast<PathDataD*>(d)->release();
      return;

    case VAR_TYPE_REGION:
      reinterpret_cast<RegionData*>(d)->release();
      return;

    case VAR_TYPE_TRANSFORMF:
    case VAR_TYPE_TRANSFORMD:
      goto _ReleaseSimple;

    case VAR_TYPE_COLOR:
      goto _ReleaseSimple;

    case VAR_TYPE_TEXTURE:
    case VAR_TYPE_GRADIENTF:
    case VAR_TYPE_GRADIENTD:
    case VAR_TYPE_PATTERN:
      // TODO: Var
      return;

    case VAR_TYPE_COLOR_STOP:
      goto _ReleaseSimple;

    case VAR_TYPE_IMAGE:
    case VAR_TYPE_IMAGE_PALETTE:
    case VAR_TYPE_FONT:
      // TODO: Var
      return;

    default:
      return;
  }

_ReleaseSimple:
  // TODO:
  return;
}

// ============================================================================
// [Init / Fini]
// ============================================================================

FOG_NO_EXPORT void Var_init(void)
{
  // --------------------------------------------------------------------------
  // [Funcs]
  // --------------------------------------------------------------------------

  _api.var.ctor = Var_ctor;
  _api.var.ctorCopy = Var_ctorCopy;
  _api.var.ctorType = Var_ctorType;
  _api.var.dtor = Var_dtor;

  _api.var.getReference = Var_getReference;
  _api.var.getTypeId = Var_getTypeId;

  _api.var.reset = Var_reset;
  _api.var.copy = Var_copy;

  _api.var.getI32 = Var_getI32;
  _api.var.getI32Bound = Var_getI32Bound;

  _api.var.getU32 = Var_getU32;
  _api.var.getU32Bound = Var_getU32Bound;

  _api.var.getI64 = Var_getI64;
  _api.var.getI64Bound = Var_getI64Bound;

  _api.var.getU64 = Var_getU64;
  _api.var.getU64Bound = Var_getU64Bound;

  _api.var.getFloat = Var_getFloat;
  _api.var.getFloatBound = Var_getFloatBound;

  _api.var.getDouble = Var_getDouble;
  _api.var.getDoubleBound = Var_getDoubleBound;

  _api.var.getType = Var_getType;
  _api.var.setType = Var_setType;

  _api.var.eq = Var_eq;
  _api.var.compare = Var_compare;

  _api.var.dCreate = Var_dCreate;
  _api.var.dAddRef = Var_dAddRef;
  _api.var.dRelease = Var_dRelease;

  // --------------------------------------------------------------------------
  // [Data]
  // --------------------------------------------------------------------------

  VarData* d = &Var_dNull;

  d->unknown.reference = 1;
  d->vType = VAR_TYPE_NULL;

  _api.var.oNull = Var_oNull.initCustom1(d);
}

} // Fog namespace