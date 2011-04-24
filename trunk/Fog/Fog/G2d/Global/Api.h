// [Fog-G2d]
//
// [License]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_G2D_GLOBAL_API_H
#define _FOG_G2D_GLOBAL_API_H

// [Dependencies]
#include <Fog/Core/Global/Constants.h>
#include <Fog/Core/Tools/Range.h>
#include <Fog/G2d/Global/Constants.h>

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct ArcF;
struct ArcD;

struct ArgbBase32;
struct ArgbBase64;

struct BoxI;
struct BoxF;
struct BoxD;

struct Color;
struct ColorLutFx;
struct ColorLutTable;

struct Image;
struct ImageBits;
struct ImageFormatDescription;

struct Painter;
struct PainterEngine;

struct PathF;
struct PathD;

struct PathFlattenParamsF;
struct PathFlattenParamsD;

struct PathClipperF;
struct PathClipperD;

struct PointI;
struct PointF;
struct PointD;

struct RectI;
struct RectF;
struct RectD;

struct Region;

struct TransformF;
struct TransformD;

struct Stub8;
struct Utf16;

//! @addtogroup Fog_G2d_Global
//! @{

// ============================================================================
// [Fog::_G2dApi]
// ============================================================================

//! @internal
//!
//! @brief Fog/G2d API.
struct _G2dApi
{
  // --------------------------------------------------------------------------
  // [Arc]
  // --------------------------------------------------------------------------

  typedef void (FOG_CDECL *ArcF_GetBoundingBox)(const ArcF* self, BoxF* dst, bool includeCenterPoint);
  typedef void (FOG_CDECL *ArcD_GetBoundingBox)(const ArcD* self, BoxD* dst, bool includeCenterPoint);

  struct _FuncsArcF
  {
    ArcF_GetBoundingBox getBoundingBox;
  } arcf;

  struct _FuncsArcD
  {
    ArcD_GetBoundingBox getBoundingBox;
  } arcd;

  // --------------------------------------------------------------------------
  // [Line]
  // --------------------------------------------------------------------------

  typedef uint32_t (FOG_CDECL *LineF_Intersect)(
    PointF* dst,
    const PointF* lineA,
    const PointF* lineB);

  typedef uint32_t (FOG_CDECL *LineD_Intersect)(
    PointD* dst,
    const PointD* lineA,
    const PointD* lineB);

  struct _FuncsLineF
  {
    LineF_Intersect intersect;
  } linef;

  struct _FuncsLineD
  {
    LineD_Intersect intersect;
  } lined;

  // --------------------------------------------------------------------------
  // [QuadCurve]
  // --------------------------------------------------------------------------

  typedef void (FOG_CDECL *QuadCurveF_GetExtrema)(const PointF* self, BoxF* dstBox);
  typedef void (FOG_CDECL *QuadCurveD_GetExtrema)(const PointD* self, BoxD* dstBox);

  typedef void (FOG_CDECL *QuadCurveF_GetLength)(const PointF* self, float* length);
  typedef void (FOG_CDECL *QuadCurveD_GetLength)(const PointD* self, double* length);

  typedef err_t (FOG_CDECL *QuadCurveF_Flatten)(const PointF* self, PathF& dst, uint8_t initialCommand, float flatness);
  typedef err_t (FOG_CDECL *QuadCurveD_Flatten)(const PointD* self, PathD& dst, uint8_t initialCommand, double flatness);

  struct _FuncsQuadCurveF
  {
    QuadCurveF_GetExtrema getExtrema;
    QuadCurveF_GetLength getLength;
    QuadCurveF_Flatten flatten;
  } quadcurvef;

  struct _FuncsQuadCurveD
  {
    QuadCurveD_GetExtrema getExtrema;
    QuadCurveD_GetLength getLength;
    QuadCurveD_Flatten flatten;
  } quadcurved;

  // --------------------------------------------------------------------------
  // [CubicCurve]
  // --------------------------------------------------------------------------

  typedef void (FOG_CDECL *CubicCurveF_GetExtrema)(const PointF* self, BoxF* dstBox);
  typedef void (FOG_CDECL *CubicCurveD_GetExtrema)(const PointD* self, BoxD* dstBox);

  typedef void (FOG_CDECL *CubicCurveF_GetLength)(const PointF* self, float* length);
  typedef void (FOG_CDECL *CubicCurveD_GetLength)(const PointD* self, double* length);

