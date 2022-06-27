#include <vector>
#include <fstream>
#include <regex>

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

#include <rng.h>

#include "tools.h"
#include "setup.h"
#include "graph.h"

#include <imgui.h>

using namespace sfn;



auto test_djkstra() -> void
{
   // example from https://www.youtube.com/watch?v=pVfj6mxhdMw
   graph g;
   g.m_nodes.push_back(node{ .m_name = "A" });
   g.m_nodes.push_back(node{ .m_name = "B" });
   g.m_nodes.push_back(node{ .m_name = "C" });
   g.m_nodes.push_back(node{ .m_name = "D" });
   g.m_nodes.push_back(node{ .m_name = "E" });

   g.add_connection("A", "B", 6.0f);
   g.add_connection("A", "D", 1.0f);
   g.add_connection("B", "D", 2.0f);
   g.add_connection("B", "E", 2.0f);
   g.add_connection("D", "E", 1.0f);
   g.add_connection("B", "C", 5.0f);
   g.add_connection("E", "C", 5.0f);

   const shortest_path_tree dijkstra = g.get_dijkstra(g.get_node_index_by_name("A"));

   shortest_path_tree expected(0, 5);
   expected.m_entries[0] = shortest_path{ .m_shortest_distance = 0, .m_previous_vertex_index = -1 };
   expected.m_entries[1] = shortest_path{ .m_shortest_distance = 3, .m_previous_vertex_index = 3 };
   expected.m_entries[2] = shortest_path{ .m_shortest_distance = 7, .m_previous_vertex_index = 4 };
   expected.m_entries[3] = shortest_path{ .m_shortest_distance = 1, .m_previous_vertex_index = 0 };
   expected.m_entries[4] = shortest_path{ .m_shortest_distance = 2, .m_previous_vertex_index = 3 };
   if (dijkstra != expected)
      std::terminate();
}

[[nodiscard]] constexpr auto c4d_convert(const glm::vec3& in) -> glm::vec3
{
   return glm::vec3{ in[0], -in[2], in[1] };
}

auto get_starfield_universe() -> universe
{
   universe starfield_universe;
   std::ifstream input("c4d.txt");
   const std::regex regex(R"(name: (.*?), position: Vector\((-?\d*.\d*), (-?\d*.\d*), (-?\d*.\d*)\))");
   for (std::string line; getline(input, line); )
   {
      std::smatch color_match;
      if (std::regex_search(line, color_match, regex) == false)
         std::terminate();
      const std::string name = color_match[1];
      if(name.contains("Camera"))
      {
         continue;
      }
      
      const float x = static_cast<float>(std::stod(color_match[2]));
      const float y = static_cast<float>(std::stod(color_match[3]));
      const float z = static_cast<float>(std::stod(color_match[4]));
      starfield_universe.m_systems.emplace_back(c4d_convert(glm::vec3{ x, y, z }), name);
   }

   // sort from left to right
   const auto pred = [](const sfn::system& a, const sfn::system& b){
      return a.m_position.x < b.m_position.x;
   };
   std::ranges::sort(starfield_universe.m_systems, pred);

   // rename unknowns
   for(sfn::system& sys : starfield_universe.m_systems)
   {
      if (sys.m_name.contains("User"))
      {
         static int unknown_count = 0;
         sys.m_name = std::format("UNKNOWN {}", unknown_count++);
      }
   }

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

   const float binary_distance = glm::distance(
      starfield_universe.get_position_by_name("binaryA0"),
      starfield_universe.get_position_by_name("binaryA1")
   );

   {
      // Correctness check with Alpha Centauri
      constexpr float AC_real_distance = 4.367f;
      const float AC_distance = glm::distance(
         starfield_universe.get_position_by_name("SOL"),
         starfield_universe.get_position_by_name("ALPHA CENTAURI")
      );
      const float relative_deviation = 100.0f * std::abs(AC_real_distance - AC_distance) / AC_real_distance;
      printf(std::format("Deviation for Alpha Centauri Distane: {:.1f} %%\n", relative_deviation).c_str());
   }

   return starfield_universe;
}


// Disable console window in release mode
#if defined(_DEBUG) || defined(SHOW_CONSOLE)
auto main(int /*argc*/, char* /*argv*/) -> int
#else
auto CALLBACK WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/) -> int
#endif
{
   test_djkstra();

   

   // const graph starfield_graph(starfield_world, 60);
   // const std::optional<jump_path> path = starfield_graph.get_jump_path("SOL", "PORRIMA");
   // if(path.has_value() == false)
   // {
   //    printf("Jump range not large enough\n");
   //    return 0;
   // }
   // starfield_graph.print_path(*path);

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
