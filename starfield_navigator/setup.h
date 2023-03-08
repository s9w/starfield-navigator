#pragma once

#include <exception>


#include "buffer.h"
#include "framebuffers.h"
#include "graph.h"


struct GLFWwindow;
struct ImGuiTextFilter;

namespace sfn
{

   const std::string sfn_version_string = "0.14";

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
      glfw_wrapper(glfw_wrapper&&) = default;
      glfw_wrapper& operator=(glfw_wrapper&&) = default;
   };

   struct window_wrapper
   {
      static inline GLFWwindow* m_window = nullptr;
      explicit window_wrapper(config& config);
      ~window_wrapper();

      window_wrapper(const window_wrapper&) = delete;
      window_wrapper& operator=(const window_wrapper&) = delete;
      window_wrapper(window_wrapper&&) = default;
      window_wrapper& operator=(window_wrapper&&) = default;
   };


   struct glad_wrapper
   {
      explicit glad_wrapper(const config& config);
      ~glad_wrapper() = default;

      glad_wrapper(const glad_wrapper&) = delete;
      glad_wrapper& operator=(const glad_wrapper&) = delete;
      glad_wrapper(glad_wrapper&&) = default;
      glad_wrapper& operator=(glad_wrapper&&) = default;
   };


   struct imgui_context
   {
      explicit imgui_context(const config& config, GLFWwindow* window);
      ~imgui_context();
      auto frame_begin() const -> void;
      auto frame_end() const -> void;
      static auto scroll_callback(GLFWwindow* window, double, double) -> void;
      static auto mouse_button_callback(GLFWwindow* window, int button, int action, int mods) -> void;

      imgui_context(const imgui_context&) = delete;
      imgui_context& operator=(const imgui_context&) = delete;
      imgui_context(imgui_context&&) = default;
      imgui_context& operator=(imgui_context&&) = default;
   };


   struct graphics_context{
      glfw_wrapper m_glfw;
      window_wrapper m_window_wrapper;
      glad_wrapper m_glad_wrapper;
      imgui_context m_imgui_context;

      explicit graphics_context(config& config);
      graphics_context(graphics_context&&) = default;
      graphics_context& operator=(graphics_context&&) = default;
      graphics_context(const graphics_context&) = delete;
      graphics_context& operator=(const graphics_context&) = delete;
      ~graphics_context() = default;
   };


   typedef int ImGuiWindowFlags;

   struct normal_imgui_window {
      explicit normal_imgui_window(const char* name, const ImGuiWindowFlags extra_flags = 0);
      explicit normal_imgui_window(const glm::ivec2& top_left, const glm::ivec2& size, const char* name, const ImGuiWindowFlags extra_flags = 0);
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

   auto setup_imgui_fonts() -> void;
   auto imgui_help(const char* desc) -> void
   ;
}
