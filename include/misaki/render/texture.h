#pragma once

#include "component.h"

namespace misaki::render {

class MSK_EXPORT Texture : public Component {
 public:
  Texture(const Properties &props);
  virtual Float eval_1(const PointGeometry &geom) const;
  virtual Color3 eval_3(const PointGeometry &geom) const;

  MSK_DECL_COMP(Component)
 protected:
  std::string m_id;
};

}  // namespace misaki::render