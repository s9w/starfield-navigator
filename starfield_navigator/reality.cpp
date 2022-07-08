#include "reality.h"

#include <fstream>
#include <format>

#include <glm/trigonometric.hpp>


namespace
{
   using namespace sfn;

   [[nodiscard]] auto get_ly_from_parsec(const float pc) -> float
   {
      return 3.26156f * pc;
   }

   // input: "    3.4964  pc  "
   [[nodiscard]] auto get_ly_from_dist_str(const std::string& str) -> float
   {
      std::string buffer = get_trimmed_str(str);
      buffer = get_split_string(buffer, "  ")[0];
      const float pc = std::stof(buffer);
      return get_ly_from_parsec(pc);
   }
   
} // namespace {}


auto sfn::read_real_stars() -> std::vector<real_star>
{
   std::vector<real_star> result;
   result.reserve(1600);

   std::ifstream input("simbad.txt");
   int counter = 1;
   for (std::string line; getline(input, line); )
   {
      const std::string start_str = std::format("{:<4}|", counter);
      const bool relevant = line.starts_with(start_str);
      if(relevant == false)
         continue;
      const std::vector<std::string> split = get_split_string(line, "|");
      const std::string identifier = get_trimmed_str(split[1]);
      const galactic_coord gc{
         .m_l = glm::radians(std::stof(get_split_string(split[3], " ")[0])),
         .m_b = glm::radians(std::stof(get_split_string(split[3], " ")[1])),
         .m_dist = get_ly_from_dist_str(split[6])
      };
      const int bib_count = std::stoi(get_trimmed_str(split[4]));

      result.push_back(
         real_star{
            .m_identifier = identifier,
            .m_proper_name = "TODO",
            .m_pos = gc,
            // .m_position = gc.get_cartesian(),
            .m_reference_count = bib_count
         }
      );

      counter++;
   }
   const auto pred = [](const real_star& a, const real_star& b)
   {
      return a.m_pos.m_dist < b.m_pos.m_dist;
   };
   std::ranges::sort(result, pred);

   return result;
}
