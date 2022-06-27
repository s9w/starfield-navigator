#pragma once

#include "texture.h"


namespace sfn
{
   enum class fb_target{full, read, draw};

   struct fb_attachments
   {
      image_metrics m_image_metrics;
      std::optional<GLuint> m_depth_attachment;
      std::optional<GLuint> m_stencil_attachment;
      std::vector<GLuint> m_color_attachments;
   };

   // color, depth or stencil attachments
   struct framebuffer
   {
      GLuint m_opengl_id{};
      id m_id;
      fb_attachments m_attachments;

      explicit framebuffer(const fb_attachments& attachments);

      // for default FB
      explicit framebuffer(no_init);

      [[nodiscard]] auto is_complete(const fb_target target) const -> bool;

   private:
      explicit framebuffer(const GLuint opengl_id, const fb_attachments& attachments);
   };

   struct framebuffer_manager
   {
   private:
      std::vector<framebuffer> m_framebuffers;
      id m_read_bount;
      id m_draw_bount;
      texture_manager& m_tex_man_ref;

   public:
      explicit framebuffer_manager(texture_manager& tex_man_ref);
      auto bind_fb(const id fb_id, const fb_target target) -> void;
      auto create_fb(const fb_attachments& attachments) -> id;
      [[nodiscard]] auto get_efault_fb() const -> id;
      [[nodiscard]] auto get_opengl_id(const id fb_id) const -> GLuint;
      [[nodiscard]] auto get_dimensions(const id fb_id) const -> glm::ivec2;
      auto clear_color(const id fb_id, const glm::vec3& rgba, const int color_buffer_attach_number) -> void;
      auto clear_depth(const id fb_id) -> void;
      auto blit_color(const id from, const id to, const interpolation_filter filter) -> void;

   private:
      [[nodiscard]] auto get_fb_ref(const id fb_id) const -> const framebuffer&;
   };

}