  typedef int (FOG_CDECL *CubicCurveF_GetInflectionPoints)(const PointF* self, float*  t);
  typedef int (FOG_CDECL *CubicCurveD_GetInflectionPoints)(const PointD* self, double* t);

  typedef int (FOG_CDECL *CubicCurveF_SimplifyForProcessing)(const PointF* self, PointF* pts, float flatness);
  typedef int (FOG_CDECL *CubicCurveD_SimplifyForProcessing)(const PointD* self, PointD* pts, double flatness);

  typedef err_t (FOG_CDECL *CubicCurveF_Flatten)(const PointF* self, PathF& dst, uint8_t initialCommand, float flatness);
  typedef err_t (FOG_CDECL *CubicCurveD_Flatten)(const PointD* self, PathD& dst, uint8_t initialCommand, double flatness);

  struct _FuncsCubicCurveF
  {
    CubicCurveF_GetExtrema getExtrema;
    CubicCurveF_GetLength getLength;
    CubicCurveF_GetInflectionPoints getInflectionPoints;
    CubicCurveF_SimplifyForProcessing simplifyForProcessing;
    CubicCurveF_Flatten flatten;
  } cubiccurvef;

  struct _FuncsCubicCurveD
  {
    CubicCurveD_GetExtrema getExtrema;
    CubicCurveD_GetLength getLength;
    CubicCurveD_GetInflectionPoints getInflectionPoints;
    CubicCurveD_SimplifyForProcessing simplifyForProcessing;
    CubicCurveD_Flatten flatten;
  } cubiccurved;

  // --------------------------------------------------------------------------
  // [Path]
  // --------------------------------------------------------------------------

  typedef void (FOG_CDECL *PathF_Ctor)(PathF& self);
  typedef void (FOG_CDECL *PathD_Ctor)(PathD& self);

  typedef void (FOG_CDECL *PathF_CtorCopyF)(PathF& self, const PathF& other);
  typedef void (FOG_CDECL *PathD_CtorCopyD)(PathD& self, const PathD& other);

  typedef void (FOG_CDECL *PathF_Dtor)(PathF& self);
  typedef void (FOG_CDECL *PathD_Dtor)(PathD& self);

  typedef err_t (FOG_CDECL *PathF_Detach)(PathF& self);
  typedef err_t (FOG_CDECL *PathD_Detach)(PathD& self);

  typedef err_t (FOG_CDECL *PathF_Reserve)(PathF& self, sysuint_t capacity);
  typedef err_t (FOG_CDECL *PathD_Reserve)(PathD& self, sysuint_t capacity);

  typedef void (FOG_CDECL *PathF_Squeeze)(PathF& self);
  typedef void (FOG_CDECL *PathD_Squeeze)(PathD& self);

  typedef sysuint_t (FOG_CDECL *PathF_Add)(PathF& self, sysuint_t count);
  typedef sysuint_t (FOG_CDECL *PathD_Add)(PathD& self, sysuint_t count);

  typedef void (FOG_CDECL *PathF_UpdateFlat)(const PathF& self);
  typedef void (FOG_CDECL *PathD_UpdateFlat)(const PathD& self);

  typedef void (FOG_CDECL *PathF_UpdateBoundingBox)(const PathF& self);
  typedef void (FOG_CDECL *PathD_UpdateBoundingBox)(const PathD& self);

  typedef void (FOG_CDECL *PathF_Clear)(PathF& self);
  typedef void (FOG_CDECL *PathD_Clear)(PathD& self);

  typedef void (FOG_CDECL *PathF_Reset)(PathF& self);
  typedef void (FOG_CDECL *PathD_Reset)(PathD& self);

  typedef err_t (FOG_CDECL *PathF_SetPathF)(PathF& self, const PathF& other);
  typedef err_t (FOG_CDECL *PathD_SetPathD)(PathD& self, const PathD& other);

  typedef err_t (FOG_CDECL *PathF_SetDeepF)(PathF& self, const PathF& other);
  typedef err_t (FOG_CDECL *PathD_SetDeepD)(PathD& self, const PathD& other);

  typedef Range (FOG_CDECL *PathF_GetSubpathRange)(const PathF& self, sysuint_t index);
  typedef Range (FOG_CDECL *PathD_GetSubpathRange)(const PathD& self, sysuint_t index);

