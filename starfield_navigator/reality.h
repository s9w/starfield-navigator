#pragma once

#include <string>
#include <vector>

#include <glm/vec3.hpp>

#include "tools.h"

namespace sfn
{

   struct real_star
   {
      std::string m_identifier;
      std::string m_proper_name;
      galactic_coord m_pos;
      int m_reference_count = 0;
   };

   [[nodiscard]] auto read_real_stars() -> std::vector<real_star>;
   
}
