#include "timing_provider.h"

namespace
{
   [[nodiscard]] auto get_tp_diff_in_seconds(
      const std::chrono::high_resolution_clock::time_point& first,
      const std::chrono::high_resolution_clock::time_point& second
   ) -> float
   {
      const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(second - first).count();
      const float seconds = static_cast<float>(ns) / 1'000'000'000.0f;
      return seconds;
   }
}


sfn::timing_provider::timing_provider()
   : m_t0(std::chrono::high_resolution_clock::now())
   , m_last_frame_t(m_t0)
   , m_t_last_fps(m_t0)
{

}


auto sfn::timing_provider::mark_frame_end() -> void
{
   const auto now = std::chrono::high_resolution_clock::now();

   ++m_fps_frame_count;

   // frame duration
   m_next_timing_info.m_last_frame_duration = static_cast<float>(get_tp_diff_in_seconds(m_last_frame_t, now));
   m_last_frame_t = now;

   // steady time
   m_next_timing_info.m_steady_time = get_tp_diff_in_seconds(m_t0, now);;

   // fps
   const float seconds_since_last_fps_query = get_tp_diff_in_seconds(m_t_last_fps, now);
   if(seconds_since_last_fps_query > 1.0)
   {
      const int fps = static_cast<int>(std::round(m_fps_frame_count / seconds_since_last_fps_query));
      m_t_last_fps = now;
      m_fps_frame_count = 0;
      m_next_timing_info.m_last_fps = fps;
   }
   else
   {
      m_next_timing_info.m_last_fps.reset();
   }
}


auto sfn::timing_provider::get_timing_info() const -> timing_info
{
   return m_next_timing_info;
}