  typedef err_t (FOG_CDECL *PathF_MoveTo)(PathF& self, const PointF& pt0);
  typedef err_t (FOG_CDECL *PathD_MoveTo)(PathD& self, const PointD& pt0);

  typedef err_t (FOG_CDECL *PathF_LineTo)(PathF& self, const PointF& pt0);
  typedef err_t (FOG_CDECL *PathD_LineTo)(PathD& self, const PointD& pt0);

  typedef err_t (FOG_CDECL *PathF_HLineTo)(PathF& self, float x);
  typedef err_t (FOG_CDECL *PathD_HLineTo)(PathD& self, double x);

  typedef err_t (FOG_CDECL *PathF_VLineTo)(PathF& self, float y);
  typedef err_t (FOG_CDECL *PathD_VLineTo)(PathD& self, double y);

  typedef err_t (FOG_CDECL *PathF_PolyTo)(PathF& self, const PointF* pts, sysuint_t count);
  typedef err_t (FOG_CDECL *PathD_PolyTo)(PathD& self, const PointD* pts, sysuint_t count);

  typedef err_t (FOG_CDECL *PathF_QuadTo)(PathF& self, const PointF& pt1, const PointF& pt2);
  typedef err_t (FOG_CDECL *PathD_QuadTo)(PathD& self, const PointD& pt1, const PointD& pt2);

  typedef err_t (FOG_CDECL *PathF_CubicTo)(PathF& self, const PointF& pt1, const PointF& pt2, const PointF& pt3);
  typedef err_t (FOG_CDECL *PathD_CubicTo)(PathD& self, const PointD& pt1, const PointD& pt2, const PointD& pt3);

  typedef err_t (FOG_CDECL *PathF_SmoothQuadTo)(PathF& self, const PointF& pt2);
  typedef err_t (FOG_CDECL *PathD_SmoothQuadTo)(PathD& self, const PointD& pt2);

  typedef err_t (FOG_CDECL *PathF_SmoothCubicTo)(PathF& self, const PointF& pt1, const PointF& pt2);
  typedef err_t (FOG_CDECL *PathD_SmoothCubicTo)(PathD& self, const PointD& pt1, const PointD& pt2);

  typedef err_t (FOG_CDECL *PathF_ArcTo)(PathF& self, const PointF& cp, const PointF& rp, float start, float sweep, bool startPath);
  typedef err_t (FOG_CDECL *PathD_ArcTo)(PathD& self, const PointD& cp, const PointD& rp, double start, double sweep, bool startPath);

  typedef err_t (FOG_CDECL *PathF_SvgArcTo)(PathF& self, const PointF& rp, float angle, bool largeArcFlag, bool sweepFlag, const PointF& pt);
  typedef err_t (FOG_CDECL *PathD_SvgArcTo)(PathD& self, const PointD& rp, double angle, bool largeArcFlag, bool sweepFlag, const PointD& pt);

  typedef err_t (FOG_CDECL *PathF_Close)(PathF& self);
  typedef err_t (FOG_CDECL *PathD_Close)(PathD& self);

  typedef err_t (FOG_CDECL *PathF_BoxI)(PathF& self, const BoxI& r, uint32_t direction);
  typedef err_t (FOG_CDECL *PathD_BoxI)(PathD& self, const BoxI& r, uint32_t direction);

  typedef err_t (FOG_CDECL *PathF_BoxF)(PathF& self, const BoxF& r, uint32_t direction);
  typedef err_t (FOG_CDECL *PathD_BoxF)(PathD& self, const BoxF& r, uint32_t direction);
  typedef err_t (FOG_CDECL *PathD_BoxD)(PathD& self, const BoxD& r, uint32_t direction);

  typedef err_t (FOG_CDECL *PathF_RectI)(PathF& self, const RectI& r, uint32_t direction);
  typedef err_t (FOG_CDECL *PathD_RectI)(PathD& self, const RectI& r, uint32_t direction);

  typedef err_t (FOG_CDECL *PathF_RectF)(PathF& self, const RectF& r, uint32_t direction);
  typedef err_t (FOG_CDECL *PathD_RectF)(PathD& self, const RectF& r, uint32_t direction);
  typedef err_t (FOG_CDECL *PathD_RectD)(PathD& self, const RectD& r, uint32_t direction);

  typedef err_t (FOG_CDECL *PathF_BoxesI)(PathF& self, const BoxI* r, sysuint_t count, uint32_t direction);
  typedef err_t (FOG_CDECL *PathD_BoxesI)(PathD& self, const BoxI* r, sysuint_t count, uint32_t direction);

