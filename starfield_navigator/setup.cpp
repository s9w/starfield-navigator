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
      log::error(message);
      // sg::log::warn(
      //    fmt::format(
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


sfn::window_wrapper::window_wrapper(config& config)
{
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config.opengl_major_version);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, config.opengl_minor_version);
   glfwWindowHint(GLFW_MAXIMIZED, true);
   glfwWindowHint(GLFW_SAMPLES, 4);
   constexpr GLFWmonitor* monitor = nullptr;
   constexpr GLFWwindow* shared_window = nullptr;
   const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

   m_window = glfwCreateWindow(config.res_x, config.res_y, config.window_title.c_str(), monitor, shared_window);
   glfwGetWindowSize(m_window, &config.res_x, &config.res_y);
   if (m_window == nullptr)
   {
      throw std::exception{ "glfwCreateWindow error" };
   }
   glfwMakeContextCurrent(m_window);

   if (config.vsync)
      glfwSwapInterval(1);

   if (glfwRawMouseMotionSupported() == false)
      std::terminate();

   glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
   const auto raw_mode = glfwGetInputMode(m_window, GLFW_RAW_MOUSE_MOTION);
   fmt::print("raw_mode: {}\n", raw_mode);
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
   glFrontFace(GL_CCW);

   glViewport(0, 0, config.res_x, config.res_y);
   glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

   glEnable(GL_MULTISAMPLE);
}


sfn::imgui_context::imgui_context(const config& config, GLFWwindow* window)
{
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGui_ImplGlfw_InitForOpenGL(window, true);
   const std::string glsl_version = fmt::format("#version {}{}0", config.opengl_major_version, config.opengl_minor_version);
   ImGui_ImplOpenGL3_Init(glsl_version.c_str());

   ImGui::GetStyle().FrameBorderSize = 1.0f;
   ImGui::GetIO().IniFilename = nullptr;
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


auto imgui_context::scroll_callback(
   GLFWwindow* window,
   double xoffset,
   double yoffset
) -> void
{
   ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}


auto imgui_context::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) -> void
{
   ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
}


graphics_context::graphics_context(config& config)
   : m_glfw()
   , m_window_wrapper(config)
   , m_glad_wrapper(config)
   , m_imgui_context(config, m_window_wrapper.m_window)
{
   
}


sfn::normal_imgui_window::normal_imgui_window(
   const char* name,
   const ImGuiWindowFlags extra_flags
)
{
   ImGui::Begin(name, nullptr, extra_flags);
}


normal_imgui_window::normal_imgui_window(
   const glm::ivec2& top_left,
   const glm::ivec2& size,
   const char* name,
   const ImGuiWindowFlags extra_flags
)
{
   ImGui::SetNextWindowPos(ImVec2(static_cast<float>(top_left[0]), static_cast<float>(top_left[1])), ImGuiCond_Once);
   ImGui::SetNextWindowSize(ImVec2(static_cast<float>(size[0]), static_cast<float>(size[1])), ImGuiCond_Once);
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

#include "fonts/FontAwesomeSolid.hpp"
#include "fonts/DroidSans.hpp"
#include "fonts/IconsFontAwesome5.h"

auto sfn::setup_imgui_fonts() -> void {
   ImGuiIO& io = ImGui::GetIO();
   ImFontConfig configBasic;
   ImFontConfig configMerge;
   configMerge.MergeMode = true;
   static const ImWchar rangesIcons[] = {
      ICON_MIN_FA, ICON_MAX_FA,
      0
   };
   constexpr float normal_font_size = 15.0f;
   constexpr float icon_font_size = 15.0f;
   io.Fonts->Clear();
   io.Fonts->AddFontFromMemoryCompressedTTF(DroidSans_compressed_data, DroidSans_compressed_size, round(normal_font_size), &configBasic);
   io.Fonts->AddFontFromMemoryCompressedTTF(FontAwesomeSolid_compressed_data, FontAwesomeSolid_compressed_size, round(icon_font_size), &configMerge, rangesIcons);
}


auto sfn::imgui_help(const char* desc) -> void {
   ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(0.0f, 0.5f, 1.0f));
   ImGui::Text((const char*)ICON_FA_QUESTION_CIRCLE);
   ImGui::PopStyleColor();
   if (ImGui::IsItemHovered())
   {
      ImGui::BeginTooltip();
      ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
      ImGui::TextUnformatted(desc);
      ImGui::PopTextWrapPos();
      ImGui::EndTooltip();
   }
}
