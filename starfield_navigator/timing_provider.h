#pragma once

#include <chrono>


namespace sfn
{
   struct timing_info
   {
      float m_steady_time{};
      float m_last_frame_duration{1.0f/60.0f}; // better than 0
      std::optional<int> m_last_fps;
   };

   // yields: time (for sin etc), fps, delta frame time
   struct timing_provider
   {
   private:
      std::chrono::high_resolution_clock::time_point m_t0; // steady time
      std::chrono::high_resolution_clock::time_point m_last_frame_t; // for delta frame time
      std::chrono::high_resolution_clock::time_point m_t_last_fps; // for fps
      int m_fps_frame_count = 0;
      timing_info m_next_timing_info{};

   public:
      explicit timing_provider();
      auto mark_frame_end() -> void;

      [[nodiscard]] auto get_timing_info() const -> timing_info;
   };

}
