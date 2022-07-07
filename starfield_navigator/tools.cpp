#include "tools.h"

#include <glm/trigonometric.hpp>


auto sfn::galactic_coord::get_cartesian() const -> glm::vec3
{
   const float theta = glm::radians(90.0f) - m_b;
   const float x = m_dist * std::cos(m_l) * std::sin(theta);
   const float y = m_dist * std::sin(m_l) * std::sin(theta);
   const float z = m_dist * std::cos(theta);
   return glm::vec3{ x, y, z };
}


auto sfn::get_galactic(const glm::vec3& cartesian) -> galactic_coord
{
   const float theta = std::atan2(std::sqrt(cartesian[0]* cartesian[0]+ cartesian[1]* cartesian[1]), cartesian[2]);
   galactic_coord result{
      .m_l = std::atan2(cartesian[1], cartesian[0]),
      .m_b = glm::radians(90.0f) - theta,
      .m_dist = std::sqrt(cartesian[0]* cartesian[0] + cartesian[1]* cartesian[1] + cartesian[2]* cartesian[2])
   };
   constexpr float two_pi = 2.0f * std::numbers::pi_v<float>;
   result.m_l = std::fmod(result.m_l + two_pi, two_pi);
   result.m_b = std::fmod(result.m_b + two_pi, two_pi);
   return result;
}


auto sfn::get_split_string(
   std::string source,
   const std::string& delim
) -> std::vector<std::string>
{
   if (delim.empty())
      std::terminate();

   std::vector<std::string> parts;
   for (auto pos = source.find(delim);
        pos != std::string::npos;
        pos = source.find(delim))
   {
      parts.emplace_back(source.substr(0, pos));
      source.erase(0, pos + delim.length());
   }
   parts.emplace_back(source);
   return parts;
}


auto sfn::get_trimmed_str(
   const std::string& str,
   const std::string& to_trim
) -> std::string
{
   const auto lpos = str.find_first_not_of(to_trim);
   if (lpos == std::string::npos)
      return str;
   const auto rpos = str.find_last_not_of(to_trim);
   return str.substr(lpos, rpos - lpos + 1);
}


auto sfn::apply_trafo(
   const glm::mat4& trafo,
   const glm::vec3& pos
) -> glm::vec3
{
   glm::vec4 result = trafo * glm::vec4{ pos, 1.0f };
   result /= result[3];
   return glm::vec3{ result };
}


auto sfn::id::create() -> id
{
   static std::atomic<id::underlying_type> next_id_value = 0;

#ifdef _DEBUG
   sfn_assert(
      next_id_value != std::numeric_limits<id::underlying_type>::max(),
      "id type overflowing. increase size!"
   );
#endif

   return id{ explicit_init{}, next_id_value.fetch_add(1)};
}


auto sfn::get_average(const std::vector<float>& vec) -> float
{
   float sum = 0.0f;
   for (const float elem : vec)
      sum += elem;
   return sum / std::size(vec);
}
