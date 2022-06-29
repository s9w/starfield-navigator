#pragma once

#include "setup.h"


namespace sfn
{

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
      auto gui_closest_stars() -> void;
      auto gui_plotter(graph& starfield_graph) -> void;
   };

}