  typedef err_t (FOG_CDECL *PathF_BoxesF)(PathF& self, const BoxF* r, sysuint_t count, uint32_t direction);
  typedef err_t (FOG_CDECL *PathD_BoxesF)(PathD& self, const BoxF* r, sysuint_t count, uint32_t direction);
  typedef err_t (FOG_CDECL *PathD_BoxesD)(PathD& self, const BoxD* r, sysuint_t count, uint32_t direction);

  typedef err_t (FOG_CDECL *PathF_RectsI)(PathF& self, const RectI* r, sysuint_t count, uint32_t direction);
  typedef err_t (FOG_CDECL *PathD_RectsI)(PathD& self, const RectI* r, sysuint_t count, uint32_t direction);

  typedef err_t (FOG_CDECL *PathF_RectsF)(PathF& self, const RectF* r, sysuint_t count, uint32_t direction);
  typedef err_t (FOG_CDECL *PathD_RectsF)(PathD& self, const RectF* r, sysuint_t count, uint32_t direction);
  typedef err_t (FOG_CDECL *PathD_RectsD)(PathD& self, const RectD* r, sysuint_t count, uint32_t direction);

  typedef err_t (FOG_CDECL *PathF_Region)(PathF& self, const Region& r, uint32_t direction);
  typedef err_t (FOG_CDECL *PathD_Region)(PathD& self, const Region& r, uint32_t direction);

  typedef err_t (FOG_CDECL *PathF_PolylineI)(PathF& self, const PointI* pts, sysuint_t count, uint32_t direction);
  typedef err_t (FOG_CDECL *PathD_PolylineI)(PathD& self, const PointI* pts, sysuint_t count, uint32_t direction);

  typedef err_t (FOG_CDECL *PathF_PolylineF)(PathF& self, const PointF* pts, sysuint_t count, uint32_t direction);
  typedef err_t (FOG_CDECL *PathD_PolylineD)(PathD& self, const PointD* pts, sysuint_t count, uint32_t direction);

  typedef err_t (FOG_CDECL *PathF_PolygonI)(PathF& self, const PointI* pts, sysuint_t count, uint32_t direction);
  typedef err_t (FOG_CDECL *PathD_PolygonI)(PathD& self, const PointI* pts, sysuint_t count, uint32_t direction);

  typedef err_t (FOG_CDECL *PathF_PolygonF)(PathF& self, const PointF* pts, sysuint_t count, uint32_t direction);
  typedef err_t (FOG_CDECL *PathD_PolygonD)(PathD& self, const PointD* pts, sysuint_t count, uint32_t direction);

  typedef err_t (FOG_CDECL *PathF_Shape)(PathF& self, uint32_t shapeType, const void* shapeData, uint32_t direction, const TransformF* tr);
  typedef err_t (FOG_CDECL *PathD_Shape)(PathD& self, uint32_t shapeType, const void* shapeData, uint32_t direction, const TransformD* tr);

  typedef err_t (FOG_CDECL *PathF_AppendPath)(PathF& self, const PathF& path, const Range* range);
  typedef err_t (FOG_CDECL *PathD_AppendPath)(PathD& self, const PathD& path, const Range* range);

  typedef err_t (FOG_CDECL *PathF_AppendTranslatedPath)(PathF& self, const PathF& path, const PointF& pt, const Range* range);
  typedef err_t (FOG_CDECL *PathD_AppendTranslatedPath)(PathD& self, const PathD& path, const PointD& pt, const Range* range);

  typedef err_t (FOG_CDECL *PathF_AppendTransformedPath)(PathF& self, const PathF& path, const TransformF& tr, const Range* range);
  typedef err_t (FOG_CDECL *PathD_AppendTransformedPath)(PathD& self, const PathD& path, const TransformD& tr, const Range* range);

  typedef err_t (FOG_CDECL *PathF_Translate)(PathF& self, const PointF& pt, const Range* range);
  typedef err_t (FOG_CDECL *PathD_Translate)(PathD& self, const PointD& pt, const Range* range);

  typedef err_t (FOG_CDECL *PathF_Transform)(PathF& self, const TransformF& tr, const Range* range);
  typedef err_t (FOG_CDECL *PathD_Transform)(PathD& self, const TransformD& tr, const Range* range);

