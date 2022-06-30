#include "setup.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h> // after glad
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui.h>
// #include <imgui_stdlib.h>
// #include <glm/geometric.hpp>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


namespace
{
   using namespace sfn;

   auto disable_maximize_button(HWND windowHandle) -> void
   {
      auto style = GetWindowLong(windowHandle, GWL_STYLE);
      style &= ~WS_MAXIMIZEBOX;
      SetWindowLong(windowHandle, GWL_STYLE, style);
   }


   void GLAPIENTRY
      MessageCallback(
         GLenum,
         GLenum type,
         GLuint,
         GLenum severity,
         GLsizei,
         const GLchar* message,
         const void*)
   {
      if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
         return;
      // ignoring performance warning for now. Maybe fixed with SPIRV?
      if (type == GL_DEBUG_TYPE_PERFORMANCE)
         return;
      int end = 0;
      // sg::log::warn(
      //    std::format(
      //       "OpenGL Error.\n\tType: {}\n\tSeverity: {}\n\tmessage: {}\n",
      //       sg::opengl_enum_to_str(type),
      //       sg::opengl_enum_to_str(severity),
      //       message
      //    )
      // );
   }


} // namespace {}


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

   disable_maximize_button(glfwGetWin32Window(m_window));

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


sfn::glad_wrapper::glad_wrapper(const config& config)
{
   const int glad_version = gladLoadGL(glfwGetProcAddress);
   if (glad_version == 0)
      throw std::exception{ "gladLoadGL error" };

   // error handling
#ifdef _DEBUG
   glEnable(GL_DEBUG_OUTPUT);
   glDebugMessageCallback(MessageCallback, 0);
   glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);

   glViewport(0, 0, config.res_x, config.res_y);
   glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

   glEnable(GL_MULTISAMPLE);
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
