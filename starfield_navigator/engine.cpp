#include "engine.h"
#include "obj_parsing.h"


#pragma warning(push, 0)
#include <GLFW/glfw3.h> // after glad
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "fonts/FontAwesomeSolid.hpp"
#include "fonts/DroidSans.hpp"
#include "fonts/IconsFontAwesome5.h"
#pragma warning(pop)


namespace
{
   using namespace sfn;


   auto right_align_text(const std::string& text) -> void
   {
      const auto posX = (ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - ImGui::CalcTextSize(text.c_str()).x
         - ImGui::GetScrollX() - 2 * ImGui::GetStyle().ItemSpacing.x);
      if (posX > ImGui::GetCursorPosX())
         ImGui::SetCursorPosX(posX);
      ImGui::Text(text.c_str());
   }

   template<is_alternative<projection_params> T>
   [[nodiscard]] auto get_projection_matrix(const config& config, const T& param) -> glm::mat4
   {
      const float aspect = static_cast<float>(config.res_x) / config.res_y;
      if constexpr (std::same_as<T, perspective_params>)
      {
         constexpr float x_fov = glm::radians(60.0f);
         const float y_fov = x_fov / aspect;
         return glm::perspective(y_fov, aspect, 0.1f, 3000.0f);
      }
      else if constexpr (std::same_as<T, ortho_params>)
      {
         const float frustum_width = param.width;
         return glm::ortho(
            -0.5f * frustum_width,
            0.5f * frustum_width,
            -0.5f * frustum_width/ aspect,
            0.5f * frustum_width/ aspect,
            0.0f,
            500.0f
         );
      }
   }


   [[nodiscard]] auto get_indicator_mesh(
      const glm::vec3& center,
      const cs& cs
   ) -> std::vector<position_vertex_data>
   {
      std::vector<position_vertex_data> result;
      result.reserve(128);

      constexpr float cross_extension = 1.0;
      result.push_back(position_vertex_data{ .m_position = center - cross_extension* cs.m_front });
      result.push_back(position_vertex_data{ .m_position = center + cross_extension* cs.m_front });
      result.push_back(position_vertex_data{ .m_position = center - cross_extension* cs.m_right });
      result.push_back(position_vertex_data{ .m_position = center + cross_extension* cs.m_right });
      result.push_back(position_vertex_data{ .m_position = center - cross_extension* cs.m_up });
      result.push_back(position_vertex_data{ .m_position = center + cross_extension* cs.m_up });

      constexpr int circle_segments = 32;
      const auto i_to_pos = [&](int i){
         i = i % circle_segments;
         const float angle = 1.0f * i / circle_segments * 2.0f * std::numbers::pi_v<float>;
         const float x_rel = cross_extension * std::cos(angle);
         const float y_rel = cross_extension * std::sin(angle);
         return center + cs.m_right * x_rel + cs.m_front * y_rel;
      };
      for(int i=0; i<circle_segments; ++i)
      {
         result.push_back(position_vertex_data{ .m_position = i_to_pos(i)});
         result.push_back(position_vertex_data{ .m_position = i_to_pos(i+1)});
      }

      return result;
   }

   const std::vector<position_vertex_data> sphere_mesh = get_position_vertex_data(get_complete_obj_info("assets/Sphere.obj", -1.0f));
   const std::vector<position_vertex_data> cylinder_mesh = get_position_vertex_data(get_complete_obj_info("assets/Cylinder.obj", -1.0f));

   [[nodiscard]] auto get_bb_mesh(
      const bb_3D& old_coord_bb,
      const glm::mat4& trafo
   ) -> std::vector<glm::vec3>
   {
      const auto forward = [&](const glm::vec3& in) {
         return apply_trafo(trafo, in);
      };

      const glm::vec3 bottom_p0 = old_coord_bb.m_min + glm::vec3{ 0, 0, 0 } *old_coord_bb.get_size();
      const glm::vec3 bottom_p1 = old_coord_bb.m_min + glm::vec3{ 1, 0, 0 } *old_coord_bb.get_size();
      const glm::vec3 bottom_p2 = old_coord_bb.m_min + glm::vec3{ 1, 1, 0 } *old_coord_bb.get_size();
      const glm::vec3 bottom_p3 = old_coord_bb.m_min + glm::vec3{ 0, 1, 0 } *old_coord_bb.get_size();
      const glm::vec3 top_p0 = old_coord_bb.m_min + glm::vec3{ 0, 0, 1 } *old_coord_bb.get_size();
      const glm::vec3 top_p1 = old_coord_bb.m_min + glm::vec3{ 1, 0, 1 } *old_coord_bb.get_size();
      const glm::vec3 top_p2 = old_coord_bb.m_min + glm::vec3{ 1, 1, 1 } *old_coord_bb.get_size();
      const glm::vec3 top_p3 = old_coord_bb.m_min + glm::vec3{ 0, 1, 1 } *old_coord_bb.get_size();


      std::vector<glm::vec3> result;
      result.reserve(24);

      // bottom
      result.push_back(forward(bottom_p0));
      result.push_back(forward(bottom_p1));
      result.push_back(forward(bottom_p1));
      result.push_back(forward(bottom_p2));
      result.push_back(forward(bottom_p2));
      result.push_back(forward(bottom_p3));
      result.push_back(forward(bottom_p3));
      result.push_back(forward(bottom_p0));

      // top
      result.push_back(forward(top_p0));
      result.push_back(forward(top_p1));
      result.push_back(forward(top_p1));
      result.push_back(forward(top_p2));
      result.push_back(forward(top_p2));
      result.push_back(forward(top_p3));
      result.push_back(forward(top_p3));
      result.push_back(forward(top_p0));

      // connections
      result.push_back(forward(bottom_p0));
      result.push_back(forward(top_p0));
      result.push_back(forward(bottom_p1));
      result.push_back(forward(top_p1));
      result.push_back(forward(bottom_p2));
      result.push_back(forward(top_p2));
      result.push_back(forward(bottom_p3));
      result.push_back(forward(top_p3));

      return result;
   }

