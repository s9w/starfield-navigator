#pragma once

#include <format>
#include <string_view>
#include <chrono>
#include <unordered_map>

#include <s9w_core.h>

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
         printf(std::format("timer: {} ms\n", static_cast<int>(duration.count())).c_str()) ;
      }
   };

   template<std::floating_point T>
   [[nodiscard]] auto equal(const T a, const T b) -> bool
   {
      const T diff = std::abs(a - b);
      return diff < static_cast<T>(0.001);
   }

   auto constexpr sfn_assert(const bool condition, const std::string& msg ) -> void;
   auto constexpr sfn_assert(const bool condition) -> void;

   struct explicit_init {};
   struct no_init {};

   struct id
   {
      using underlying_type = uint32_t;
      underlying_type m_id{};
      static constexpr inline underlying_type null_id = std::numeric_limits<underlying_type>::max();

      constexpr explicit id(explicit_init, const underlying_type id) : m_id(id) {}
      constexpr explicit id(no_init) : m_id(null_id) {}
      [[nodiscard]] static auto create()->id;
   };
   constexpr auto operator==(const id a, const id b) -> bool;

   template<typename type_with_id>
   [[nodiscard]] auto get_thing_by_id(const std::vector<type_with_id>& vec, const id target_id) -> const type_with_id&;

   template <typename T, typename pred_type>
   auto find_obj(const std::vector<T>& vec, const pred_type& pred, const std::string& error_msg) -> const T&;

   [[nodiscard]] auto get_average(const std::vector<float>& vec) -> float;

   template <std::floating_point T>
   constexpr auto srgb_to_linear_fp(const T x)->T;
   auto constexpr srgb_to_linear_ui8(const uint8_t in)->uint8_t;

   struct image_metrics
   {
      int m_width{};
      int m_height{};
      int m_bpp{};
      [[nodiscard]] constexpr auto get_byte_count() const -> int
      {
         return m_width * m_height * m_bpp;
      }
   };
}


template<std::floating_point T>
constexpr auto sfn::srgb_to_linear_fp(const T x) -> T
{
   if (x <= static_cast<T>(0.04045))
      return x / static_cast<T>(12.92);
   return s9w::pow((x + static_cast<T>(0.055)) / static_cast<T>(1.055), static_cast<T>(2.4));
}


auto constexpr sfn::srgb_to_linear_ui8(const uint8_t in) -> uint8_t
{
   return static_cast<uint8_t>(s9w::round(srgb_to_linear_fp(in / 255.0) * 255.0));
}


template <typename T, typename pred_type>
auto sfn::find_obj(
   const std::vector<T>& vec,
   const pred_type& pred,
   const std::string& error_msg
) -> const T&
{
   const auto it = std::ranges::find_if(vec, pred);
   sfn_assert(it != std::cend(vec), error_msg);
   return *it;
}


constexpr auto sfn::operator==(const id a, const id b) -> bool
{
#ifdef _DEBUG 
   sfn_assert(a.m_id != id::null_id && b.m_id != id::null_id);
#endif
   return a.m_id == b.m_id;
}


constexpr auto sfn::sfn_assert(const bool condition, const std::string& msg) -> void
{
   if (condition == true)
   {
      return;
   }
   if (msg.empty() == false)
   {
      printf(std::format("error: {}\n", msg).c_str());
   }

   std::terminate();
}


constexpr auto sfn::sfn_assert(const bool condition) -> void
{
   if (condition == true)
   {
      return;
   }

   std::terminate();
}
