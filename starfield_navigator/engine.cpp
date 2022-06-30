#include "engine.h"

#include <GLFW/glfw3.h> // after glad

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>

#include "fonts/FontAwesomeSolid.hpp"
#include "fonts/DroidSans.hpp"
#include "fonts/IconsFontAwesome5.h"



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


   auto right_align_text(const std::string& text) -> void
   {
      const auto posX = (ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(text.c_str()).x
         - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
      if (posX > ImGui::GetCursorPosX())
         ImGui::SetCursorPosX(posX);
      ImGui::Text(text.c_str());
   }

   auto get_view_matrix(const glm::vec3& camera_pos) -> glm::mat4
   {
      glm::mat4 view_matrix{ 1.0f };

      view_matrix = glm::rotate(view_matrix, glm::radians(-90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });

      // camera_pos += height * std::tan(view_tilt) * glm::vec3{ 0, -1, 0 };
      view_matrix = glm::translate(view_matrix, -camera_pos);
   
      return view_matrix;
   }

   auto get_projection_matrxi() -> glm::mat4
   {
      constexpr float aspect = 1280.0f / 720.0f;
      const float x_fov = glm::radians(63.0f);
      const float y_fov = x_fov / aspect;
      return glm::perspective(y_fov, aspect, 0.1f, 3000.0f);
   }


   auto get_star_vertex_data(const universe& universe) -> std::vector<position_vertex_data>
   {
      std::vector<position_vertex_data> result;
      result.reserve(universe.m_systems.size());
      for (const sfn::system& sys : universe.m_systems)
      {
         result.push_back(position_vertex_data{ .m_position = sys.m_position });
      }
      return result;
   }

   std::vector<position_vertex_data> star_mesh;
   std::vector<position_vertex_data> screen_rect_mesh = {
      position_vertex_data{.m_position = { -1, -1, 0}},
      position_vertex_data{.m_position = {  1, -1, 0}},
      position_vertex_data{.m_position = { -1,  1, 0}},
      position_vertex_data{.m_position = { -1, -1, 0}},
      position_vertex_data{.m_position = {  1,  1, 0}},
      position_vertex_data{.m_position = { -1,  1, 0}}
   };
   // std::vector<position_vertex_data> connection_mesh;

} // namespace {}