   std::vector<line_vertex_data> jump_line_mesh;
   std::vector<position_vertex_data> indicator_mesh;


   // written so it yields (0, -1, 0) for 0, 0, 1 parameters, which is how the geometry is set up
   [[nodiscard]] auto get_cartesian_from_spherical(
      const float phi_offset,
      const float theta,
      const float r,
      const cs& cs
   ) -> glm::vec3
   {
      const float phi = -std::numbers::pi_v<float> / 2 + phi_offset;
      const float equation_theta = -theta + std::numbers::pi_v<float> / 2.0f;
      const float x = r * std::cos(phi) * std::sin(equation_theta);
      const float y = r * std::sin(phi) * std::sin(equation_theta);
      const float z = r * std::cos(equation_theta);
      return glm::vec3{} + x * cs.m_right + y * cs.m_front + z * cs.m_up;
   }


   auto tooltip(const char* text) -> void
   {
      if (ImGui::IsItemHovered())
      {
         ImGui::BeginTooltip();
         ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
         ImGui::Text(text);
         ImGui::PopTextWrapPos();
         ImGui::EndTooltip();
      }
   }


   [[nodiscard]] auto get_obj_trafo_between_points(
      const glm::vec3& p0,
      const glm::vec3& p1,
      const float diameter
   ) -> glm::mat4
   {
      glm::mat4 trafo(1.0f);

      constexpr glm::vec3 galactic_cylinder{ 1, 0, 0 };
      const glm::vec3 target_direction = glm::normalize(p1 - p0);
      const float length = glm::distance(p1, p0);

      trafo = glm::translate(trafo, p0);

      const auto axis = glm::cross(galactic_cylinder, target_direction);
      const float angle = glm::orientedAngle(galactic_cylinder, target_direction, axis);
      trafo = glm::rotate(trafo, angle, axis);
      const float radius = 0.5f * diameter;
      trafo = glm::scale(trafo, glm::vec3{ length, radius, radius });
      return trafo;
   }


   struct mouse_movement_visitor
   {
      glm::vec2 m_mouse_movement;

      template<typename T>
      auto operator()([[maybe_unused]] T& alternative) -> void
      {
         
      }

      template<centery T>
      auto operator()(T& alternative) -> void
      {
         alternative.horiz_angle_offset += -0.005f * m_mouse_movement[0];
         alternative.vert_angle_offset += 0.005f * m_mouse_movement[1];
      }
   };

} // namespace {}


mouse_mover::mouse_mover(GLFWwindow* window)
   : m_pos(get_cursor_pos(window))
{
   
}


auto mouse_mover::get_mouse_movement(GLFWwindow* window) -> glm::vec2
{
   glm::vec2 new_pos = get_cursor_pos(window);
   const glm::vec2 dpos = new_pos - m_pos;
   m_pos = new_pos;
   return dpos;
}


sfn::engine::engine(const config& config, std::unique_ptr<graphics_context>&& gc, universe&& universe)
   : m_config(config)
   , m_graphics_context(std::move(gc))
   , m_universe(std::move(universe))
   , m_buffers2(128)
   , m_shader_stars("star_shader")
   , m_shader_lines("line_shader")
   , m_shader_indicator("indicator_shader")
   , m_shader_bb("bb_shader")
   , m_shader_connection("connection_shader")
   , m_framebuffers(m_textures)
{
   update_ssbo_bb();
   update_ssbo_colors_and_positions(0.0f);

   if (engine_ptr != nullptr)
      std::terminate();
   engine_ptr = this;
   glfwSetFramebufferSizeCallback(get_window(), engine::static_resize_callback);
   glfwSetScrollCallback(get_window(), engine::static_scroll_callback);
   glfwSetMouseButtonCallback(get_window(), engine::static_mouse_button_callback);

   std::vector<segment_type> buffer_layout;
   buffer_layout.emplace_back(ubo_segment(sizeof(mvp_type), "ubo_mvp"));

   buffer_layout.emplace_back(get_soa_vbo_segment(sphere_mesh));
   buffer_layout.emplace_back(get_soa_vbo_segment<line_vertex_data>(100*100));
   buffer_layout.emplace_back(get_soa_vbo_segment<position_vertex_data>(128));
   buffer_layout.emplace_back(ssbo_segment(m_star_props_ssbo.get_byte_count(), "star_ssbo"));
   buffer_layout.emplace_back(get_soa_vbo_segment(cylinder_mesh));
   const std::vector<id> segment_ids = m_buffers2.create_buffer(std::move(buffer_layout), usage_pattern::dynamic_draw);
   m_mvp_ubo_id = segment_ids[0];
   m_star_vbo_id = segment_ids[1];
   m_jump_lines_vbo_id = segment_ids[2];
   m_indicator_vbo_id = segment_ids[3];
   m_star_ssbo_id = segment_ids[4];
   m_cylinder_vbo_id = segment_ids[5];

   m_binding_point_man.add(m_mvp_ubo_id);
   m_binding_point_man.add(m_star_ssbo_id);
   m_main_fb = m_framebuffers.get_efault_fb();

   const buffer& buffer_ref = m_buffers2.get_single_buffer_ref();
   bind_ubo("ubo_mvp", buffer_ref, m_mvp_ubo_id, m_shader_stars);
   bind_ubo("ubo_mvp", buffer_ref, m_mvp_ubo_id, m_shader_lines);
   bind_ubo("ubo_mvp", buffer_ref, m_mvp_ubo_id, m_shader_connection);
   bind_ssbo("star_ssbo", buffer_ref, m_star_ssbo_id, m_shader_stars);
   bind_ssbo("star_ssbo", buffer_ref, m_star_ssbo_id, m_shader_connection);
   bind_ssbo("star_ssbo", buffer_ref, m_star_ssbo_id, m_shader_bb);

   m_vao_stars.emplace(m_buffers2, m_star_vbo_id, m_shader_stars);
   m_vao_jump_lines.emplace(m_buffers2, m_jump_lines_vbo_id, m_shader_lines);
   m_vao_connection_lines.emplace(m_buffers2, m_cylinder_vbo_id, m_shader_connection);
   m_vao_indicator.emplace(m_buffers2, m_indicator_vbo_id, m_shader_indicator);
   m_vao_bb.emplace(m_buffers2, m_cylinder_vbo_id, m_shader_bb);
}


