#pragma once

#include "fwd.h"

namespace misaki::render {

class MSK_EXPORT Properties {
 public:
  Properties();
  Properties(const std::string &plugin_name);

  const std::string& plugin_name() const;
 private:
  std::string m_id, m_plugin_name;
};

}