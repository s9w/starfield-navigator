#pragma once

#include <cstdio>
#include <format>

namespace sfn::log
{

   inline auto error(const std::string& msg) -> void
   {
      printf(std::format("ERROR: {}", msg).c_str());
   }
}