auto sfn::engine::get_window() const -> GLFWwindow*
{
   return window_wrapper::m_window;
}


auto sfn::engine::static_resize_callback(GLFWwindow* window, int new_width, int new_height) -> void
{
   engine_ptr->resize_callback(window, new_width, new_height);
}

auto engine::static_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) -> void
{
   engine_ptr->scroll_callback(window, xoffset, yoffset);
}


auto engine::static_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) -> void
{
   engine_ptr->mouse_button_callback(window, button, action, mods);
}


auto sfn::engine::resize_callback(
   [[maybe_unused]] GLFWwindow* window,
   int new_width,
   int new_height
) -> void
{
   if (new_width == 0 || new_height == 0)
      return;
   m_config.res_x = new_width;
   m_config.res_y = new_height;
   glViewport(0, 0, new_width, new_height);
   draw_frame();
}


auto sfn::engine::scroll_callback(
   GLFWwindow* window,
   double xoffset,
   double yoffset
) -> void
{
   imgui_context::scroll_callback(window, xoffset, yoffset);
   const auto zoom = [&]<typename T>(T& mode){
      if constexpr (centery<T>)
      {
         mode.distance += -5.0f * static_cast<float>(yoffset);
         mode.distance = std::clamp(mode.distance, 8.0f, 250.0f);
      }
   };
   std::visit(zoom, m_camera_mode);
}


auto engine::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) -> void
{
   imgui_context::mouse_button_callback(window, button, action, mods);
   
   constexpr auto is_cursor_in_window = [](GLFWwindow* window){
      int window_width, window_height;
      glfwGetWindowSize(window, &window_width, &window_height);
      const glm::vec2 cursor_pos = get_cursor_pos(window);
      return cursor_pos[0] >= 0 && cursor_pos[0] < window_width && cursor_pos[1] >= 0 && cursor_pos[1] < window_width;
   };
   
   if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && is_cursor_in_window(window))
   {
      if (m_mouse_mover.has_value())
         std::terminate();
      m_mouse_mover.emplace(window);
   }
   if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
   {
      m_mouse_mover.reset();
   }
}


