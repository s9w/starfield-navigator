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

namespace
{
   using namespace sfn;

   auto print_system(
      const universe& universe,
      const int i,
      const bool* selected_target
   ) -> std::optional<int>
   {
      if (universe.m_systems[i].m_info_quality == info_quality::unknown)
         ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.7f));
      else if (universe.m_systems[i].m_info_quality == info_quality::speculation)
         ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(0.3f, 1.0f, 1.0f));

      std::optional<int> return_selection;
      if (selected_target == nullptr)
         ImGui::Text(universe.m_systems[i].m_name.c_str());
      else
      {
         if (ImGui::Selectable(universe.m_systems[i].m_name.c_str(), *selected_target))
         {
            return_selection = i;
         }
      }
      if (universe.m_systems[i].m_info_quality != info_quality::confirmed)
         ImGui::PopStyleColor();
      return return_selection;
   }

}

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


sfn::engine::engine(const config& config, universe&& universe)
   : m_window_wrapper(config)
   , m_imgui_context(config, m_window_wrapper.m_window)
   , m_universe(std::move(universe))
   , m_buffers2(mvp_type::size)
   , m_shader_stars("star_shader")
   , m_framebuffers(m_textures)
{
   if (engine_ptr != nullptr)
      std::terminate();
   engine_ptr = this;
   glfwSetFramebufferSizeCallback(get_window(), engine::static_resize_callback);

   std::vector<segment_type> buffer_layout;
   buffer_layout.emplace_back(ubo_segment(sizeof(mvp_type), "ubo_mvp"));
   const std::vector<id> segment_ids = m_buffers2.create_buffer(std::move(buffer_layout), usage_pattern::dynamic_draw);
   m_mvp_ubo_id = segment_ids[0];
   m_binding_point_man.add(m_mvp_ubo_id);
   m_main_fb = m_framebuffers.get_efault_fb();
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



auto sfn::engine::draw_list() -> void
{
   static ImGuiTextFilter filter;
   filter.Draw("filter");

   if (ImGui::BeginTable("##table_selector", 2, ImGuiTableFlags_RowBg| ImGuiTableFlags_ScrollY))
   {
      ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
      ImGui::TableSetupColumn("idx", ImGuiTableColumnFlags_WidthFixed, 30.0f);
      ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_None);
      ImGui::TableHeadersRow();

      for (int i = 0; i < m_universe.m_systems.size(); i++)
      {
         if (filter.PassFilter(m_universe.m_systems[i].m_name.c_str()) == false)
          continue;
         const bool is_selected = (m_list_selection == i);

         ImGui::TableNextRow();
         ImGui::TableSetColumnIndex(0);
         ImGui::Text(std::format("{}", i).c_str());
         ImGui::TableSetColumnIndex(1);

         if(const auto x = print_system(m_universe, i, &is_selected); x.has_value())
         {
            m_list_selection = *x;
         }
      }
      ImGui::EndTable();
   }
}


