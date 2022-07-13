#pragma once


#include <chrono>
#include <unordered_map>
#include <span>

#include <s9w_core.h>

#include <fmt/format.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace sfn
{

   // l and b in radians
   struct galactic_coord {
      float m_l;
      float m_b;
      float m_dist;

      [[nodiscard]] auto get_cartesian() const -> glm::vec3;
   };
   [[nodiscard]] auto get_galactic(const glm::vec3& cartesian) -> galactic_coord;

   struct bb_3D
   {
      glm::vec3 m_min{};
      glm::vec3 m_max{};

      [[nodiscard]] constexpr auto get_size() const -> glm::vec3
      {
         return m_max - m_min;
      }
   };

   template<typename T>
   [[nodiscard]] auto get_bb(const std::vector<T>& vertices, const auto& pred) -> bb_3D;
   template<typename T>
   [[nodiscard]] auto get_bb(const std::vector<T>& vertices) -> bb_3D;

   [[nodiscard]] constexpr auto c4d_convert(const glm::vec3& in) -> glm::vec3
   {
      return glm::vec3{ in[0], in[2], in[1] };
   }

   template<typename T>
   auto as_bytes(const std::vector<T>& vec) -> auto
   {
      return std::as_bytes(std::span{ vec });
   }

   template<typename T>
   auto as_bytes(const T& object) -> auto
   {
      return std::as_bytes(std::span{ &object, 1 });
   }

   [[nodiscard]] auto get_split_string( std::string source, const std::string& delim) -> std::vector<std::string>;
   [[nodiscard]] auto get_trimmed_str(const std::string& str, const std::string& to_trim = " ") -> std::string;
   [[nodiscard]] auto apply_trafo(
      const glm::mat4& trafo,
      const glm::vec3& pos
   ) -> glm::vec3
   ;

   struct timer{
      std::chrono::high_resolution_clock::time_point m_t0;
      explicit timer()
         : m_t0(std::chrono::high_resolution_clock::now())
      {}

      ~timer()
      {
         const auto t1 = std::chrono::high_resolution_clock::now();

         using dbl_ms = std::chrono::duration<double, std::milli>;
         const dbl_ms duration = dbl_ms(t1 - m_t0);
         fmt::print("timer: {} ms\n", static_cast<int>(duration.count()));
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

   struct id_hash_callable {
      [[nodiscard]] constexpr auto operator()(const id& p) const -> size_t {
         return p.m_id;
      }
   };

   [[nodiscard]] auto get_average(const std::vector<float>& vec) -> float;
   [[nodiscard]] auto get_aad(const std::vector<float>& vec) -> float;

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
      fmt::print("error: {}\n", msg);
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


template<typename T>
auto sfn::get_bb(
   const std::vector<T>& vertices,
   const auto& pred
) -> bb_3D
{
   bool init = false;
   glm::vec3 min{};
   glm::vec3 max{};
   for (const T& vertex : vertices)
   {
      if (pred(vertex) == false)
         continue;
      if(init == false)
      {
         min = vertex.m_position;
         max = vertex.m_position;
         init = true;
      }
      else
      {
         min = glm::min(min, vertex.m_position);
         max = glm::max(max, vertex.m_position);
      }
   }
   return bb_3D{ .m_min = min, .m_max = max };
}


template<typename T>
auto sfn::get_bb(
   const std::vector<T>& vertices
) -> bb_3D
{
   const auto always_true = []([[maybe_unused]] const T& in)
   {
      return true;
   };
   return get_bb(vertices, always_true);
}