  typedef err_t (FOG_CDECL *PathF_FitTo)(PathF& self, const RectF& r);
  typedef err_t (FOG_CDECL *PathD_FitTo)(PathD& self, const RectD& r);

  typedef err_t (FOG_CDECL *PathF_Scale)(PathF& self, const PointF& pt, bool keepStartPos);
  typedef err_t (FOG_CDECL *PathD_Scale)(PathD& self, const PointD& pt, bool keepStartPos);

  typedef err_t (FOG_CDECL *PathF_FlipX)(PathF& self, float x0, float x1);
  typedef err_t (FOG_CDECL *PathD_FlipX)(PathD& self, double x0, double x1);

  typedef err_t (FOG_CDECL *PathF_FlipY)(PathF& self, float y0, float y1);
  typedef err_t (FOG_CDECL *PathD_FlipY)(PathD& self, double y0, double y1);

  typedef err_t (FOG_CDECL *PathF_Flatten)(PathF& dst, const PathF& src, const PathFlattenParamsF& params, const Range* range);
  typedef err_t (FOG_CDECL *PathD_Flatten)(PathD& dst, const PathD& src, const PathFlattenParamsD& params, const Range* range);

  typedef bool (FOG_CDECL *PathF_Eq)(const PathF& a, const PathF& b);
  typedef bool (FOG_CDECL *PathD_Eq)(const PathD& a, const PathD& b);

  struct _FuncsPathF
  {
    PathF_Ctor ctor;
    PathF_CtorCopyF ctorCopyF;
    PathF_Dtor dtor;
    PathF_Detach detach;
    PathF_Reserve reserve;
    PathF_Squeeze squeeze;
    PathF_Add add;
    PathF_UpdateFlat updateFlat;
    PathF_UpdateBoundingBox updateBoundingBox;
    PathF_Clear clear;
    PathF_Reset reset;
    PathF_SetPathF setPathF;
    PathF_SetDeepF setDeepF;
    PathF_GetSubpathRange getSubpathRange;
    PathF_MoveTo moveTo;
    PathF_MoveTo moveToRel;
    PathF_LineTo lineTo;
    PathF_LineTo lineToRel;
    PathF_HLineTo hlineTo;
    PathF_HLineTo hlineToRel;
    PathF_VLineTo vlineTo;
    PathF_VLineTo vlineToRel;
    PathF_PolyTo polyTo;
    PathF_PolyTo polyToRel;
    PathF_QuadTo quadTo;
    PathF_QuadTo quadToRel;
    PathF_CubicTo cubicTo;
    PathF_CubicTo cubicToRel;
    PathF_SmoothQuadTo smoothQuadTo;
    PathF_SmoothQuadTo smoothQuadToRel;
    PathF_SmoothCubicTo smoothCubicTo;
    PathF_SmoothCubicTo smoothCubicToRel;
    PathF_ArcTo arcTo;
    PathF_ArcTo arcToRel;
    PathF_SvgArcTo svgArcTo;
    PathF_SvgArcTo svgArcToRel;
    PathF_Close close;
    PathF_BoxI boxI;
    PathF_BoxF boxF;
    PathF_RectI rectI;
    PathF_RectF rectF;
    PathF_BoxesI boxesI;
    PathF_BoxesF boxesF;
    PathF_RectsI rectsI;
    PathF_RectsF rectsF;
    PathF_Region region;
    PathF_PolylineI polylineI;
    PathF_PolylineF polylineF;
    PathF_PolygonI polygonI;
    PathF_PolygonF polygonF;
    PathF_Shape shape;
    PathF_AppendPath appendPath;
    PathF_AppendTranslatedPath appendTranslatedPath;
    PathF_AppendTransformedPath appendTransformedPath;
    PathF_Translate translate;
    PathF_Transform transform;
    PathF_FitTo fitTo;
    PathF_Scale scale;
    PathF_FlipX flipX;
    PathF_FlipY flipY;
    PathF_Flatten flatten;
    PathF_Eq eq;
  } pathf;

