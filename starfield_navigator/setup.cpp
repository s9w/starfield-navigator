#include "setup.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h> // after glad
// #define GLFW_EXPOSE_NATIVE_WIN32
// #include <GLFW/glfw3native.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <glm/geometric.hpp>

#include <optional>


sfn::glfw_wrapper::glfw_wrapper()
{
   if (glfwInit() == GLFW_FALSE)
      throw std::exception{ "glfwInit() error" };
}

sfn::glfw_wrapper::~glfw_wrapper()
{
   glfwTerminate();
}

sfn::window_wrapper::window_wrapper(const config& config)
{
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config.opengl_major_version);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, config.opengl_minor_version);
   constexpr GLFWmonitor* monitor = nullptr;
   constexpr GLFWwindow* shared_window = nullptr;
   m_window = glfwCreateWindow(config.res_x, config.res_y, config.window_title.c_str(), monitor, shared_window);
   if (m_window == nullptr)
   {
      throw std::exception{ "glfwCreateWindow error" };
   }
   glfwMakeContextCurrent(m_window);

   if (config.vsync)
      glfwSwapInterval(1);
}


sfn::window_wrapper::~window_wrapper()
{
   if (m_window != nullptr)
   {
      glfwDestroyWindow(m_window);
   }
}


sfn::glad_wrapper::glad_wrapper()
{
   const int glad_version = gladLoadGL(glfwGetProcAddress);
   if (glad_version == 0)
      throw std::exception{ "gladLoadGL error" };
}


sfn::imgui_context::imgui_context(const config& config, GLFWwindow* window)
{
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGui_ImplGlfw_InitForOpenGL(window, true);
   const std::string glsl_version = std::format("#version {}{}0", config.opengl_major_version, config.opengl_minor_version);
   ImGui_ImplOpenGL3_Init(glsl_version.c_str());
}


sfn::imgui_context::~imgui_context()
{
   ImGui_ImplOpenGL3_Shutdown();
   ImGui_ImplGlfw_Shutdown();
   ImGui::DestroyContext();
}


auto sfn::imgui_context::frame_begin() const -> void
{
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();
}


auto sfn::imgui_context::frame_end() const -> void
{
   ImGui::Render();
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


sfn::engine::engine(const config& config, const universe& universe)
   : m_window_wrapper(config)
   , m_imgui_context(config, m_window_wrapper.m_window)
   , m_universe(universe)
{
   if (engine_ptr != nullptr)
      std::terminate();
   engine_ptr = this;
   glfwSetFramebufferSizeCallback(get_window(), engine::static_resize_callback);
}


auto sfn::engine::get_window() const -> GLFWwindow*
{
   return m_window_wrapper.m_window;
}


auto sfn::engine::static_resize_callback(GLFWwindow* window, int new_width, int new_height) -> void
{
   engine_ptr->resize_callback(window, new_width, new_height);
}


auto sfn::engine::resize_callback(
   [[maybe_unused]] GLFWwindow* window,
   [[maybe_unused]] int new_width,
   [[maybe_unused]] int new_height
) -> void
{
   draw_frame();
}


auto sfn::engine::draw_frame() -> void
{
   glClear(GL_COLOR_BUFFER_BIT);
   m_imgui_context.frame_begin();

   this->draw_fun();

   m_imgui_context.frame_end();
   glfwSwapBuffers(this->get_window());
   glfwPollEvents();
}


auto sfn::engine::draw_loop() -> void
{
   while (glfwWindowShouldClose(this->get_window()) == false)
   {
      this->draw_frame();
   }
}


auto sfn::engine::draw_list(
   int& selected_index,
   const std::string& imgui_id_base,
   ImGuiTextFilter& filter
) const -> bool
{
   bool changed = false;
   filter.Draw((imgui_id_base+"_filter").c_str());
   if (ImGui::BeginListBox((imgui_id_base + "_list").c_str(), ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
   {
      for (int i = 0; i < m_universe.m_systems.size(); i++)
      {
         if (filter.PassFilter(m_universe.m_systems[i].m_name.c_str()) == false)
            continue;
         const bool is_selected = (selected_index == i);
         if (ImGui::Selectable(m_universe.m_systems[i].m_name.c_str(), is_selected))
         {
            selected_index = i;
            changed = true;
         }

         // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
         if (is_selected)
            ImGui::SetItemDefaultFocus();
      }
      ImGui::EndListBox();
   }
   return changed;
}


auto sfn::engine::draw_fun() -> void
{
   normal_imgui_window w("Plotter");

   bool course_changed = false;
   static int src_index = 0;
   static ImGuiTextFilter src_filter;
   static int dest_index = 0;
   static ImGuiTextFilter dst_filter;

   constexpr bool child_border = false;
   const float width_avail = ImGui::GetContentRegionAvail().x;
   const float height = (5+3) * ImGui::GetTextLineHeightWithSpacing();
   ImGui::BeginChild("ChildL", ImVec2(width_avail * 0.5f, height), child_border);
   course_changed |= draw_list(src_index, "##selector_left", src_filter);
   ImGui::EndChild();
   ImGui::SameLine();
   ImGui::BeginChild("ChildR", ImVec2(width_avail * 0.5f, height), child_border);
   course_changed |= draw_list(dest_index, "##selector_right", dst_filter);
   ImGui::EndChild();

   course_changed |= ImGui::SliderFloat("jump range", &m_jump_range, 0.0f, 100.0f);

   static graph starfield_graph;
   static std::optional<jump_path> path;
   static std::vector<std::string> path_strings;

   if (course_changed)
   {
      starfield_graph = graph(m_universe, m_jump_range);
      path = starfield_graph.get_jump_path(m_universe.m_systems[src_index].m_name, m_universe.m_systems[dest_index].m_name);

      if (path.has_value())
      {
         starfield_graph.print_path(*path);
         path_strings.clear();
         float travelled_distance = 0.0f;
         for (int i = 0; i < path->m_stops.size() - 1; ++i)
         {
            const int this_stop_system = path->m_stops[i];
            const int next_stop_system = path->m_stops[i + 1];
            const float dist = glm::distance(starfield_graph.m_nodes[this_stop_system].m_position, starfield_graph.m_nodes[next_stop_system].m_position);
            travelled_distance += dist;

            path_strings.push_back(std::format(
               "Jump {}: {} to {}. Distance: {:.1f} LY\n",
               i,
               starfield_graph.m_nodes[this_stop_system].m_name,
               starfield_graph.m_nodes[next_stop_system].m_name,
               dist
            ));
         }
      }
   }

   if (path.has_value() == false)
   {
      ImGui::Text("Jump range not large enough\n");
   }
   else
   {
      ImGui::BeginListBox("##result", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing()));
      for (const std::string& path_string : path_strings)
         ImGui::Text(path_string.c_str());
      ImGui::EndListBox();
   }
   

   // ImGui::ShowDemoWindow();
}


sfn::normal_imgui_window::normal_imgui_window(const char* name, const ImGuiWindowFlags extra_flags)
{
   ImGui::Begin(name, nullptr, extra_flags);
}


sfn::normal_imgui_window::~normal_imgui_window()
{
   ImGui::End();
}


sfn::single_imgui_window::single_imgui_window(const ImGuiWindowFlags extra_flags)
{
   ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
   ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
   ImGuiWindowFlags flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
   flags |= extra_flags;
   ImGui::Begin(" ", nullptr, flags);
}


sfn::single_imgui_window::~single_imgui_window()
{
   ImGui::End();
}