auto sfn::engine::draw_frame() -> void
{
   const timing_info timing_info = m_frame_pacer.get_timing_info();

   m_framebuffers.bind_fb(m_main_fb, fb_target::full);
   m_framebuffers.clear_depth(m_main_fb);
   constexpr glm::vec3 bg_color{};
   m_framebuffers.clear_color(m_main_fb, bg_color, 0);

   m_graphics_context->m_imgui_context.frame_begin();
   if (m_mouse_mover.has_value())
   {
      std::visit(mouse_movement_visitor{ m_mouse_mover->get_mouse_movement(this->get_window()) }, m_camera_mode);
   }
   const auto center_tiler = [&]<typename T>(T & mode) {
      if constexpr (centery<T>)
      {
         if (is_button_pressed(this->get_window(), GLFW_KEY_W))
            mode.vert_angle_offset += 0.01f;
         if (is_button_pressed(this->get_window(), GLFW_KEY_S))
            mode.vert_angle_offset -= 0.01f;
         if (is_button_pressed(this->get_window(), GLFW_KEY_A))
            mode.horiz_angle_offset -= 0.01f;
         if (is_button_pressed(this->get_window(), GLFW_KEY_D))
            mode.horiz_angle_offset += 0.01f;
      }
   };
   std::visit(center_tiler, m_camera_mode);

   const auto pitch_limiter = [&]<typename T>(T & mode) {
      if constexpr (centery<T>)
      {
         constexpr float half_pi = glm::radians(85.0f);
         mode.vert_angle_offset = std::clamp(mode.vert_angle_offset, -half_pi, half_pi);
      }
   };
   std::visit(pitch_limiter, m_camera_mode);

   if (std::holds_alternative<wasd_mode>(m_camera_mode))
   {
      auto& camera_pos = std::get<wasd_mode>(m_camera_mode).m_camera_pos;
      constexpr float ly_per_sec = 10.0f;
      const float move_distance = timing_info.m_last_frame_duration * ly_per_sec;
      if (is_button_pressed(this->get_window(), GLFW_KEY_W))
         camera_pos += move_distance * m_universe.m_cam_info.m_cs.m_front;
      if (is_button_pressed(this->get_window(), GLFW_KEY_S))
         camera_pos += -move_distance * m_universe.m_cam_info.m_cs.m_front;
      if (is_button_pressed(this->get_window(), GLFW_KEY_A))
         camera_pos += -move_distance * m_universe.m_cam_info.m_cs.m_right;
      if (is_button_pressed(this->get_window(), GLFW_KEY_D))
         camera_pos += move_distance * m_universe.m_cam_info.m_cs.m_right;
   }

   // calculate things
   update_mvp_member();
   if(std::holds_alternative<trailer_mode>(m_camera_mode))
   {
      float& progress = std::get<trailer_mode>(m_camera_mode).m_progress;
      progress = std::fmod(progress + 0.002f, 1.0f);
   }

   // upload things
   gpu_upload();

   // render things
   glEnable(GL_DEPTH_CLAMP);
   // draw bb
   if (this->m_show_bb)
   {
      m_vao_bb->bind();
      m_shader_bb.use();
      m_shader_bb.set_uniform("time", timing_info.m_steady_time);
      glDisable(GL_DEPTH_TEST);
      glDrawArraysInstanced(GL_TRIANGLES, 0, static_cast<GLsizei>(cylinder_mesh.size()), 12);
      glEnable(GL_DEPTH_TEST);
   }

   if (std::holds_alternative<jumps_mode>(m_gui_mode))
   {
      m_vao_connection_lines->bind();
      m_shader_connection.use();
      glDepthMask(false);
      glDrawArraysInstanced(GL_TRIANGLES, 0, static_cast<GLsizei>(cylinder_mesh.size()), m_connection_trafo_count);
      glDepthMask(true);

      m_vao_jump_lines->bind();
      m_shader_lines.use();
      m_shader_lines.set_uniform("time", timing_info.m_steady_time);
      glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(jump_line_mesh.size()));
   }
   else if (std::holds_alternative<connections_mode>(m_gui_mode))
   {
      m_vao_connection_lines->bind();
      m_shader_connection.use();
      glDrawArraysInstanced(GL_TRIANGLES, 0, static_cast<GLsizei>(cylinder_mesh.size()), m_connection_trafo_count);
   }

   
   if (std::holds_alternative<circle_mode>(m_camera_mode))
   {
      m_vao_indicator->bind();
      m_shader_indicator.use();
      glDisable(GL_DEPTH_TEST);
      glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(indicator_mesh.size()));
      glEnable(GL_DEPTH_TEST);
   }
   else if (std::holds_alternative<galactic_circle_mode>(m_camera_mode))
   {
      m_vao_indicator->bind();
      m_shader_indicator.use();
      glDisable(GL_DEPTH_TEST);
      glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(indicator_mesh.size()));
      glEnable(GL_DEPTH_TEST);

      const glm::vec3 system_pos = m_universe.m_systems[m_list_selection].get_position(m_position_mode);
      const float distance = glm::distance(system_pos, this->get_camera_pos());
      if (distance < 50.0f)
      {
         constexpr float dist_from_center = 1.1f;
         constexpr glm::vec4 color{ 1, 0, 0, 1 };
         this->draw_text("0",   system_pos + dist_from_center * glm::vec3{  1,  0, 0 }, glm::vec2{}, color);
         this->draw_text("90",  system_pos + dist_from_center * glm::vec3{  0,  1, 0 }, glm::vec2{}, color);
         this->draw_text("180", system_pos + dist_from_center * glm::vec3{ -1,  0, 0 }, glm::vec2{}, color);
         this->draw_text("270", system_pos + dist_from_center * glm::vec3{  0, -1, 0 }, glm::vec2{}, color);
      }
   }



   if(std::holds_alternative<wasd_mode>(m_camera_mode))
   {
      const glm::vec3 system_pos = m_universe.m_systems[m_list_selection].get_position(m_position_mode);
      const float distance_from_cam = glm::distance(this->get_camera_pos(), system_pos);
      const float pointsize = 500 / distance_from_cam;
      const float planet_radius = 0.5f * pointsize;
      draw_circle(system_pos, planet_radius + 4.0f, glm::vec4{1, 1, 1, 0.7f});
   }

   m_vao_stars->bind();
   m_shader_stars.use();
   glEnable(GL_DEPTH_TEST);
   glDepthMask(true);
   glDrawArraysInstanced(GL_TRIANGLES, 0, static_cast<GLsizei>(sphere_mesh.size()), static_cast<GLsizei>(m_universe.m_systems.size()));

   if(m_show_star_labels)
      draw_system_labels();

   // GUI
   this->gui_draw();

   m_graphics_context->m_imgui_context.frame_end();
   glfwSwapBuffers(this->get_window());
   glfwPollEvents();
   m_frame_pacer.mark_frame_end();
}


auto sfn::engine::draw_loop() -> void
{
   while (glfwWindowShouldClose(this->get_window()) == false)
   {
      this->draw_frame();
   }
}