  struct _FuncsPathD
  {
    PathD_Ctor ctor;
    PathD_CtorCopyD ctorCopyD;
    PathD_Dtor dtor;
    PathD_Detach detach;
    PathD_Reserve reserve;
    PathD_Squeeze squeeze;
    PathD_Add add;
    PathD_UpdateFlat updateFlat;
    PathD_UpdateBoundingBox updateBoundingBox;
    PathD_Clear clear;
    PathD_Reset reset;
    PathD_SetPathD setPathD;
    PathD_SetDeepD setDeepD;
    PathD_GetSubpathRange getSubpathRange;
    PathD_MoveTo moveTo;
    PathD_MoveTo moveToRel;
    PathD_LineTo lineTo;
    PathD_LineTo lineToRel;
    PathD_HLineTo hlineTo;
    PathD_HLineTo hlineToRel;
    PathD_VLineTo vlineTo;
    PathD_VLineTo vlineToRel;
    PathD_PolyTo polyTo;
    PathD_PolyTo polyToRel;
    PathD_QuadTo quadTo;
    PathD_QuadTo quadToRel;
    PathD_CubicTo cubicTo;
    PathD_CubicTo cubicToRel;
    PathD_SmoothQuadTo smoothQuadTo;
    PathD_SmoothQuadTo smoothQuadToRel;
    PathD_SmoothCubicTo smoothCubicTo;
    PathD_SmoothCubicTo smoothCubicToRel;
    PathD_ArcTo arcTo;
    PathD_ArcTo arcToRel;
    PathD_SvgArcTo svgArcTo;
    PathD_SvgArcTo svgArcToRel;
    PathD_Close close;
    PathD_BoxI boxI;
    PathD_BoxF boxF;
    PathD_BoxD boxD;
    PathD_RectI rectI;
    PathD_RectF rectF;
    PathD_RectD rectD;
    PathD_BoxesI boxesI;
    PathD_BoxesF boxesF;
    PathD_BoxesD boxesD;
    PathD_RectsI rectsI;
    PathD_RectsF rectsF;
    PathD_RectsD rectsD;
    PathD_Region region;
    PathD_PolylineI polylineI;
    PathD_PolylineD polylineD;
    PathD_PolygonI polygonI;
    PathD_PolygonD polygonD;
    PathD_Shape shape;
    PathD_AppendPath appendPath;
    PathD_AppendTranslatedPath appendTranslatedPath;
    PathD_AppendTransformedPath appendTransformedPath;
    PathD_Translate translate;
    PathD_Transform transform;
    PathD_FitTo fitTo;
    PathD_Scale scale;
    PathD_FlipX flipX;
    PathD_FlipY flipY;
    PathD_Flatten flatten;
    PathD_Eq eq;
  } pathd;

  // --------------------------------------------------------------------------
  // [PathClipper]
  // --------------------------------------------------------------------------

  typedef uint32_t (FOG_CDECL *PathClipperF_InitPath)(PathClipperF& self, const PathF& src);
  typedef uint32_t (FOG_CDECL *PathClipperD_InitPath)(PathClipperD& self, const PathD& src);

  typedef err_t (FOG_CDECL *PathClipperF_ContinuePath)(PathClipperF& self, PathF& dst, const PathF& src);
  typedef err_t (FOG_CDECL *PathClipperD_ContinuePath)(PathClipperD& self, PathD& dst, const PathD& src);

  typedef err_t (FOG_CDECL *PathClipperF_ContinueRaw)(PathClipperF& self, PathF& dst, const PointF* srcPts, const uint8_t* srcCmd, sysuint_t srcLength);
  typedef err_t (FOG_CDECL *PathClipperD_ContinueRaw)(PathClipperD& self, PathD& dst, const PointD* srcPts, const uint8_t* srcCmd, sysuint_t srcLength);

  typedef err_t (FOG_CDECL *PathClipperF_ClipPath)(PathClipperF& self, PathF& dst, const PathF& src, const TransformF* tr);
  typedef err_t (FOG_CDECL *PathClipperD_ClipPath)(PathClipperD& self, PathD& dst, const PathD& src, const TransformD* tr);

  struct _FuncsPathClipperF
  {
    PathClipperF_InitPath initPath;
    PathClipperF_ContinuePath continuePath;
    PathClipperF_ContinueRaw continueRaw;
    PathClipperF_ClipPath clipPath;
  } pathclipperf;

  struct _FuncsPathClipperD
  {
    PathClipperD_InitPath initPath;
    PathClipperD_ContinuePath continuePath;
    PathClipperD_ContinueRaw continueRaw;
    PathClipperD_ClipPath clipPath;
  } pathclipperd;

  // --------------------------------------------------------------------------
  // [Shape]
  // --------------------------------------------------------------------------

