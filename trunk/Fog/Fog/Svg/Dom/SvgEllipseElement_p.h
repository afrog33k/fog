// [Fog-Svg]
//
// [License]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_SVG_DOM_SVGELLIPSEELEMENT_P_H
#define _FOG_SVG_DOM_SVGELLIPSEELEMENT_P_H

// [Dependencies]
#include <Fog/Core/Global/Global.h>
#include <Fog/Svg/Dom/SvgCoordAttribute_p.h>
#include <Fog/Svg/Dom/SvgStyledElement_p.h>

namespace Fog {

//! @addtogroup Fog_Svg_Dom
//! @{

// ============================================================================
// [Fog::SvgEllipseElement]
// ============================================================================

struct FOG_NO_EXPORT SvgEllipseElement : public SvgStyledElement
{
  typedef SvgStyledElement base;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  SvgEllipseElement();
  virtual ~SvgEllipseElement();

  // --------------------------------------------------------------------------
  // [SVG Attributes]
  // --------------------------------------------------------------------------

  virtual XmlAttribute* _createAttribute(const ManagedString& name) const;

  // --------------------------------------------------------------------------
  // [SVG Interface]
  // --------------------------------------------------------------------------

  virtual err_t onProcess(SvgVisitor* visitor) const;
  virtual err_t onGeometryBoundingBox(BoxF& box, const TransformF* tr) const;

  // --------------------------------------------------------------------------
  // [SVG Embedded Attributes]
  // --------------------------------------------------------------------------

  SvgCoordAttribute a_cx;
  SvgCoordAttribute a_cy;
  SvgCoordAttribute a_rx;
  SvgCoordAttribute a_ry;

private:
  _FOG_CLASS_NO_COPY(SvgEllipseElement)
};

//! @}

} // Fog namespace

// [Guard]
#endif // _FOG_SVG_DOM_SVGELLIPSEELEMENT_P_H
