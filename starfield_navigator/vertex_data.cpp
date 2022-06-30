#include "vertex_data.h"


auto sfn::position_vertex_data::for_each(vertex_data_visitor& visitor) -> void
{
   visitor.visit<decltype(m_position)>("position");
}
