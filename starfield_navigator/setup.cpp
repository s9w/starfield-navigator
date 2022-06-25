#include "setup.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h> // after glad
// #define GLFW_EXPOSE_NATIVE_WIN32
// #include <GLFW/glfw3native.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui.h>
#include <imgui_stdlib.h>

#include <optional>


glfw_wrapper::glfw_wrapper()
{
   if (glfwInit() == GLFW_FALSE)
      throw std::exception{ "glfwInit() error" };
}

glfw_wrapper::~glfw_wrapper()
{
   glfwTerminate();
}

window_wrapper::window_wrapper(const config& config)
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


window_wrapper::~window_wrapper()
{
   if (m_window != nullptr)
   {
      glfwDestroyWindow(m_window);
   }
}


glad_wrapper::glad_wrapper()
{
   const int glad_version = gladLoadGL(glfwGetProcAddress);
   if (glad_version == 0)
      throw std::exception{ "gladLoadGL error" };
}


imgui_context::imgui_context(const config& config, GLFWwindow* window)
{
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGui_ImplGlfw_InitForOpenGL(window, true);
   const std::string glsl_version = std::format("#version {}{}0", config.opengl_major_version, config.opengl_minor_version);
   ImGui_ImplOpenGL3_Init(glsl_version.c_str());
}


imgui_context::~imgui_context()
{
   ImGui_ImplOpenGL3_Shutdown();
   ImGui_ImplGlfw_Shutdown();
   ImGui::DestroyContext();
}


auto imgui_context::frame_begin() const -> void
{
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplGlfw_NewFrame();
   ImGui::NewFrame();
}


auto imgui_context::frame_end() const -> void
{
   ImGui::Render();
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


engine::engine(const config& config, const draw_fun_type& draw_fun)
   : m_window_wrapper(config)
   , m_imgui_context(config, m_window_wrapper.m_window)
   , m_draw_fun(draw_fun)
{
   // if (engine_ptr != nullptr)
   //    std::terminate();
   // engine_ptr = this;
   glfwSetFramebufferSizeCallback(get_window(), engine::static_resize_callback);
}


auto engine::get_window() const -> GLFWwindow*
{
   return m_window_wrapper.m_window;
}


auto engine::static_resize_callback(GLFWwindow* window, int new_width, int new_height) -> void
{
   engine_ptr->resize_callback(window, new_width, new_height);
}


auto engine::resize_callback(
   [[maybe_unused]] GLFWwindow* window,
   [[maybe_unused]] int new_width,
   [[maybe_unused]] int new_height
) const -> void
{
   draw_frame();
}


auto engine::draw_frame() const -> void
{
   glClear(GL_COLOR_BUFFER_BIT);
   m_imgui_context.frame_begin();

   m_draw_fun(this->get_window());

   m_imgui_context.frame_end();
   glfwSwapBuffers(this->get_window());
   glfwPollEvents();
}


auto engine::draw_loop() const -> void
{
   while (glfwWindowShouldClose(this->get_window()) == false)
   {
      draw_frame();
   }
}


normal_imgui_window::normal_imgui_window(const char* name, const ImGuiWindowFlags extra_flags)
{
   ImGui::Begin(name, nullptr, extra_flags);
}


normal_imgui_window::~normal_imgui_window()
{
   ImGui::End();
}


single_imgui_window::single_imgui_window(const ImGuiWindowFlags extra_flags)
{
   ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
   ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
   ImGuiWindowFlags flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
   flags |= extra_flags;
   ImGui::Begin(" ", nullptr, flags);
}


single_imgui_window::~single_imgui_window()
{
   ImGui::End();
}
