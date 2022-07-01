#include "framebuffers.h"

#pragma warning(push, 0)    
#include <glm/gtc/type_ptr.hpp>
#pragma warning(pop)

#include "logging.h"


namespace
{
   using namespace sfn;

   [[nodiscard]] constexpr auto get_ogl_fb_target(const fb_target target) -> GLenum
   {
      switch (target)
      {
      case fb_target::full:     return GL_FRAMEBUFFER;
      case fb_target::read:     return GL_READ_FRAMEBUFFER;
      case fb_target::draw:     return GL_DRAW_FRAMEBUFFER;
      }
      log::error("wrong enum");
      std::terminate();
   }

   [[nodiscard]] auto get_new_fb_id() -> GLuint
   {
      GLuint opengl_id;
      glCreateFramebuffers(1, &opengl_id);
      return opengl_id;
   }


   auto get_default_fb_attachments() -> fb_attachments
   {
      GLint dims[4] = { 0 };
      glGetIntegerv(GL_VIEWPORT, dims);
      return fb_attachments{
         .m_image_metrics = {.m_width = dims[2], .m_height = dims[3], .m_bpp = 3} // DEPTH correct? TODO
      };
   }

   constexpr GLuint default_framebuffer_id = 0;

} // namespace {}


sfn::framebuffer::framebuffer(
   const fb_attachments& attachments
)
   : framebuffer(get_new_fb_id(), attachments)
{
   if (attachments.m_color_attachments.empty() == false)
   {
      glNamedFramebufferTexture(m_opengl_id, GL_COLOR_ATTACHMENT0, attachments.m_color_attachments[0], 0);
   }
   if(attachments.m_depth_attachment.has_value())
   {
      glNamedFramebufferTexture(m_opengl_id, GL_DEPTH_ATTACHMENT, attachments.m_depth_attachment.value(), 0);
   }
   if (attachments.m_stencil_attachment.has_value())
   {
      glNamedFramebufferTexture(m_opengl_id, GL_STENCIL_ATTACHMENT, attachments.m_stencil_attachment.value(), 0);
   }
   if(attachments.m_color_attachments.empty())
   {
      glNamedFramebufferDrawBuffer(m_opengl_id, GL_NONE);
      glNamedFramebufferReadBuffer(m_opengl_id, GL_NONE);
   }

   if(this->is_complete(fb_target::full) == false)
   {
      log::error("fb not complete");
      std::terminate();
   }
}


sfn::framebuffer::framebuffer(no_init)
   : framebuffer(default_framebuffer_id, get_default_fb_attachments())
{
   if(this->is_complete(fb_target::full) == false)
   {
      // log::error("default fb not complete?");
   }
}


auto sfn::framebuffer::is_complete(const fb_target target) const -> bool
{
   return glCheckNamedFramebufferStatus(m_opengl_id, get_ogl_fb_target(target)) == GL_FRAMEBUFFER_COMPLETE;
}


framebuffer::framebuffer(
   const GLuint opengl_id,
   const fb_attachments& attachments
)
   : m_opengl_id(opengl_id)
   , m_id(id::create())
   , m_attachments(attachments)
{

}


sfn::framebuffer_manager::framebuffer_manager(texture_manager& tex_man_ref)
   : m_framebuffers({ framebuffer(no_init{}) })
   , m_read_bount(m_framebuffers.back().m_id)
   , m_draw_bount(m_framebuffers.back().m_id)
   , m_tex_man_ref(tex_man_ref)
{

}


auto sfn::framebuffer_manager::bind_fb(
   const id fb_id,
   const fb_target target
) -> void
{
   glBindFramebuffer(get_ogl_fb_target(target), this->get_fb_ref(fb_id).m_opengl_id);
   if(target == fb_target::draw || target == fb_target::full)
   {
      m_draw_bount = fb_id;
   }
   else if (target == fb_target::read || target == fb_target::full)
   {
      m_read_bount = fb_id;
   }
}


auto sfn::framebuffer_manager::create_fb(
   const fb_attachments& attachments
) -> id
{
   return m_framebuffers.emplace_back(attachments).m_id;
}


auto framebuffer_manager::get_efault_fb() const -> id
{
   return m_framebuffers.front().m_id;
}


auto framebuffer_manager::get_opengl_id(const id fb_id) const -> GLuint
{
   return this->get_fb_ref(fb_id).m_opengl_id;
}


auto framebuffer_manager::get_dimensions(const id fb_id) const -> glm::ivec2
{
   const framebuffer& fb_ref = this->get_fb_ref(fb_id);
   return glm::ivec2{
      fb_ref.m_attachments.m_image_metrics.m_width,
      fb_ref.m_attachments.m_image_metrics.m_height
   };
}


auto framebuffer_manager::clear_color(
   const id fb_id,
   const glm::vec3& rgba,
   const int color_buffer_attach_number
) -> void
{
   if(m_draw_bount != fb_id)
   {
      log::error("can't clear buffer that isn't draw-bound");
      std::terminate();
   }
   glClearNamedFramebufferfv(get_opengl_id(fb_id), GL_COLOR, color_buffer_attach_number, glm::value_ptr(rgba));
}


auto framebuffer_manager::clear_depth(
   const id fb_id
) -> void
{
   if (m_draw_bount != fb_id)
   {
      log::error("can't clear buffer that isn't draw-bound");
      std::terminate();
   }
   constexpr float depth_value = 1.0f; // I think this must always be 1.0
   glClearNamedFramebufferfv(get_opengl_id(fb_id), GL_DEPTH, 0, &depth_value);
}


auto framebuffer_manager::blit_color(
   const id from,
   const id to,
   const interpolation_filter filter
) -> void
{
   const framebuffer& from_ref = get_fb_ref(from);
   const framebuffer& to_ref = get_fb_ref(to);
   int param;
   glGetNamedFramebufferParameteriv(from_ref.m_opengl_id, GL_SAMPLES, &param);
   glGetNamedFramebufferParameteriv(to_ref.m_opengl_id, GL_SAMPLES, &param);
   glGetNamedFramebufferParameteriv(from_ref.m_opengl_id, GL_SAMPLE_BUFFERS, &param);
   glGetNamedFramebufferParameteriv(to_ref.m_opengl_id, GL_SAMPLE_BUFFERS, &param);
   glBlitNamedFramebuffer(
      from_ref.m_opengl_id,
      to_ref.m_opengl_id,
      0, 0, from_ref.m_attachments.m_image_metrics.m_width, from_ref.m_attachments.m_image_metrics.m_height,
      0, 0, to_ref.m_attachments.m_image_metrics.m_width, to_ref.m_attachments.m_image_metrics.m_height,
      GL_COLOR_BUFFER_BIT,
      get_mag_filter(filter)
   );
}


auto framebuffer_manager::get_fb_ref(const id fb_id) const -> const framebuffer&
{
   const auto has_target_id = [&](const framebuffer& fb)
   {
      return fb.m_id == fb_id;
   };
   const auto it = std::ranges::find_if(m_framebuffers, has_target_id);
   if (it == std::end(m_framebuffers))
   {
      log::error("FB not found");
      std::terminate();
   }
   return *it;
}