sfn::engine::engine(const config& config, universe&& universe)
   : m_window_wrapper(config)
   , m_imgui_context(config, m_window_wrapper.m_window)
   , m_universe(std::move(universe))
   , m_buffers2(128)
   , m_shader_stars("star_shader")
   , m_framebuffers(m_textures)
{
   {
      ImGuiIO& io = ImGui::GetIO();
      ImFontConfig configBasic;
      ImFontConfig configMerge;
      configMerge.MergeMode = true;

      static const ImWchar rangesBasic[] = {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x03BC, 0x03BC, // micro
        0x03C3, 0x03C3, // small sigma
        0x2013, 0x2013, // en dash
        0x2264, 0x2264, // less-than or equal to
        0,
      };
      static const ImWchar rangesIcons[] = {
          ICON_MIN_FA, ICON_MAX_FA,
          0
      };
      constexpr float normal_font_size = 15.0f;
      constexpr float icon_font_size = 15.0f;

      io.Fonts->Clear();
      io.Fonts->AddFontFromMemoryCompressedTTF(DroidSans_compressed_data, DroidSans_compressed_size, round(normal_font_size), &configBasic, rangesBasic);
      io.Fonts->AddFontFromMemoryCompressedTTF(FontAwesomeSolid_compressed_data, FontAwesomeSolid_compressed_size, round(icon_font_size), &configMerge, rangesIcons);

      // ImGui_ImplOpenGL3_DestroyFontsTexture();
      // ImGui_ImplOpenGL3_CreateFontsTexture();
   }

   if (engine_ptr != nullptr)
      std::terminate();
   engine_ptr = this;
   glfwSetFramebufferSizeCallback(get_window(), engine::static_resize_callback);

   std::vector<segment_type> buffer_layout;
   buffer_layout.emplace_back(ubo_segment(sizeof(mvp_type), "ubo_mvp"));

   star_mesh = get_star_vertex_data(m_universe);
   buffer_layout.emplace_back(get_soa_vbo_segment(star_mesh));
   buffer_layout.emplace_back(get_soa_vbo_segment(screen_rect_mesh));
   buffer_layout.emplace_back(get_soa_vbo_segment<position_vertex_data>(100*100));
   const std::vector<id> segment_ids = m_buffers2.create_buffer(std::move(buffer_layout), usage_pattern::dynamic_draw);
   m_mvp_ubo_id = segment_ids[0];
   m_star_vbo_id = segment_ids[1];
   m_screen_rect_vbo_id = segment_ids[1];
   m_connections_vbo_id = segment_ids[2];

   m_binding_point_man.add(m_mvp_ubo_id);
   m_main_fb = m_framebuffers.get_efault_fb();
   m_binding_point_man.add(m_mvp_ubo_id);

   const buffer& buffer_ref = m_buffers2.get_single_buffer_ref();
   bind_ubo("ubo_mvp", buffer_ref, m_mvp_ubo_id, m_shader_stars);

   m_vao_stars.emplace(m_buffers2, m_star_vbo_id, m_shader_stars);
   m_vao_screen_rect.emplace(m_buffers2, m_screen_rect_vbo_id, m_shader_stars);
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
   // glClear(GL_COLOR_BUFFER_BIT);
   m_framebuffers.bind_fb(m_main_fb, fb_target::full);
   m_framebuffers.clear_depth(m_main_fb);
   constexpr glm::vec3 bg_color{}; //{ 0.01f, 0.156f, 0.139f };
   m_framebuffers.clear_color(m_main_fb, bg_color, 0);

   m_imgui_context.frame_begin();

   // calculate things
   update_mvp_member(); // TODO

   // upload things
   gpu_upload();

   // render things
   m_vao_stars->bind();
   m_shader_stars.use();
   glViewport(0, 0, 1280, 720);
   glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
   // glPointSize(5);
   glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(m_universe.m_systems.size()));

   glEnable(GL_LINE_SMOOTH);
   glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
   glDrawArrays(GL_LINES, 0, 4);

   // GUI
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
   filter.Draw((const char*)ICON_FA_SEARCH " Filter");

   if (ImGui::BeginTable("##table_selector", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
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
         right_align_text(std::format("{}", i));
         // ImGui::Text(std::format("{:0>2}", i).c_str());
         ImGui::TableSetColumnIndex(1);

         if (const auto x = print_system(m_universe, i, &is_selected); x.has_value())
         {
            m_list_selection = *x;
         }
      }
      ImGui::EndTable();
   }
}


