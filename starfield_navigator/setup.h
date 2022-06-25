#pragma once

#include <exception>
#include <format>


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


struct GLFWwindow;

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


using draw_fun_type = std::add_pointer_t<void(GLFWwindow*)>;


struct engine
{
private:
   static inline engine* engine_ptr;
public:

   glfw_wrapper m_glfw;
   window_wrapper m_window_wrapper;
   glad_wrapper m_glad_wrapper;
   imgui_context m_imgui_context;
   draw_fun_type m_draw_fun;

   explicit engine(const config& config, const draw_fun_type& draw_fun);
   ~engine() = default;
   [[nodiscard]] auto get_window() const -> GLFWwindow*;

   static auto static_resize_callback(
      GLFWwindow* window,
      int new_width,
      int new_height
   ) -> void;

   auto resize_callback(
      [[maybe_unused]] GLFWwindow* window,
      [[maybe_unused]] int new_width,
      [[maybe_unused]] int new_height
   ) const -> void;

   auto draw_frame() const -> void;
   auto draw_loop() const -> void;

   engine(const engine&) = delete;
   engine& operator=(const engine&) = delete;
   engine(engine&&) = delete;
   engine& operator=(engine&&) = delete;
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
