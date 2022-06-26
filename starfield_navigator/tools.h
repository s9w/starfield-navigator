#pragma once

#include <format>
#include <string_view>
#include <chrono>

using dbl_ms = std::chrono::duration<double, std::milli>;

namespace sfn
{
   // template <typename... param_types>
   // auto print(const std::string_view fmt, const param_types&... arg) -> void
   // {
   //    std::format(fmt, arg...);
   // }

   struct timer{
      std::chrono::high_resolution_clock::time_point m_t0;
      explicit timer()
         : m_t0(std::chrono::high_resolution_clock::now())
      {}

      ~timer()
      {
         const auto t1 = std::chrono::high_resolution_clock::now();
         const dbl_ms duration = dbl_ms(t1 - m_t0);
         printf(std::format("timer: {} ns\n", static_cast<int>(duration.count())).c_str()) ;
      }
   };

   template<std::floating_point T>
   [[nodiscard]] auto equal(const T a, const T b) -> bool
   {
      const T diff = std::abs(a - b);
      return diff < static_cast<T>(0.001);
   }
}
