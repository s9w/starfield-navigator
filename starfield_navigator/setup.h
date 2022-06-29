#pragma once

#include <exception>
#include <format>

#include "buffer.h"
#include "framebuffers.h"
#include "graph.h"


struct GLFWwindow;
struct ImGuiTextFilter;

namespace sfn
{

   struct config {
      int res_x = 1280;
      int res_y = 720;
      int opengl_major_version = 4;
      int opengl_minor_version = 5;
      bool vsync = true;
      std::string window_title;
   };


   struct glfw_wrapper
   {
      explicit glfw_wrapper();
      ~glfw_wrapper();

      glfw_wrapper(const glfw_wrapper&) = delete;
      glfw_wrapper& operator=(const glfw_wrapper&) = delete;
      glfw_wrapper(glfw_wrapper&&) = delete;
      glfw_wrapper& operator=(glfw_wrapper&&) = delete;
   };

   struct window_wrapper
   {
      GLFWwindow* m_window = nullptr;
      explicit window_wrapper(const config& config);
      ~window_wrapper();

      window_wrapper(const window_wrapper&) = delete;
      window_wrapper& operator=(const window_wrapper&) = delete;
      window_wrapper(window_wrapper&&) = delete;
      window_wrapper& operator=(window_wrapper&&) = delete;
   };


   struct glad_wrapper
   {
      explicit glad_wrapper();
      ~glad_wrapper() = default;

      glad_wrapper(const glad_wrapper&) = delete;
      glad_wrapper& operator=(const glad_wrapper&) = delete;
      glad_wrapper(glad_wrapper&&) = delete;
      glad_wrapper& operator=(glad_wrapper&&) = delete;
   };


   struct imgui_context
   {
      explicit imgui_context(const config& config, GLFWwindow* window);
      ~imgui_context();
      auto frame_begin() const -> void;
      auto frame_end() const -> void;

      imgui_context(const imgui_context&) = delete;
      imgui_context& operator=(const imgui_context&) = delete;
      imgui_context(imgui_context&&) = delete;
      imgui_context& operator=(imgui_context&&) = delete;
   };

   struct alignas(256) ubo_type {};

   struct ubo_star
   {
      alignas(sizeof(glm::vec4)) glm::mat4 m_model_matrix;
      alignas(sizeof(glm::vec4)) glm::vec3 m_position;
   };

   struct mvp_type : ubo_type
   {
      alignas(sizeof(glm::vec4)) glm::vec3 m_lookat_pos;
      alignas(sizeof(glm::vec4)) glm::mat4 m_view{ 1.0f };
      alignas(sizeof(glm::vec4)) glm::mat4 m_projection{ 1.0f };
      alignas(sizeof(glm::vec4)) glm::mat4 m_pv{ 1.0f };
      alignas(sizeof(glm::vec4)) glm::mat4 m_pv_inverse{ 1.0f };
      alignas(glm::vec4) glm::vec3 m_cam_pos;

      constexpr static inline int size = 128;
      ubo_star m_array[size];
   };


   struct engine
   {
   private:
      static inline engine* engine_ptr;
   public:

      glfw_wrapper m_glfw;
      window_wrapper m_window_wrapper;
      glad_wrapper m_glad_wrapper;
      imgui_context m_imgui_context;

      universe m_universe;
      float m_jump_range = 30.0f;
      int m_list_selection = 0;
      int m_source_index = m_universe.get_index_by_name("SOL");
      int m_destination_index = m_universe.get_index_by_name("PORRIMA");

      buffers m_buffers2;
      id m_mvp_ubo_id{ no_init{} };
      id m_main_fb{ no_init{} };
      binding_point_man m_binding_point_man;
      shader_program m_shader_stars;
      texture_manager m_textures{};
      framebuffer_manager m_framebuffers; // needs to be after texture manager

      explicit engine(const config& config, universe&& universe);
      ~engine() = default;
      [[nodiscard]] auto get_window() const->GLFWwindow*;

      static auto static_resize_callback(
         GLFWwindow* window,
         int new_width,
         int new_height
      ) -> void;

      auto resize_callback(
         [[maybe_unused]] GLFWwindow* window,
         [[maybe_unused]] int new_width,
         [[maybe_unused]] int new_height
      ) -> void;

      auto draw_frame() -> void;
      auto draw_loop() -> void;
      auto draw_fun() -> void;

      engine(const engine&) = delete;
      engine& operator=(const engine&) = delete;
      engine(engine&&) = delete;
      engine& operator=(engine&&) = delete;

      auto draw_list() -> void;
      auto gui_closest_stars(const graph& starfield_graph) -> void;
      auto gui_plotter(graph& starfield_graph) -> void;
   };


   typedef int ImGuiWindowFlags;

   struct normal_imgui_window {
      explicit normal_imgui_window(const char* name, const ImGuiWindowFlags extra_flags = 0);
      ~normal_imgui_window();

      normal_imgui_window(const normal_imgui_window&) = delete;
      normal_imgui_window& operator=(const normal_imgui_window&) = delete;
      normal_imgui_window(normal_imgui_window&&) = delete;
      normal_imgui_window& operator=(normal_imgui_window&&) = delete;
   };


   struct single_imgui_window {
      explicit single_imgui_window(const ImGuiWindowFlags extra_flags = 0);
      ~single_imgui_window();

      single_imgui_window(const single_imgui_window&) = delete;
      single_imgui_window& operator=(const single_imgui_window&) = delete;
      single_imgui_window(single_imgui_window&&) = delete;
      single_imgui_window& operator=(single_imgui_window&&) = delete;
   };


}
