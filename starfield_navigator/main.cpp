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

#pragma warning(push, 0)
#include <glm/trigonometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#pragma warning(pop)

#include "engine.h"
#include "graph.h"
#include "setup.h"
#include "universe_creation.h"


#include <glad/gl.h>
#include <GLFW/glfw3.h> // after glad



// Disable console window in release mode
#if defined(_DEBUG) || defined(SHOW_CONSOLE)
auto main(int /*argc*/, char* /*argv*/) -> int
#else
auto CALLBACK WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/) -> int
#endif
{
   using namespace sfn;

   config cfg{
         .res_x = 1280, .res_y = 720,
         .opengl_major_version = 4, .opengl_minor_version = 5,
         .vsync = true,
         .window_title = fmt::format("Starfield navigator {}", sfn_version_string)
   };
   std::unique_ptr<graphics_context> gc = std::make_unique<graphics_context>(cfg);
   setup_imgui_fonts();

   universe_creator creator;
   creator_result result = 0.0f;
   while (std::holds_alternative<float>(result) && glfwWindowShouldClose(gc->m_window_wrapper.m_window) == false)
   {
      glClear(GL_COLOR_BUFFER_BIT);
      gc->m_imgui_context.frame_begin();

      result = creator.get();
      if (std::holds_alternative<float>(result))
      {
         const glm::ivec2 window_size{ cfg.res_x, cfg.res_y };
         const glm::ivec2 dialog_size = window_size / 2;
         const glm::ivec2 offset = (window_size - dialog_size) / 2;
         normal_imgui_window w(offset, dialog_size, "Loading");
         ImGui::Text("Aligning reconstructed positions to star catalog...");
         ImGui::ProgressBar(std::get<float>(result));
      }

      gc->m_imgui_context.frame_end();
      glfwSwapBuffers(window_wrapper::m_window);
      glfwPollEvents();
   }

   if (glfwWindowShouldClose(window_wrapper::m_window))
   {
      return 0;
   }

   engine engine(
      cfg,
      std::move(gc),
      std::move(std::get<universe>(result))
   );
   engine.draw_loop();

   return 0;
}

