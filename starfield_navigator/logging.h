#pragma once

#include <cstdio>
#include <string>
#include <fmt/format.h>


namespace sfn::log
{

   inline auto error(const std::string& msg) -> void
   {
      fmt::print("ERROR: {}", msg);
   }
}