  typedef void (FOG_CDECL *ShapeF_GetBoundingBox)(uint32_t shapeType, const void* shapeData, BoxF* dst);
  typedef void (FOG_CDECL *ShapeD_GetBoundingBox)(uint32_t shapeType, const void* shapeData, BoxD* dst);

  struct _FuncsShapeF
  {
    ShapeF_GetBoundingBox getBoundingBox;
  } shapef;

  struct _FuncsShapeD
  {
    ShapeD_GetBoundingBox getBoundingBox;
  } shaped;

  // --------------------------------------------------------------------------
  // [Color]
  // --------------------------------------------------------------------------

  typedef void (FOG_CDECL *Color_Convert)(void* dst, const void* src);

  typedef err_t (FOG_CDECL *Color_SetModel)(Color& self, uint32_t model);
  typedef err_t (FOG_CDECL *Color_SetData)(Color& self, uint32_t modelExtended, const void* modelData);
  typedef err_t (FOG_CDECL *Color_Mix)(Color& self, uint32_t mixOp, uint32_t alphaOp, const Color& secondary, float mask);
  typedef err_t (FOG_CDECL *Color_Adjust)(Color& self, uint32_t adjustOp, float param);

  typedef err_t (FOG_CDECL *Color_ParseA)(Color& self, const Stub8& str, uint32_t flags);
  typedef err_t (FOG_CDECL *Color_ParseU)(Color& self, const Utf16& str, uint32_t flags);

  struct _FuncsColor
  {
    Color_Convert convert[_COLOR_MODEL_COUNT][_COLOR_MODEL_COUNT];

    Color_SetModel setModel;
    Color_SetData setData;

    Color_Mix mix;
    Color_Adjust adjust;

    Color_ParseA parseA;
    Color_ParseU parseU;
  } color;

  // --------------------------------------------------------------------------
  // [ColorLutFx]
  // --------------------------------------------------------------------------

  // TODO: Deprecated...
  typedef void (FOG_FASTCALL *ColorLut_Reset)(ColorLutTable* self, uint32_t channel);

  struct _FuncsColorLut
  {
    ColorLut_Reset reset;
  } colorlut;

  // --------------------------------------------------------------------------
  // [ImageFormatDescription]
  // --------------------------------------------------------------------------

  typedef uint32_t (FOG_CDECL *ImageFormatDescription_GetCompatibleFormat)(
    const ImageFormatDescription* self);

  typedef err_t (FOG_CDECL *ImageFormatDescription_CreateArgb)(
    ImageFormatDescription* self,
    uint32_t depth, uint32_t flags,
    uint64_t aMask, uint64_t rMask, uint64_t gMask, uint64_t bMask);

  struct _ImageFormatDescriptionFuncs
  {
    ImageFormatDescription_GetCompatibleFormat getCompatibleFormat;
    ImageFormatDescription_CreateArgb createArgb;
  } imageformatdescription;

  // --------------------------------------------------------------------------
  // [Painter]
  // --------------------------------------------------------------------------

  typedef PainterEngine* (FOG_CDECL *Painter_GetNullEngine)();

  typedef err_t (FOG_CDECL *Painter_BeginImage)(Painter& self, Image& image, const RectI* rect, uint32_t initFlags);
  typedef err_t (FOG_CDECL *Painter_BeginIBits)(Painter& self, const ImageBits& imageBits, const RectI* rect, uint32_t initFlags);

  typedef err_t (FOG_CDECL *Painter_SwitchToImage)(Painter& self, Image& image, const RectI* rect);
  typedef err_t (FOG_CDECL *Painter_SwitchToIBits)(Painter& self, const ImageBits& imageBits, const RectI* rect);

  struct _FuncsPainter
  {
    Painter_GetNullEngine getNullEngine;
    Painter_BeginImage beginImage;
    Painter_BeginIBits beginIBits;
    Painter_SwitchToImage switchToImage;
    Painter_SwitchToIBits switchToIBits;
  } painter;

  // --------------------------------------------------------------------------
  // [Transform]
  // --------------------------------------------------------------------------

  typedef err_t (FOG_CDECL *TransformF_Create)(TransformF& self, uint32_t createType, const void* params);
  typedef err_t (FOG_CDECL *TransformD_Create)(TransformD& self, uint32_t createType, const void* params);

  typedef uint32_t (FOG_CDECL *TransformF_Update)(const TransformF& self);
  typedef uint32_t (FOG_CDECL *TransformD_Update)(const TransformD& self);

