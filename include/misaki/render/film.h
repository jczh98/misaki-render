#pragma once

#include "component.h"
#include "rfilter.h"
#include "imageblock.h"

namespace misaki::render {

class MSK_EXPORT Film : public Component {
 public:
  Film(const Properties &props);
  virtual void put(const ImageBlock *block) = 0;
  virtual void set_destination_file(const fs::path &filename) = 0;
  virtual void develop() = 0;
  const Vector2i &size() const { return m_size; }
  const ReconstructionFilter *filter() const { return m_filter.get(); }
  virtual std::string to_string() const override;

  MSK_DECL_COMP(Component)
 protected:
  Vector2i m_size;
  std::shared_ptr<ReconstructionFilter> m_filter;
};

}  // namespace misaki::render