auto engine::gui_closest_stars(const graph& starfield_graph) -> void
{
   static int selection = 14;
   if (ImGui::Button("Take from selector -> ##vicinity"))
   {
      selection = m_list_selection;
   }
   ImGui::SameLine();
   ImGui::Text(std::format("{}", m_universe.m_systems[selection].m_name).c_str());

   // TODO why does this even use the graph and not just the universe?
   const std::vector<int> closest = starfield_graph.get_closest(selection);


   if (ImGui::BeginTable("##table_closest", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
   {
      ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
      ImGui::TableSetupColumn("Dist [LY]", ImGuiTableColumnFlags_WidthFixed, 80.0f);
      ImGui::TableSetupColumn("System", ImGuiTableColumnFlags_None);

      ImGui::TableHeadersRow();
      for (int i = 1; i < std::ssize(closest); ++i)
      {
         ImGui::TableNextRow();

         ImGui::TableSetColumnIndex(0);
         const float dist = glm::distance(
            m_universe.m_systems[selection].m_position,
            m_universe.m_systems[closest[i]].m_position
         );
         ImGui::Text(std::format("{:>5.1f}", dist).c_str());

         ImGui::TableSetColumnIndex(1);
         print_system(m_universe, closest[i], nullptr);
      }

      ImGui::EndTable();
   }
}


auto engine::gui_plotter(graph& starfield_graph) -> void
{
   static std::optional<jump_path> path;

   static bool first_plot = true;
   bool course_changed = first_plot;
   first_plot = false;

   if (ImGui::Button("Take from selector -> ##src"))
   {
      course_changed = true;
      m_source_index = m_list_selection;
   }
   ImGui::SameLine();
   ImGui::Text(std::format("Source: {}", m_universe.m_systems[m_source_index].m_name).c_str());

   if (ImGui::Button("Take from selector -> ##dst"))
   {
      course_changed = true;
      m_destination_index = m_list_selection;
   }
   ImGui::SameLine();
   ImGui::Text(std::format("Destination: {}", m_universe.m_systems[m_destination_index].m_name).c_str());

   ImGui::Text(std::format(
      "Total distance: {:.1f} LY",
      m_universe.get_distance(m_source_index, m_destination_index)
   ).c_str());

   static float slider_min = 0.0f;
   static float slider_max = 100.0f;
   if (course_changed)
   {
      slider_min = get_min_jump_dist(m_universe, m_source_index, m_destination_index) + 0.001f;
      slider_max = m_universe.get_distance(m_source_index, m_destination_index) + 0.001f;
      m_jump_range = 0.5f * (slider_min + slider_max);
   }

   course_changed |= ImGui::SliderFloat("jump range", &m_jump_range, slider_min, slider_max);

   static std::vector<std::string> path_strings;
   // Graph and path update
   if (course_changed)
   {
      starfield_graph = graph(m_universe, m_jump_range);
      path = starfield_graph.get_jump_path(m_source_index, m_destination_index);

      if (path.has_value())
      {
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
         path_strings.push_back("-----");
         path_strings.push_back(std::format("Travelled distance: {:.1f} LY", travelled_distance));
      }
      int end = 0;
   }

   // Path display
   if (path.has_value() == false)
   {
      ImGui::Text("Jump range not large enough\n");
   }
   else
   {
      const auto avail = ImGui::GetContentRegionAvail();
      if (ImGui::BeginListBox("##result", ImVec2(-FLT_MIN, avail.y)))
      {
         for (const std::string& path_string : path_strings)
            ImGui::Text(path_string.c_str());
         ImGui::EndListBox();
      }
   }
}


auto sfn::engine::draw_fun() -> void
{
   static graph starfield_graph;
   
   

   if(ImGui::GetFrameCount() == 1)
   {
      starfield_graph = graph(m_universe, m_jump_range);
   }

   {
      normal_imgui_window w("System selector");
      draw_list();
   }

   {
      normal_imgui_window w("Tools");
      
      if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None))
      {
         ImGui::PushStyleColor(ImGuiCol_Tab, (ImVec4)ImColor::HSV(0.0f, 0.5f, 0.6f));
         ImGui::PushStyleColor(ImGuiCol_TabHovered, (ImVec4)ImColor::HSV(0.0f, 0.5f, 0.8f));
         ImGui::PushStyleColor(ImGuiCol_TabActive, (ImVec4)ImColor::HSV(0.0f, 0.5f, 0.9f));
         if (ImGui::BeginTabItem("Closest stars"))
         {
            
            gui_closest_stars(starfield_graph);
            ImGui::EndTabItem();
         }
         
         if (ImGui::BeginTabItem("Jump calculations"))
         {
            gui_plotter(starfield_graph);
            ImGui::EndTabItem();
         }
         ImGui::PopStyleColor(3);

         ImGui::EndTabBar();
      }
      
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
