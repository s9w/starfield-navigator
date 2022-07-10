#include "buffer.h"

#include "logging.h"


namespace
{
   using namespace sfn;

   constexpr auto get_aligned_ubo_sizeof(const int byte_count) -> int
   {
      constexpr int todo_alignment = 256;
      if (byte_count % todo_alignment == 0)
         return byte_count;
      const int div = byte_count / todo_alignment;
      return (div + 1) * todo_alignment;
   }


   [[nodiscard]] constexpr auto get_gl_usage_pattern(const usage_pattern usage)
   {
      switch (usage)
      {
      case usage_pattern::static_draw:  return GL_STATIC_DRAW;
      case usage_pattern::dynamic_draw: return GL_DYNAMIC_DRAW;
      }
      log::error("bad enum");
      std::terminate();
   }


   [[nodiscard]] auto are_attribs_matching(
      const vbo_class_member& vbo_attrib,
      const shader_io& vertex_attrib
   ) -> bool
   {
      const bool equal_name = vertex_attrib.m_name == vbo_attrib.m_attrib_name;
      const bool equal_type = vertex_attrib.m_data_type == vbo_attrib.m_type;
      return equal_name && equal_type;
   }
   
   [[nodiscard]] auto get_matching_shader_attrib_location(
      const vbo_class_member& vbo_attrib,
      const std::vector<shader_io>& shader_inputs
   ) -> std::optional<int>
   {
      auto is_attrib_match = [&](const shader_io& vertex_attrib) -> bool
      {
         return are_attribs_matching(vbo_attrib, vertex_attrib);
      };
      const auto attrib_it = std::ranges::find_if(shader_inputs, is_attrib_match);
      if (attrib_it == std::end(shader_inputs))
      {
         return std::nullopt;
      }
      return attrib_it->m_location;
   }

} // namespace {}


ubo_segment::ubo_segment(const int byte_count, const std::string& description)
   : m_id(id::create())
   , m_sizeof(get_aligned_ubo_sizeof(byte_count))
   , m_description(description)
{ }

auto ubo_segment::get_byte_count() const -> int
{
   return m_sizeof;
}


ssbo_segment::ssbo_segment(const int byte_count, const std::string& description)
   : m_id(id::create())
   , m_sizeof(get_aligned_ubo_sizeof(byte_count))
   , m_description(description)
{ }


auto ssbo_segment::get_byte_count() const -> int
{
   return m_sizeof;
}


vbo_class_member::vbo_class_member(const data_type data_type, const std::string& attrib_name, const bool normalized)
   : m_type(data_type)
   , m_attrib_name(attrib_name)
   , m_normalized(normalized)
{ }


auto vbo_class_member::get_byte_count() const -> int
{
   return get_data_type_size(m_type);
}


vbo_class::vbo_class(const std::vector<vbo_class_member>& attributes, const int element_count)
   : m_attributes(attributes)
   , m_element_count(element_count)
{ }


auto vbo_class::element_byte_count() const -> int
{
   int byte_count = 0;
   for (const auto& attrib : m_attributes)
      byte_count += attrib.get_byte_count();
   return byte_count;
}


auto vbo_class::get_byte_count() const -> int
{
   return m_element_count * this->element_byte_count();
}


vbo_segment::vbo_segment(const std::vector<vbo_class>& classes)
   : m_id(id::create())
   , m_classes(classes)
{ }


auto vbo_segment::get_byte_count() const -> int
{
   int byte_count = 0;
   for (const vbo_class& class_el : m_classes)
      byte_count += class_el.get_byte_count();
   
   return get_aligned_ubo_sizeof(byte_count);
}


auto vbo_segment::get_aos_stride() const -> int
{
   int byte_count = 0;
   for (const auto& class_el : m_classes)
      byte_count += class_el.element_byte_count();
   return byte_count;
}


auto vbo_segment::get_vbo_class_relative_offset(const int class_index) const -> int
{
   int offset = 0;
   for(int i=0; i<class_index; ++i)
   {
      offset += m_classes[i].get_byte_count();
   }
   return offset;
}


auto segment_type::get_byte_count() const -> int
{
   auto get_segment_size = []<typename T>(const T & segment_alternative)
   {
      return segment_alternative.get_byte_count();
   };
   return std::visit(get_segment_size, *this);
}


auto segment_type::get_id() const -> id
{
   auto get_segment_size = []<typename T>(const T & segment_alternative)
   {
      return segment_alternative.m_id;
   };
   return std::visit(get_segment_size, *this);
}


