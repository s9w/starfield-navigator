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


// Disable console window in release mode
#if defined(_DEBUG) || defined(SHOW_CONSOLE)
auto main(int /*argc*/, char* /*argv*/) -> int
#else
auto CALLBACK WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/) -> int
#endif
{
   // constexpr int n = 80;
   //
   // std::vector<glm::vec3> positions;
   // tools::rng_state rng;
   // for(int i=0; i<n; ++i)
   // {
   //    positions.push_back(
   //       glm::vec3{
   //          rng.get_real(0.0f, 100.0f),
   //          rng.get_real(0.0f, 100.0f),
   //          rng.get_real(0.0f, 100.0f)
   //       }
   //    );
   // }
   //
   // sfn::timer t;
   // double distance_avg = 0.0;
   // for(int i=0; i<n; ++i)
   // {
   //    for (int j = 0; j < n; ++j)
   //    {
   //       distance_avg += glm::distance(positions[i], positions[j]);
   //    }
   // }
   // distance_avg /= n * n;
   // printf(std::format("dist: {}\n", distance_avg).c_str());


   graph g;
   g.m_nodes.push_back(node{.m_name="A"});
   g.m_nodes.push_back(node{.m_name="B"});
   g.m_nodes.push_back(node{.m_name="C"});
   g.m_nodes.push_back(node{.m_name="D"});
   g.m_nodes.push_back(node{.m_name="E"});

   g.add_connection("A", "B", 6.0f);
   g.add_connection("A", "D", 1.0f);
   g.add_connection("B", "D", 2.0f);
   g.add_connection("B", "E", 2.0f);
   g.add_connection("D", "E", 1.0f);
   g.add_connection("B", "C", 5.0f);
   g.add_connection("E", "C", 5.0f);


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