auto sfn::engine::draw_list() -> bool
{
   static ImGuiTextFilter filter;
   filter.Draw((const char*)ICON_FA_SEARCH " Filter");

   int old_selection = m_list_selection;
   if (ImGui::BeginTable("##table_selector", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
   {
      ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
      ImGui::TableSetupColumn("idx", ImGuiTableColumnFlags_WidthFixed, 30.0f);
      ImGui::TableSetupColumn("SF name", ImGuiTableColumnFlags_None);
      ImGui::TableSetupColumn("real name", ImGuiTableColumnFlags_None);
      ImGui::TableHeadersRow();

      for (int i = 0; i < m_universe.m_systems.size(); i++)
      {
         if (filter.PassFilter(m_universe.m_systems[i].get_name().c_str()) == false)
            continue;
         const bool is_selected = (m_list_selection == i);

         ImGui::TableNextRow();
         ImGui::TableSetColumnIndex(0);
         right_align_text(fmt::format("{}", i));

         const galactic_coord gc = get_galactic(m_universe.m_systems[i].get_position(m_position_mode));

         std::string tooltip_str = fmt::format(
            "Original name: {}\nGalactic coord:\nl: {:.1f} deg\nb: {:.1f} deg\ndist: {:.1f} LY",
            m_universe.m_systems[i].m_name, glm::degrees(gc.m_l), glm::degrees(gc.m_b), gc.m_dist
         );
         if (m_universe.m_systems[i].m_speculative == true)
            tooltip_str = "SPECULATIVE!\n" + tooltip_str;

         {
            ImGui::TableSetColumnIndex(1);
            const std::optional<std::string> name = m_universe.m_systems[i].get_starfield_name();
            ImVec4 text_color = name.has_value() ? (ImVec4)ImColor::HSV(1.0f, 0.0f, 1.0f) : (ImVec4)ImColor::HSV(0.0f, 0.0f, 0.5f);
            if (m_universe.m_systems[i].m_speculative == true)
               text_color = (ImVec4)ImColor(1.0f, 1.0f, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, text_color);
            const std::string imgui_label = fmt::format("{} ##LC{}", name.value_or("unknown"), i);
            if (ImGui::Selectable(imgui_label.c_str(), is_selected))
            {
               m_list_selection = i;
            }
            ImGui::PopStyleColor();
            
            tooltip(tooltip_str.c_str());
         }
         {
            ImGui::TableSetColumnIndex(2);
            const std::string imgui_label = fmt::format("{} ##RC{}", m_universe.m_systems[i].m_astronomic_name, i);
            if (m_universe.m_systems[i].m_speculative == true)
               ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(1.0f, 1.0f, 0.0f));
            if (ImGui::Selectable(imgui_label.c_str(), is_selected))
            {
               m_list_selection = i;
            }
            if (m_universe.m_systems[i].m_speculative == true)
               ImGui::PopStyleColor();
            tooltip(tooltip_str.c_str());
         }
      }
      ImGui::EndTable();
   }
   return m_list_selection == old_selection;
}


auto sfn::engine::draw_jump_calculations(const bool switched_into_tab) -> void
{
   // static float jump_range = 20.0f;
   static std::optional<jump_path> path;

   static bool first_plot = true;
   bool course_changed = first_plot || switched_into_tab;
   first_plot = false;


   if (ImGui::Button(fmt::format("Source: {} {}", m_universe.m_systems[m_source_index].m_name, (const char*)ICON_FA_MAP_MARKER_ALT).c_str()))
   {
      course_changed = true;
      m_source_index = m_list_selection;
   }
   ImGui::SameLine();
   if (ImGui::Button(fmt::format("Destination: {} {}", m_universe.m_systems[m_destination_index].m_name, (const char*)ICON_FA_MAP_MARKER_ALT).c_str()))
   {
      course_changed = true;
      m_destination_index = m_list_selection;
   }



   ImGui::Text(fmt::format(
      "Direct distance: {:.1f} LY",
      m_universe.get_distance(m_source_index, m_destination_index, m_position_mode)
   ).c_str());

   static float slider_min = 0.0f;
   static float slider_max = 100.0f;
   
   if (course_changed)
   {
      slider_min = get_min_jump_dist(m_universe, m_source_index, m_destination_index, m_position_mode) + 0.001f;
      slider_max = m_universe.get_distance(m_source_index, m_destination_index, m_position_mode) + 0.001f;
      m_gui_mode.get_jumprange() = slider_min;
   }

   course_changed |= ImGui::SliderFloat("jump range", &m_gui_mode.get_jumprange(), slider_min, slider_max);

   static std::vector<std::string> path_strings;
   // Graph and path update
   if (course_changed || switched_into_tab)
   {
      m_starfield_graph = get_graph_from_universe(m_universe, m_gui_mode.get_jumprange());
      build_connection_mesh_from_graph(m_starfield_graph);

      const auto distance_getter = [&](const int i, const int j) {return m_universe.get_distance(i, j, m_position_mode); };
      path = m_starfield_graph.get_jump_path(m_source_index, m_destination_index, distance_getter);

      if (path.has_value())
      {
         path_strings.clear();
         float travelled_distance = 0.0f;
         for (int i = 0; i < path->m_stops.size() - 1; ++i)
         {
            const int this_stop_system = path->m_stops[i];
            const int next_stop_system = path->m_stops[i + 1];
            const float dist = glm::distance(
               m_universe.m_systems[this_stop_system].get_position(m_position_mode),
               m_universe.m_systems[next_stop_system].get_position(m_position_mode)
            );
            travelled_distance += dist;

            path_strings.push_back(fmt::format(
               "Jump {}: {} to {}. Distance: {:.1f} LY\n",
               i+1,
               m_universe.m_systems[this_stop_system].m_name,
               m_universe.m_systems[next_stop_system].m_name,
               dist
            ));
         }
         path_strings.push_back("-----");
         path_strings.push_back(fmt::format("Travelled distance: {:.1f} LY", travelled_distance));

         // update vertices
         jump_line_mesh.clear();
         travelled_distance = 0.0f;
         for (int i = 0; i < path->m_stops.size() - 1; ++i)
         {
            const int this_stop_system = path->m_stops[i];
            const int next_stop_system = path->m_stops[i + 1];
            const float dist = glm::distance(
               m_universe.m_systems[this_stop_system].get_position(m_position_mode),
               m_universe.m_systems[next_stop_system].get_position(m_position_mode)
            );

            jump_line_mesh.push_back(
               line_vertex_data{
                  .m_position = m_universe.m_systems[this_stop_system].get_position(m_position_mode),
                  .m_progress = travelled_distance
               }
            );
            travelled_distance += dist;
            jump_line_mesh.push_back(
               line_vertex_data{
                  .m_position = m_universe.m_systems[next_stop_system].get_position(m_position_mode),
                  .m_progress = travelled_distance
               }
            );
         }
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


auto engine::bind_ssbo(
   const std::string& name,
   const buffer& buffer_ref,
   const id segment_id,
   const shader_program& shader
) const -> void
{
   const GLuint block_index = glGetProgramResourceIndex(shader.m_opengl_id, GL_SHADER_STORAGE_BLOCK, name.c_str());

   const int binding_point = m_binding_point_man.get_point(segment_id);
   glShaderStorageBlockBinding(shader.m_opengl_id, block_index, binding_point);
   glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point, buffer_ref.m_buffer_opengl_id);

   const auto segment_size = buffer_ref.get_segment_size(segment_id);
   const auto offset_in_buffer = buffer_ref.get_segment_offset(segment_id);
   glBindBufferRange(GL_SHADER_STORAGE_BUFFER, binding_point, buffer_ref.m_buffer_opengl_id, offset_in_buffer, segment_size);
}


auto engine::gpu_upload() const -> void
{
   m_buffers2.upload_ubo(m_mvp_ubo_id, as_bytes(m_current_mvp));
   m_buffers2.upload_vbo(m_star_vbo_id, as_bytes(sphere_mesh));
   m_buffers2.upload_vbo(m_jump_lines_vbo_id, as_bytes(jump_line_mesh));
   m_buffers2.upload_vbo(m_indicator_vbo_id, as_bytes(indicator_mesh));
   m_buffers2.upload_vbo(m_cylinder_vbo_id, as_bytes(cylinder_mesh));

   m_buffers2.upload_ssbo(m_star_ssbo_id, as_bytes(m_star_props_ssbo), 0);
}


auto engine::update_mvp_member() -> void
{
   m_current_mvp.m_cam_pos = get_camera_pos();
   m_current_mvp.m_selected_system_pos = m_universe.m_systems[m_list_selection].get_position(m_position_mode);
   m_current_mvp.m_projection = std::visit(
      [&](const auto& alternative) {return get_projection_matrix(m_config, alternative); },
      m_projection_params
   );
   m_current_mvp.m_view = std::visit(
      [&](const auto& x) {return get_view_matrix(x); },
      m_camera_mode
   );
}


auto engine::get_camera_pos() const -> glm::vec3
{
   const auto visitor = [&]<typename T>(const T& mode) -> glm::vec3{
      if constexpr(std::same_as<T, wasd_mode>)
      {
         return mode.m_camera_pos;
      }
      else if constexpr (std::same_as<T, trailer_mode>)
      {
         return glm::mix(m_universe.m_cam_info.m_cam_pos0, m_universe.m_cam_info.m_cam_pos1, mode.m_progress);
      }
      else if constexpr (centery<T>)
      {
         const glm::vec3 offset = get_cartesian_from_spherical(mode.horiz_angle_offset, mode.vert_angle_offset, mode.distance, get_cs());
         return m_universe.m_systems[mode.m_planet].get_position(m_position_mode) + offset;
      }
   };
   return std::visit(visitor, m_camera_mode);
}





auto sfn::engine::build_connection_mesh_from_graph(
   const graph& connection_graph
) -> void
{
   m_connection_trafo_count = std::clamp(
      static_cast<int>(std::ssize(connection_graph.m_connections)),
      0,
      static_cast<int>(std::ssize(m_star_props_ssbo.connection_trafos))-1
   );

   int i = 0;
   for (const auto& [key, con] : connection_graph.m_connections)
   {
      if (i >= std::ssize(m_star_props_ssbo.connection_trafos))
         return;

      const glm::vec3& p0 = m_universe.m_systems[con.m_node_index0].get_position(m_position_mode);
      const glm::vec3& p1 = m_universe.m_systems[con.m_node_index1].get_position(m_position_mode);
      m_star_props_ssbo.connection_trafos[i] = get_obj_trafo_between_points(p0, p1, 0.05f);
      ++i;
   }
}


auto sfn::engine::gui_draw() -> void
{
   bool view_mode_changed = false;
   {
      normal_imgui_window w(glm::ivec2{ 250, 0 }, glm::ivec2{ 500, 60 }, fmt::format("Camera {}", (const char*)ICON_FA_VIDEO).c_str());

      const auto old_index = m_camera_mode.index();
      ImGui::AlignTextToFramePadding();

      if(ImGui::RadioButton("Frontal", std::holds_alternative<wasd_mode>(m_camera_mode)))
      {
         m_camera_mode = wasd_mode{.m_camera_pos = m_universe.m_cam_info.m_cam_pos0};
      }
      tooltip("WASD to move");
      ImGui::SameLine();
      if (ImGui::RadioButton("Center selection", std::holds_alternative<circle_mode>(m_camera_mode)))
      {
         m_camera_mode = circle_mode{
            .m_planet = m_list_selection,
            .distance = 100.0f,
            .horiz_angle_offset = 0,
            .vert_angle_offset = 0
         };
      }
      tooltip("WASD to rotate\nMouse wheel to change distance");

      ImGui::SameLine();
      if (ImGui::RadioButton("Center selection (galactic)", std::holds_alternative<galactic_circle_mode>(m_camera_mode)))
      {
         m_camera_mode = galactic_circle_mode{
            .m_planet = m_list_selection,
            .distance = 100.0f,
            .horiz_angle_offset = 0,
            .vert_angle_offset = 0
         };
      }
      tooltip("Galactic coordinates.\nWASD to rotate\nMouse wheel to change distance");

      ImGui::SameLine();
      if (ImGui::RadioButton("Like reveal", std::holds_alternative<trailer_mode>(m_camera_mode)))
      {
         m_camera_mode = trailer_mode{};
      }
      view_mode_changed = old_index != m_camera_mode.index();
      tooltip("Replays the camera movement from the reveal video");

      
   }

   bool selection_changed = false;
   {
      normal_imgui_window w(glm::ivec2{ 0, 0 }, glm::ivec2{ 250, 500 }, "System selector");
      selection_changed = draw_list();
   }
   {
      normal_imgui_window w(glm::ivec2{ 0, 500 }, glm::ivec2{ 350, m_config.res_y-500 }, "Options");

      ImGui::Checkbox("Show star names", &m_show_star_labels);
      ImGui::Checkbox("Show bounding box", &m_show_bb);
      {
         static int radio_selected = 0;
         const int old_selected = radio_selected;
         ImGui::AlignTextToFramePadding();
         ImGui::Text("Star coloring:");
         ImGui::SameLine();
         if (ImGui::RadioButton("big/small", &radio_selected, 0))
         {
            m_star_color_mode = star_color_mode::big_small;
         }
         tooltip("Coloring stars green/red according depending on whether they were shown as big or small dots in the gameplay reveal");
         ImGui::SameLine();
         if (ImGui::RadioButton("absolute magnitude", &radio_selected, 1))
         {
            m_star_color_mode = star_color_mode::abs_mag;
         }
         if (m_star_color_mode == star_color_mode::abs_mag)
         {
            ImGui::PushItemWidth(-FLT_MIN);
            if (ImGui::SliderFloat("", &m_abs_mag_threshold, 0.0f, 20.0f))
            {
               this->update_ssbo_colors_and_positions(m_abs_mag_threshold);
            }
            ImGui::PopItemWidth();
            tooltip("Stars with magnitude higher than this (=darker) are dimmed");
         }

         if (radio_selected != old_selected)
         {
            this->update_ssbo_colors_and_positions(m_abs_mag_threshold);
         }
      }
      

      {
         ImGui::AlignTextToFramePadding();
         ImGui::Text("Camera mode:");
         ImGui::SameLine();
         if (ImGui::RadioButton("Perspective", std::holds_alternative<perspective_params>(m_projection_params)))
         {
            m_projection_params = perspective_params{};
         }
         ImGui::SameLine();
         if (ImGui::RadioButton("Orthographic", std::holds_alternative<ortho_params>(m_projection_params)))
         {
            m_projection_params = ortho_params{.width = 50.0f};
         }
         if (std::holds_alternative<ortho_params>(m_projection_params))
         {
            ImGui::SliderFloat("width", &std::get<ortho_params>(m_projection_params).width, 20.0f, 200.0f);
            ImGui::SameLine();
            imgui_help("Adjust this for \"zoom\". It's the width of the view frustrum in ly");
         }
      }


      ImGui::Text("Star positions:");
      ImGui::SameLine();
      if (ImGui::RadioButton("Reconstructed", m_position_mode==position_mode::reconstructed))
      {
         m_position_mode = position_mode::reconstructed;
         this->update_ssbo_colors_and_positions(m_abs_mag_threshold);
         this->build_connection_mesh_from_graph(m_starfield_graph);
      }
      ImGui::SameLine();
      if (ImGui::RadioButton("Accurate", m_position_mode == position_mode::from_catalog))
      {
         m_position_mode = position_mode::from_catalog;
         this->update_ssbo_colors_and_positions(m_abs_mag_threshold);
         this->build_connection_mesh_from_graph(m_starfield_graph);
      }
   }
   if(selection_changed || view_mode_changed)
   {

      const auto center_updater = [&]<typename T>(T& alternative){
         if constexpr(centery<T>)
         {
            alternative.m_planet = m_list_selection;
         }
      };
      std::visit(center_updater, m_camera_mode);

      indicator_mesh = get_indicator_mesh(m_universe.m_systems[m_list_selection].get_position(m_position_mode), this->get_cs());
   }

   {
      constexpr float tools_width = 500.0f;
      normal_imgui_window w(glm::ivec2{ m_config.res_x- tools_width, 0 }, glm::ivec2{ tools_width, 250 }, "Tools");

      if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None))
      {
         ImGui::PushStyleColor(ImGuiCol_Tab, (ImVec4)ImColor::HSV(0.0f, 0.5f, 0.6f));
         ImGui::PushStyleColor(ImGuiCol_TabHovered, (ImVec4)ImColor::HSV(0.0f, 0.5f, 0.8f));
         ImGui::PushStyleColor(ImGuiCol_TabActive, (ImVec4)ImColor::HSV(0.0f, 0.5f, 0.9f));

         static auto old_gui_index = m_gui_mode.index();
         if (ImGui::BeginTabItem("Show connections"))
         {
            if(std::holds_alternative<connections_mode>(m_gui_mode) == false)
               m_gui_mode = gui_mode{ connections_mode{ m_gui_mode.get_jumprange() } };
            static graph connection_graph = get_graph_from_universe(m_universe, m_gui_mode.get_jumprange());
            bool changed = m_gui_mode.index() != old_gui_index;
            changed |= ImGui::SliderFloat("jump range", &m_gui_mode.get_jumprange(), 0, 30);
            if(changed || m_connection_trafo_count == 0)
            {
               connection_graph = get_graph_from_universe(m_universe, m_gui_mode.get_jumprange());
               build_connection_mesh_from_graph(connection_graph);
            }
            ImGui::EndTabItem();
         }

         if (ImGui::BeginTabItem("Jump calculations"))
         {
            if (std::holds_alternative<jumps_mode>(m_gui_mode) == false)
               m_gui_mode = gui_mode{ jumps_mode{ m_gui_mode.get_jumprange() } };
            draw_jump_calculations(m_gui_mode.index() != old_gui_index);
            ImGui::EndTabItem();
         }
         old_gui_index = m_gui_mode.index();

         // if (ImGui::BeginTabItem("game"))
         // {
         //    m_gui_mode = gui_mode::game;
         //    static float game_jump_range = 20.0f;
         //    static int current_system = 14;
         //    ImGui::SliderFloat("jump range", &game_jump_range, 10, 30);
         //
         //    ImGui::EndTabItem();
         // }

         ImGui::PopStyleColor(3);

         ImGui::EndTabBar();
      }

   }

   // ImGui::ShowDemoWindow();
}


auto engine::get_view_matrix(const camera_mode& mode) const -> glm::mat4
{
   glm::vec3 up_vector = m_universe.m_cam_info.m_cs.m_up;
   if(std::holds_alternative<galactic_circle_mode>(mode))
   {
      up_vector = glm::vec3{ 0, 0, 1 };
   }

   return glm::lookAt(
      this->get_camera_pos(),
      get_camera_target(mode),
      up_vector
   );
}


auto engine::get_camera_target(const camera_mode& mode) const -> glm::vec3
{
   const auto visitor = [&]<typename T>(const T& alternative) {
      if constexpr(centery<T>)
         return m_universe.m_systems[alternative.m_planet].get_position(m_position_mode);
      else if constexpr(std::same_as<T, wasd_mode> || std::same_as<T, trailer_mode>)
         return this->get_camera_pos() + m_universe.m_cam_info.m_cs.m_front;
   };
   return std::visit(visitor, mode);
}


auto engine::draw_text(
   const std::string& text,
   const glm::vec3& pos,
   const glm::vec2& center_offset,
   const glm::vec4& color
) const -> void
{
   glm::vec4 screen_pos = m_current_mvp.m_projection * m_current_mvp.m_view * glm::vec4{ pos, 1.0f };
   if (screen_pos[3] < 0)
      return;
   screen_pos /= screen_pos[3];

   glm::vec2 imgui_draw_pos = 0.5f * (glm::vec2(screen_pos) + 1.0f);
   imgui_draw_pos[1] = 1.0f - imgui_draw_pos[1];
   imgui_draw_pos *= glm::vec2{ m_config.res_x, m_config.res_y };

   const ImVec2 text_size = ImGui::CalcTextSize(text.c_str());
   imgui_draw_pos.x -= 0.5f * text_size.x;
   imgui_draw_pos.y -= 0.5f * text_size.y;

   imgui_draw_pos += glm::vec2{1, -1} * center_offset;
   const auto text_color = ImColor(color[0], color[1], color[2], color[3]);
   ImGui::GetBackgroundDrawList()->AddText(ImVec2(imgui_draw_pos[0], imgui_draw_pos[1]), text_color, text.c_str());
}


auto engine::draw_circle(const glm::vec3& pos, const float radius, const glm::vec4& color) const -> void
{
   glm::vec4 screen_pos = m_current_mvp.m_projection * m_current_mvp.m_view * glm::vec4{ pos, 1.0f };
   if (screen_pos[3] < 0)
      return;
   screen_pos /= screen_pos[3];

   glm::vec2 imgui_draw_pos = 0.5f * (glm::vec2(screen_pos) + 1.0f);
   imgui_draw_pos[1] = 1.0f - imgui_draw_pos[1];
   imgui_draw_pos *= glm::vec2{ m_config.res_x, m_config.res_y };
   const auto imgui_color = ImColor(color[0], color[1], color[2], color[3]);
   ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(imgui_draw_pos[0], imgui_draw_pos[1]), radius, imgui_color);
}