buffer::buffer(const usage_pattern usage, std::vector<segment_type>&& segments)
   : m_id(id::create())
   , m_usage_pattern(usage)
   , m_segments(std::move(segments))
{
   glCreateBuffers(1, &m_buffer_opengl_id);

   // first init with glNamedBufferData with null and the usage pattern
   // then upload the index buffer with glNamedBufferSubData

   // log::info("created buffer with {}", get_human_size(this->get_byte_count()));
   glNamedBufferData(this->m_buffer_opengl_id, this->get_byte_count(), nullptr, get_gl_usage_pattern(usage));
}


auto buffer::get_byte_count() const -> int
{
   int byte_count = 0;
   for (const segment_type& segment : m_segments)
   {
      byte_count += segment.get_byte_count();
   }
   return byte_count;
}


auto buffer::is_segment_in_buffer(const id target_segment_id) const -> bool
{
   const auto pred = [&](const segment_type& segment)
   {
      return segment.get_id() == target_segment_id;
   };
   return std::ranges::any_of(m_segments, pred);
}


auto buffer::get_segment_offset(const id segment_id) const -> int
{
   int byte_count = 0;
   for(const auto& segment : m_segments)
   {
      if(segment.get_id() == segment_id)
      {
         return byte_count;
      }
      byte_count += segment.get_byte_count();
   }
   log::error("couldn't find segment");
   std::terminate();
}


auto buffer::get_segment_size(const id segment_id) const -> int
{
   for (const auto& segment : m_segments)
   {
      if (segment.get_id() == segment_id)
      {
         return segment.get_byte_count();;
      }
   }
   log::error("couldn't find segment");
   std::terminate();
}


auto buffer::get_opt_segment_ref(
   const id segment_id
) const -> std::optional<std::reference_wrapper<const segment_type>>
{
   for (const auto& segment : m_segments)
   {
      if (segment.get_id() == segment_id)
      {
         return segment;
      }
   }
   return std::nullopt;
}


auto buffer::get_opt_vbo_ref(
   const id segment_id
) const -> std::optional<std::reference_wrapper<const vbo_segment>>
{
   const std::optional<std::reference_wrapper<const segment_type>> segment_var = this->get_opt_segment_ref(segment_id);
   if(segment_var.has_value() == false)
   {
      return std::nullopt;
   }
   const segment_type& segment_ref = (*segment_var).get();
   return std::get<vbo_segment>(segment_ref);
}


rect_index_buffer::rect_index_buffer(const int rect_count)
   : m_id(id::create())
   , m_rect_count(rect_count)
{
   glCreateBuffers(1, &m_opengl_id);

   std::vector<unsigned int> indices;
   indices.reserve(6 * rect_count);
   for (int i = 0; i < rect_count; ++i)
   {
      indices.push_back(0 + i * 4);
      indices.push_back(1 + i * 4);
      indices.push_back(2 + i * 4);
      indices.push_back(0 + i * 4);
      indices.push_back(2 + i * 4);
      indices.push_back(3 + i * 4);
   }
   constexpr usage_pattern index_buffer_usage_pattern = usage_pattern::static_draw;
   glNamedBufferData(m_opengl_id, this->get_byte_count(), indices.data(), get_gl_usage_pattern(index_buffer_usage_pattern));
}


auto rect_index_buffer::get_byte_count() const -> int
{
   const int index_count = 6 * m_rect_count;
   const int byte_count = static_cast<int>(sizeof(unsigned int) * index_count);
   return byte_count;
}


buffers::buffers(const int rect_count)
   : m_rect_index_buffer(rect_count)
{ }


auto buffers::create_buffer(
   std::vector<segment_type>&& segments,
   const usage_pattern usage
) -> std::vector<id>
{
   const buffer& ref = m_buffers.emplace_back(usage, std::move(segments));

   std::vector<id> segment_ids;
   for(const segment_type& segment : ref.m_segments)
   {
      segment_ids.push_back(segment.get_id());
   }
   return segment_ids;
}


auto sfn::buffers::get_buffer_ref(const id buffer_id) const -> const buffer&
{
   const auto pred = [&](const buffer& boof)
   {
      return boof.m_id == buffer_id;
   };
   return *std::ranges::find_if(m_buffers, pred);
}


auto buffers::get_single_buffer_ref() const -> const buffer&
{
   sfn_assert(m_buffers.size() == 1);
   return m_buffers.front();
}


