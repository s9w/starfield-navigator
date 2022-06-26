#include <vector>

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

#include <rng.h>

#include "tools.h"
#include "setup.h"
#include "graph.h"

#include <imgui.h>


auto inner_draw(GLFWwindow* window) -> void
{

   // ImGui::ShowDemoWindow();

   {
      normal_imgui_window w("a");
      ImGui::Text("abc");
   }
   {
      normal_imgui_window w("b");
      ImGui::Text("abc");
   }
}

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




// Disable console window in release mode
#if defined(_DEBUG) || defined(SHOW_CONSOLE)
auto main(int /*argc*/, char* /*argv*/) -> int
#else
auto CALLBACK WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/) -> int
#endif
{
   test_djkstra();

   try {
      const engine engine(
         config{
            .res_x = 800, .res_y = 400,
            .opengl_major_version = 4, .opengl_minor_version = 5,
            .vsync = true,
            .window_title = "Starfield navigator"
         },
         inner_draw
      );
      engine.draw_loop();
   }
   catch (const std::exception& e)
   {
      printf(std::format("caught exception: {}\n", e.what()).c_str());
   }

   return 0;
}