auto engine::get_cs() const -> cs
{
   if(std::holds_alternative<galactic_circle_mode>(m_camera_mode))
   {
      return cs(glm::vec3{ 0, 1, 0 }, glm::vec3{ 0, 0, 1 });
   }
   else
   {
      return m_universe.m_cam_info.m_cs;
   }
}


auto engine::update_ssbo_colors_and_positions(const float abs_threshold) -> void
{
   constexpr glm::vec3 speculative_color{ 1, 1, 0 };

   

   if (m_star_color_mode == star_color_mode::big_small)
   {
      for (int i = 0; i < m_universe.m_systems.size(); ++i)
      {
         constexpr glm::vec3 red{ 1.0f, 0.5f, 0.5f };
         constexpr glm::vec3 green{ 0.5f, 1.0f, 0.5f };
         m_star_props_ssbo.m_stars[i].color = (m_universe.m_systems[i].m_size == system_size::small) ? red : green;
         if (m_universe.m_systems[i].m_speculative)
            m_star_props_ssbo.m_stars[i].color = speculative_color;
         m_star_props_ssbo.m_stars[i].position = m_universe.m_systems[i].get_position(m_position_mode);
      }
   }
   else if (m_star_color_mode == star_color_mode::abs_mag)
   {
      for (int i = 0; i < std::ssize(m_universe.m_systems); ++i)
      {
         constexpr glm::vec3 bright{ 1.0f };
         constexpr glm::vec3 faint{ 0.5f };
         m_star_props_ssbo.m_stars[i].position = m_universe.m_systems[i].get_position(m_position_mode);
         m_star_props_ssbo.m_stars[i].color = (m_universe.m_systems[i].m_abs_mag < abs_threshold) ? bright : faint;
         if (m_universe.m_systems[i].m_speculative)
            m_star_props_ssbo.m_stars[i].color = speculative_color;
      }
   }
}