auto buffers::get_buffer_ref_from_segment_id(const id segment_id) const -> const buffer&
{
   for(const buffer& buffer : m_buffers)
   {
      const auto segment = buffer.get_opt_segment_ref(segment_id);
      if(segment.has_value())
      {
         return buffer;
      }
   }
   log::error("couldn't find buffer");
   std::terminate();
}


auto buffers::get_segment_size(const id segment_id) const -> size_t
{
   for (const buffer& buffer : m_buffers)
   {
      const std::optional<std::reference_wrapper<const segment_type>> ref = buffer.get_opt_segment_ref(segment_id);
      if (ref.has_value())
      {
         return (*ref).get().get_byte_count();
      }
   }
   log::error("couldn't find buffer");
   std::terminate();
}


auto buffers::get_vbo_ref(const id vbo_segment_id) const -> const vbo_segment&
{
   for (const auto& buffer : m_buffers)
   {
      const std::optional<std::reference_wrapper<const vbo_segment>> ref = buffer.get_opt_vbo_ref(vbo_segment_id);
      if (ref.has_value())
      {
         return (*ref).get();
      }
   }
   log::error("couldn't find buffer");
   std::terminate();
}


auto buffers::get_buffer_opengl_id_from_buffer(const id buffer_id) const -> GLuint
{
   return this->get_buffer_ref(buffer_id).m_buffer_opengl_id;
}

auto buffers::upload_ubo(const id ubo_id, const std::span<const std::byte> data) const -> void
{
   const buffer& boof = get_buffer_ref_from_segment_id(ubo_id);
   const GLuint buffer_opengl_id = boof.m_buffer_opengl_id;
   const int offset = boof.get_segment_offset(ubo_id);;
   glNamedBufferSubData(buffer_opengl_id, offset, data.size_bytes(), data.data());
}


auto buffers::upload_ssbo_impl(const id ssbo_id, const std::span<const std::byte> data,
   const int byte_offset) const -> void
{
   const buffer& boof = get_buffer_ref_from_segment_id(ssbo_id);
   const GLuint buffer_opengl_id = boof.m_buffer_opengl_id;
   const int offset = boof.get_segment_offset(ssbo_id) + byte_offset;
   glNamedBufferSubData(buffer_opengl_id, offset, data.size_bytes(), data.data());
}


auto buffers::upload_ssbo(
   const id ssbo_id, const std::span<const std::byte> data, const int offset
) const -> void
{
   upload_ssbo_impl(ssbo_id, data, offset);
}


auto sfn::buffers::upload_vbo(const id vbo_id, const std::span<const std::byte> data) const -> void
{
   const buffer& buffer_ref = this->get_buffer_ref_from_segment_id(vbo_id);
   const vbo_segment& vbo = this->get_vbo_ref(vbo_id);
   const GLuint buffer_opengl_id = buffer_ref.m_buffer_opengl_id;
   const int segment_offset = buffer_ref.get_segment_offset(vbo_id);
   constexpr int class_index = 0; // class index for offset is always 0 // TODO actually not true with SOA upload
   const int relative_offset = vbo.get_vbo_class_relative_offset(class_index);
   const int offset = segment_offset + relative_offset;
   glNamedBufferSubData(buffer_opengl_id, offset, data.size_bytes(), data.data());
}


auto sfn::is_shader_and_vbo_aos_compat(
   const std::vector<shader_io>& shader_inputs,
   const std::vector<vbo_class_member>& vbo_segment_attributes
) -> bool
{
   if(shader_inputs.size() != vbo_segment_attributes.size())
   {
      return false;
   }

   const auto pred = [&](const vbo_class_member& vbo_attrib)
   {
      return get_matching_shader_attrib_location(vbo_attrib, shader_inputs).has_value();
   };
   return std::ranges::all_of(vbo_segment_attributes, pred);
}


sfn::vao::vao(
   const buffers& buffers,
   const id vbo_id,
   const shader_program& shader
)
   : vao(buffers, vbo_id, shader, std::nullopt)
{}