auto sfn::engine::gui_closest_stars() -> void
{
   static int selection = 14;

   ImGui::AlignTextToFramePadding();
   ImGui::Text(std::format("Closest systems around: {}", m_universe.m_systems[selection].m_name).c_str());
   ImGui::SameLine();
   ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
   if (ImGui::Button((const char*)ICON_FA_MAP_MARKER_ALT, ImVec2(30, 20)))
   {
      selection = m_list_selection;
   }
   ImGui::PopStyleVar();

   // TODO why does this even use the graph and not just the universe?
   const std::vector<int> closest = m_universe.get_closest(selection);


   if (ImGui::BeginTable("##table_closest", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
   {
      ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
      ImGui::TableSetupColumn("Dist [LY]", ImGuiTableColumnFlags_WidthFixed, 60.0f);
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
         right_align_text(std::format("{:.1f}", dist));

         ImGui::TableSetColumnIndex(1);
         print_system(m_universe, closest[i], nullptr);
      }

      ImGui::EndTable();
   }
}


auto sfn::engine::gui_plotter(graph& starfield_graph) -> void
{
   static std::optional<jump_path> path;

   static bool first_plot = true;
   bool course_changed = first_plot;
   first_plot = false;


   ImGui::AlignTextToFramePadding();
   ImGui::Text(std::format("Source: {}", m_universe.m_systems[m_source_index].m_name).c_str());
   ImGui::SameLine();
   ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
   if (ImGui::Button((const char*)ICON_FA_MAP_MARKER_ALT "##src", ImVec2(30, 20)))
   {
      course_changed = true;
      m_source_index = m_list_selection;
   }
   ImGui::PopStyleVar();


   ImGui::AlignTextToFramePadding();
   ImGui::Text(std::format("Destination: {}", m_universe.m_systems[m_destination_index].m_name).c_str());
   ImGui::SameLine();
   ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
   if (ImGui::Button((const char*)ICON_FA_MAP_MARKER_ALT "##dst", ImVec2(30, 20)))
   {
      course_changed = true;
      m_destination_index = m_list_selection;
   }
   ImGui::PopStyleVar();



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


auto engine::bind_ubo(
   const std::string& name,
   const buffer& buffer_ref,
   const id segment_id,
   const shader_program& shader
) const -> void
{
   const GLuint block_index = glGetUniformBlockIndex(shader.m_opengl_id, name.c_str());
   const int binding_point = m_binding_point_man.get_point(segment_id);
   glUniformBlockBinding(shader.m_opengl_id, block_index, binding_point);
   glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, buffer_ref.m_buffer_opengl_id);

   const auto segment_size = buffer_ref.get_segment_size(segment_id);
   const auto offset_in_buffer = buffer_ref.get_segment_offset(segment_id);
   glBindBufferRange(GL_UNIFORM_BUFFER, binding_point, buffer_ref.m_buffer_opengl_id, offset_in_buffer, segment_size);
}



auto engine::gpu_upload() -> void
{
   m_buffers2.upload_ubo(m_mvp_ubo_id, as_bytes(m_current_mvp));
   m_buffers2.upload_vbo(m_star_vbo_id, as_bytes(star_mesh));
   m_buffers2.upload_vbo(m_screen_rect_vbo_id, as_bytes(screen_rect_mesh));
}

auto engine::update_mvp_member() -> void
{
   m_current_mvp.m_cam_pos = m_camera_pos;
   m_current_mvp.m_projection = get_projection_matrxi();
   m_current_mvp.m_view = get_view_matrix(m_camera_pos);
}



auto sfn::engine::draw_fun() -> void
{
   static graph starfield_graph;

   {
      normal_imgui_window w("camera");

      const auto is_button_pressed = [&](const int key) -> bool {
         return glfwGetKey(m_window_wrapper.m_window, key) == GLFW_PRESS;
      };
      if (m_wasda_enabled)
      {
         if (is_button_pressed(GLFW_KEY_W)) {
            m_camera_pos.y += 0.1f;
         }
         if (is_button_pressed(GLFW_KEY_S)) {
            m_camera_pos.y += -0.1f;
         }
         if (is_button_pressed(GLFW_KEY_A)) {
            m_camera_pos.x += -0.1f;
         }
         if (is_button_pressed(GLFW_KEY_D)) {
            m_camera_pos.x += +0.1f;
         }
      }

      if(ImGui::Button(std::format("toggle WASDA (currently: {})", m_wasda_enabled).c_str()))
      {
         m_wasda_enabled = !m_wasda_enabled;
      }

      // ImGui::InputFloat("X", &m_camera_pos.x);
      // ImGui::InputFloat("Y", &m_camera_pos.y);
      // ImGui::InputFloat("Z", &m_camera_pos.z);
   }


   if (ImGui::GetFrameCount() == 1)
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

            gui_closest_stars();
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