auto engine::update_ssbo_bb() -> void
{
   const std::vector<glm::vec3> x = get_bb_mesh(m_universe.m_map_bb, m_universe.m_trafo);
   for (int i = 0; i < x.size()/2; ++i)
   {
      const glm::vec3 p0 = x[2 * i];
      const glm::vec3 p1 = x[2 * i + 1];
      m_star_props_ssbo.bb_elements[i].trafo = get_obj_trafo_between_points(p0, p1, 0.1f);
   }
}


auto engine::draw_system_labels() const -> void
{
   const glm::vec3 cam_pos = this->get_camera_pos();
   for (const system& system : m_universe.m_systems)
   {
      if (system.get_useful_name().has_value() == false)
         continue;

      const float distance_from_cam = glm::distance(cam_pos, system.get_position(m_position_mode));
      const float pointsize = 500 / distance_from_cam;
      const float planet_radius = 0.5f * pointsize;
      const glm::vec2 offset{0, planet_radius + 8.0f };
      constexpr float label_opacity = 0.8f;
      constexpr glm::vec4 normal_color{ 1, 1, 1, label_opacity };
      constexpr glm::vec4 speculation_color{ 1, 0.6, 0.95, label_opacity };
      glm::vec4 color = system.get_starfield_name().has_value() ? normal_color : speculation_color;
      if (system.m_speculative)
         color = glm::vec4{1, 1, 0, 1};
      this->draw_text(system.get_useful_name().value(), system.get_position(m_position_mode), offset, color);
   }
}