  typedef err_t (FOG_CDECL *TransformF_Transform)(TransformF& self, uint32_t transformOp, const void* params);
  typedef err_t (FOG_CDECL *TransformD_Transform)(TransformD& self, uint32_t transformOp, const void* params);

  typedef TransformF (FOG_CDECL *TransformF_Transformed)(const TransformF& self, uint32_t transformOp, const void* params);
  typedef TransformD (FOG_CDECL *TransformD_Transformed)(const TransformD& self, uint32_t transformOp, const void* params);

  typedef void (FOG_CDECL *TransformF_Multiply)(TransformF& self, const TransformF& a, const TransformF& b);
  typedef void (FOG_CDECL *TransformD_Multiply)(TransformD& self, const TransformD& a, const TransformD& b);

  typedef bool (FOG_CDECL *TransformF_Invert)(TransformF& self, const TransformF& a);
  typedef bool (FOG_CDECL *TransformD_Invert)(TransformD& self, const TransformD& a);

  typedef void (FOG_CDECL *TransformF_MapPointF)(const TransformF& self, PointF& dst, const PointF& src);
  typedef void (FOG_CDECL *TransformD_MapPointD)(const TransformD& self, PointD& dst, const PointD& src);

  typedef void (FOG_CDECL *TransformF_MapPointsF)(const TransformF& self, PointF* dst, const PointF* src, sysuint_t length);
  typedef void (FOG_CDECL *TransformD_MapPointsD)(const TransformD& self, PointD* dst, const PointD* src, sysuint_t length);

  typedef err_t (FOG_CDECL *TransformF_MapPathF)(const TransformF& self, PathF& dst, const PathF& src);
  typedef err_t (FOG_CDECL *TransformD_MapPathD)(const TransformD& self, PathD& dst, const PathD& src);

  typedef void (FOG_CDECL *TransformF_MapBoxF)(const TransformF& self, BoxF& dst, const BoxF& src);
  typedef void (FOG_CDECL *TransformD_MapBoxD)(const TransformD& self, BoxD& dst, const BoxD& src);

  typedef void (FOG_CDECL *TransformF_MapVectorF)(const TransformF& self, PointF& dst, const PointF& src);
  typedef void (FOG_CDECL *TransformD_MapVectorD)(const TransformD& self, PointD& dst, const PointD& src);

  typedef PointF (FOG_CDECL *TransformF_GetScaling)(const TransformF& self, bool absolute);
  typedef PointD (FOG_CDECL *TransformD_GetScaling)(const TransformD& self, bool absolute);

  typedef float (FOG_CDECL *TransformF_GetRotation)(const TransformF& self);
  typedef double (FOG_CDECL *TransformD_GetRotation)(const TransformD& self);

  typedef float (FOG_CDECL *TransformF_GetAverageScaling)(const TransformF& self);
  typedef double (FOG_CDECL *TransformD_GetAverageScaling)(const TransformD& self);

  struct _FuncsTransformF
  {
    TransformF_Create create;
    TransformF_Update update;
    TransformF_Transform transform;
    TransformF_Transformed transformed;
    TransformF_Multiply multiply;
    TransformF_Invert invert;
    TransformF_MapPointF mapPointF;
    TransformF_MapPointsF mapPointsF[TRANSFORM_TYPE_COUNT];
    TransformF_MapPathF mapPathF;
    TransformF_MapBoxF mapBoxF;
    TransformF_MapVectorF mapVectorF;
    TransformF_GetScaling getScaling;
    TransformF_GetRotation getRotation;
    TransformF_GetAverageScaling getAverageScaling;
  } transformf;

  struct _FuncsTransformD
  {
    TransformD_Create create;
    TransformD_Update update;
    TransformD_Transform transform;
    TransformD_Transformed transformed;
    TransformD_Multiply multiply;
    TransformD_Invert invert;
    TransformD_MapPointD mapPointD;
    TransformD_MapPointsD mapPointsD[TRANSFORM_TYPE_COUNT];
    TransformD_MapPathD mapPathD;
    TransformD_MapBoxD mapBoxD;
    TransformD_MapVectorD mapVectorD;
    TransformD_GetScaling getScaling;
    TransformD_GetRotation getRotation;
    TransformD_GetAverageScaling getAverageScaling;
  } transformd;
};

extern FOG_API _G2dApi _g2d;

//! @}

} // Fog namespace

// [Guard]
#endif // _FOG_G2D_GLOBAL_API_H