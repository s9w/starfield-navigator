#include <vector>
#include <fstream>

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <imgui.h>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <rng.h>

#include "tools.h"
// #include "setup.h"
#include "engine.h"
#include "graph.h"



using namespace sfn;


[[nodiscard]] constexpr auto c4d_convert(const glm::vec3& in) -> glm::vec3
{
   return glm::vec3{ in[0], -in[2], in[1] };
}



[[nodiscard]] auto get_split_string(
   std::string source,
   const std::string& delim
)->std::vector<std::string>
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

auto get_starfield_universe() -> universe
{
   universe starfield_universe;
   std::ifstream input("system_data.txt");
   for (std::string line; getline(input, line); )
   {
      const std::vector<std::string> split = get_split_string(line, "; ");
      const std::string name = split[0];
      if(name.starts_with("Solved Camera"))
      {
         continue;
      }
      
      const float x = static_cast<float>(std::stod(split[1]));
      const float y = static_cast<float>(std::stod(split[2]));
      const float z = static_cast<float>(std::stod(split[3]));
      starfield_universe.m_systems.emplace_back(c4d_convert(glm::vec3{ x, y, z }), name);
   }

   // sort from left to right
   const auto pred = [](const sfn::system& a, const sfn::system& b){
      return a.m_position.x < b.m_position.x;
   };
   std::ranges::sort(starfield_universe.m_systems, pred);

   // Calibration with Porrima
   {
      const float measured_porrima_dist = glm::distance(starfield_universe.get_position_by_name("ALPHA CENTAURI"), starfield_universe.get_position_by_name("PORRIMA"));
      constexpr float real_porrima_dist = 38.11f;
      const float correction_factor = real_porrima_dist / measured_porrima_dist;
      for (sfn::system& sys : starfield_universe.m_systems)
      {
         sys.m_position *= correction_factor;
      }
   }

   auto print_sol_deviation = [&](const std::string& sys_name, const float reference_dist)
   {
      const float dist = glm::distance(
         starfield_universe.get_position_by_name("SOL"),
         starfield_universe.get_position_by_name(sys_name)
      );
      const float relative_deviation = 100.0f * std::abs(reference_dist - dist) / reference_dist;
      printf(std::format("Deviation for {} Distance: {:.2f} %%\n", sys_name, relative_deviation).c_str());
   };
   print_sol_deviation("ALPHA CENTAURI", 4.367f);
   print_sol_deviation("SIRIUS", 8.611f);
   print_sol_deviation("ALTAIR", 16.73f);

   auto dist_report = [&](const std::string& name_a, const std::string& name_b){
      const float dist = glm::distance(
         starfield_universe.get_position_by_name(name_a),
         starfield_universe.get_position_by_name(name_b)
      );
      printf(std::format("dist {}<->{}: {:.2f} LY\n", name_a, name_b, dist).c_str());
   };
   dist_report("User 64", "User 65");
   dist_report("User 62", "User 63");

   // {
   //    const float sufficient_jump_range = get_absolute_min_jump_range(starfield_universe);
   //    printf(std::format("sufficient_jump_range: {:.1f}\n", sufficient_jump_range).c_str());
   // }

   {
      const auto closest = get_closest_distances_for_all(starfield_universe);
      printf(std::format("avg of closest: {:.2f}\n", get_average(closest)).c_str());
      printf(std::format("max of closest: {:.2f}\n", *std::ranges::max_element(closest)).c_str());
   }

   // timer t;
   // float ddd;
   // for(int i=0; i<1000; ++i)
   //    ddd = get_min_jump_dist(starfield_universe, "SOL", "PORRIMA");
   // printf(std::format("ddd: {}", ddd).c_str());

   return starfield_universe;
}

auto main(int /*argc*/, char* /*argv*/) -> int

// // Disable console window in release mode
// #if defined(_DEBUG) || defined(SHOW_CONSOLE)
// auto main(int /*argc*/, char* /*argv*/) -> int
// #else
// auto CALLBACK WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/) -> int
// #endif
{

   try {
      engine engine(
         config{
            .res_x = 1280, .res_y = 720,
            .opengl_major_version = 4, .opengl_minor_version = 5,
            .vsync = true,
            .window_title = "Starfield navigator"
         },
         get_starfield_universe()
      );
      engine.draw_loop();
   }
   catch (const std::exception& e)
   {
      printf(std::format("caught exception: {}\n", e.what()).c_str());
   }

   return 0;
}
