#pragma once

#include "setup.h"
#include "vertex_data.h"
#include "buffer.h"
#include "timing_provider.h"
#include "universe.h"


namespace sfn
{

   struct alignas(256) ubo_type {};

   struct mvp_type : ubo_type
   {
      alignas(glm::vec4) glm::vec3 m_cam_pos;
      alignas(sizeof(glm::vec4)) glm::mat4 m_view{ 1.0f };
      alignas(sizeof(glm::vec4)) glm::mat4 m_projection{ 1.0f };
   };

   struct wasd_mode
   {
      glm::vec3 m_camera_pos{};
   };
   struct circle_mode
   {
      int m_planet;
      float distance;
      float horiz_angle_offset;
      float vert_angle_offset;
   };
   struct trailer_mode{
      float m_progress = 0.0;
   };

   using camera_mode = std::variant<wasd_mode, circle_mode, trailer_mode>;

   enum class gui_mode{closest, jumps, connections, game};

   struct engine
   {
   private:
      static inline engine* engine_ptr;
   public:
      config m_config;
      timing_provider m_frame_pacer{};
      glfw_wrapper m_glfw;
      window_wrapper m_window_wrapper;
      glad_wrapper m_glad_wrapper;
      imgui_context m_imgui_context;

      universe m_universe;
      int m_list_selection = m_universe.get_index_by_name("SOL");
      int m_source_index = m_universe.get_index_by_name("SOL");
      int m_destination_index = m_universe.get_index_by_name("PORRIMA");
      gui_mode m_gui_mode = gui_mode::connections;

      camera_mode m_camera_mode = wasd_mode{ m_universe.m_cam_info.m_cam_pos0 };
      buffers m_buffers2;
      id m_mvp_ubo_id{ no_init{} };
      id m_main_fb{ no_init{} };
      id m_star_vbo_id{ no_init{} };
      id m_screen_rect_vbo_id{ no_init{} };
      id m_jump_lines_vbo_id{ no_init{} };
      id m_connection_lines_vbo_id{ no_init{} };
      id m_closest_lines_vbo_id{ no_init{} };
      id m_indicator_vbo_id{ no_init{} };
      binding_point_man m_binding_point_man;
      mvp_type m_current_mvp{};
      shader_program m_shader_stars;
      shader_program m_shader_lines;
      shader_program m_shader_indicator;
      texture_manager m_textures{};
      framebuffer_manager m_framebuffers; // needs to be after texture manager
      std::optional<vao> m_vao_stars;
      std::optional<vao> m_vao_jump_lines;
      std::optional<vao> m_vao_connection_lines;
      std::optional<vao> m_vao_closest_lines;
      std::optional<vao> m_vao_screen_rect;
      std::optional<vao> m_vao_indicator;

      explicit engine(const config& config, universe&& universe);
      [[nodiscard]] auto get_window() const->GLFWwindow*;

      static auto static_resize_callback(
         GLFWwindow* window,
         int new_width,
         int new_height
      ) -> void;
      static auto static_scroll_callback(
         GLFWwindow* window,
         double xoffset, double yoffset
      ) -> void;



      
      auto draw_loop() -> void;

      engine(const engine&) = delete;
      engine& operator=(const engine&) = delete;
      engine(engine&&) = delete;
      engine& operator=(engine&&) = delete;

   private:
      auto resize_callback(
         GLFWwindow* window,
         int new_width,
         int new_height
      ) -> void;
      auto scroll_callback(
         GLFWwindow* window,
         double xoffset,
         double yoffset
      ) -> void;

      auto draw_frame() -> void;
      auto gui_draw() -> void;
      auto draw_list() -> bool;
      auto gui_closest_stars(const bool switched_into_tab) -> void;
      auto draw_jump_calculations(const bool switched_into_tab) -> void;
      auto bind_ubo(const std::string& name, const buffer& buffer_ref, const id segment_id, const shader_program& shader) const -> void;
      auto gpu_upload() const -> void;
      auto update_mvp_member() -> void;
      auto get_camera_pos() const -> glm::vec3;
      [[nodiscard]] auto get_view_matrix(const camera_mode& mode) const -> glm::mat4;
      [[nodiscard]] auto get_camera_target(const camera_mode& mode) const -> glm::vec3;
      auto draw_system_labels() const -> void;
      auto build_connection_mesh_from_graph(const graph& connection_graph) const -> std::vector<line_vertex_data>;
      auto build_neighbor_connection_mesh(const universe& universe, const int center_system) const -> std::vector<line_vertex_data>;
   };
}