sfn::vao::vao(
   const buffers& buffers,
   const id vbo_id,
   const shader_program& shader,
   const std::optional<GLuint> index_buffer_opengl_id
)
{
   glCreateVertexArrays(1, &m_vao_id);

   const buffer& buffer_ref = buffers.get_single_buffer_ref();
   const vbo_segment& vbo_segment = buffers.get_vbo_ref(vbo_id);
   const auto& shader_inputs = shader.get_vertex_inputs();
   const int offset_to_vbo = buffer_ref.get_segment_offset(vbo_id);

   // AOS:
   {
      const int stride = vbo_segment.get_aos_stride();
      //const int global_offset = offset_to_vbo;
      constexpr int bindingindex = 1; // doesn't matter
      glVertexArrayVertexBuffer(m_vao_id, bindingindex, buffer_ref.m_buffer_opengl_id, offset_to_vbo, stride);
      if(index_buffer_opengl_id.has_value())
      {
         glVertexArrayElementBuffer(m_vao_id, index_buffer_opengl_id.value());
      }

      int offset = 0; // this offset is like offset(class, m_member)
      for (const vbo_class_member& vbo_attrib : vbo_segment.m_classes[0].m_attributes)
      {
         const std::optional<int> opt_location = get_matching_shader_attrib_location(vbo_attrib, shader_inputs);

         // TODO this isn't ideal. A shader could still use a VBO even if it doesn't *completely* match the attributes.
         // A partial match works just fine. So maybe check compatibility on binding? Or pass a list of shaders?
         if(opt_location.has_value() == false)
         {
            // log::warn("VBO Attribute \"{}\" wasn't found in shader. Ignoring...", vbo_attrib.m_attrib_name);
            offset += get_data_type_size(vbo_attrib.m_type);
            continue;
         }
         const int location = opt_location.value();
         const attrib_format_breakdown breakdown = get_attrib_format_breakdown(vbo_attrib.m_type);
         glEnableVertexArrayAttrib(m_vao_id, location);

         if(vbo_attrib.m_type == data_type::ui)
         {
            // integer attributes don't work on intel
            glVertexArrayAttribIFormat(m_vao_id, location, breakdown.m_count, breakdown.m_type, offset);
            log::error("Don't use integer vertex attributes. Bugged on intel drivers");
            std::terminate();
         }
         else
         {
            glVertexArrayAttribFormat(m_vao_id, location, breakdown.m_count, breakdown.m_type, vbo_attrib.m_normalized, offset);
         }

         glVertexArrayAttribBinding(m_vao_id, location, bindingindex);

         offset += get_data_type_size(vbo_attrib.m_type);
      }
   }
   return;


   // SOA :
   //std::optional<int> last_index;
   //int bindingindex = 0; // start value doesn't matter apparently // TODO still valid? see opengl.md
   //for (int segment_index = 0; segment_index < vbo_segment.m_classes.size(); ++segment_index)
   //{
   //   const vbo_class& class_x = vbo_segment.m_classes[segment_index];
   //   const std::optional<int> fitting_shader_attrib_index = get_matching_shader_attrib_location(class_x.m_attributes.front(), shader_inputs);

   //   if (last_index.has_value() == false)
   //   {
   //      if (fitting_shader_attrib_index.has_value() == false)
   //      {
   //         // No problem if no first has been found yet
   //         continue;
   //      }
   //   }
   //   else
   //   {
   //      if (fitting_shader_attrib_index.has_value() == false)
   //      {
   //         log::error("not directly after last!");
   //         std::terminate();
   //      }
   //   }
   //   
   //   //const buffer& buffer_ref = buffers.get_buffer_ref(only_buffer_id);
   //   //const int offset_to_vbo = buffer_ref.get_segment_offset(vbo_segment.m_id);
   //   const int relative_offset = vbo_segment.get_vbo_class_relative_offset(segment_index);

   //   {
   //      const int stride = class_x.element_byte_count();
   //      glVertexArrayVertexBuffer(m_vao_id, bindingindex, buffer_ref.m_buffer_opengl_id, offset_to_vbo+relative_offset, stride);
   //   }
   //   glVertexArrayElementBuffer(m_vao_id, buffers.m_rect_index_buffer.m_opengl_id);

   //   {
   //      const vbo_class_member& vbo_attrib = class_x.m_attributes.front();
   //      const int location = get_matching_shader_attrib_location(vbo_attrib, shader_inputs).value();
   //      const attrib_format_breakdown breakdown = get_attrib_format_breakdown(vbo_attrib.m_type);
   //      glEnableVertexArrayAttrib(m_vao_id, location);
   //      glVertexArrayAttribFormat(m_vao_id, location, breakdown.m_count, breakdown.m_type, vbo_attrib.m_normalized, 0); // SOA implies this offset=0
   //      glVertexArrayAttribBinding(m_vao_id, location, bindingindex);

   //      ++bindingindex;
   //   }

   //   last_index = fitting_shader_attrib_index.value();
   //}
}


auto sfn::vao::bind() const -> void
{
   glBindVertexArray(m_vao_id);
}

