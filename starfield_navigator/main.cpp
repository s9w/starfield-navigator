#include <vector>
#include <fstream>

#include <glm/vec3.hpp>
#include <glm/geometric.hpp>
#include <imgui.h>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <rng.h>

#include "tools.h"

#pragma warning(push, 0)   
#include <glm/trigonometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#pragma warning(pop)

#include "engine.h"
#include "graph.h"


using namespace sfn;


[[nodiscard]] constexpr auto c4d_convert(const glm::vec3& in) -> glm::vec3
{
   return glm::vec3{ in[0], in[2], in[1] };
}


[[nodiscard]] auto get_and_delete_cam_info(std::vector<sfn::system>& systems) -> cam_info
{
   const auto generator = [&](const std::string& target)
   {
      const auto camera_pred = [&](const sfn::system& sys)
      {
         return sys.m_name.starts_with(target);
      };
      const auto it = std::ranges::find_if(systems, camera_pred);
      if (it == std::end(systems))
         std::terminate();
      const glm::vec3 copy = it->m_position;
      systems.erase(it);
      return copy;
   };
   const glm::vec3 cam0 = generator("cam0");
   const glm::vec3 cam1 = generator("cam1");
   const glm::vec3 cam_up = generator("cam_up");
   const glm::vec3 cam_front = generator("cam_front");
   return cam_info{
      .m_cs = cs(glm::normalize(cam_front - cam0), glm::normalize(cam_up - cam0)),
      .m_cam_pos0 = cam0,
      .m_cam_pos1 = cam1
   };
}

struct alignment_stars
{
   glm::vec3 m_sol;
   glm::vec3 m_alpha_centauri;
   glm::vec3 m_porrima;
};


[[nodiscard]] auto apply_trafo(
   const glm::mat4& trafo,
   const glm::vec3& pos
) -> glm::vec3
{
   glm::vec4 result = trafo * glm::vec4{ pos, 1.0f };
   result /= result[3];
   return glm::vec3{ result };
}


auto get_trafo_a(
   const alignment_stars& source,
   const alignment_stars& target
) -> glm::mat4
{
   glm::mat4 trafo(1.0f);

   // 1. scale to porrima
   const float scale = glm::length(target.m_porrima) / glm::length(source.m_porrima);
   trafo = glm::scale(trafo, glm::vec3{ scale, scale, scale });

   // 3. Rotate Alpha Centauri old to new
   {
      const glm::vec3 axis = glm::normalize(target.m_porrima);
      const float angle = glm::orientedAngle(glm::normalize(source.m_alpha_centauri), glm::normalize(target.m_alpha_centauri), axis);
      trafo = glm::rotate(trafo, angle, axis);
   }

   // 2. Rotate porrima old to new
   {
      const glm::vec3 axis = glm::cross(glm::normalize(source.m_porrima), glm::normalize(target.m_porrima));
      const float angle = glm::orientedAngle(glm::normalize(source.m_porrima), glm::normalize(target.m_porrima), axis);
      trafo = glm::rotate(trafo, angle, axis);
   }
   return trafo;
}


auto get_trafo_b(
   const alignment_stars& source,
   const alignment_stars& target
) -> glm::mat4
{
   glm::mat4 trafo(1.0f);

   // 3. Rotate Alpha Centauri old to new
   {
      const glm::vec3 axis = glm::normalize(target.m_porrima);
      const float angle = glm::orientedAngle(glm::normalize(source.m_alpha_centauri), glm::normalize(target.m_alpha_centauri), axis);
      trafo = glm::rotate(trafo, angle, axis);
   }
   return trafo;
}


auto get_trafo(
   const alignment_stars& source,
   const alignment_stars& target
) -> glm::mat4
{
   const glm::mat4 trafo_a = get_trafo_a(source, target);

   const alignment_stars new_source = alignment_stars{
      .m_sol = apply_trafo(trafo_a, source.m_sol),
      .m_alpha_centauri = apply_trafo(trafo_a, source.m_alpha_centauri),
      .m_porrima = apply_trafo(trafo_a, source.m_porrima)
   };
   const glm::mat4 trafo_b = get_trafo_b(new_source, target);
   return trafo_b * trafo_a;
}


struct real {
   std::string m_catalog_name;
   glm::vec3 m_coordinates;
   std::string m_proper_name;
};

struct real_universe{
   std::vector<real> m_stars;
   [[nodiscard]] auto get_pos_by_name(const std::string& name) const -> glm::vec3
   {
      return m_stars[get_index_by_name(name)].m_coordinates;
   }
   [[nodiscard]] auto get_name_by_index(const int i) const -> std::string
   {
      std::string result = m_stars[i].m_catalog_name;
      if (m_stars[i].m_proper_name.empty() == false)
         result += fmt::format(" ({})", m_stars[i].m_proper_name);
      return result;
   }
   [[nodiscard]] auto get_index_by_name(const std::string& either_name) const -> int
   {
      const auto pred = [&](const real& r) {
         return r.m_proper_name == either_name || r.m_catalog_name == either_name;
      };
      const auto it = std::ranges::find_if(m_stars, pred);
      return static_cast<int>(std::distance(std::cbegin(m_stars), it));
   }
};


auto get_real_entries() -> real_universe
{
   std::ifstream input("50lys.txt");
   const bool b = input.good();
   real_universe result;
   for (std::string line; getline(input, line); )
   {
      if(line.starts_with("#"))
         continue;
      const auto split = get_split_string(line, ";");
      const std::string catalog_name = get_trimmed_str(split[0]);

      // const std::string eq_coordinates_str = split[1];
      
      // const std::string classification = split[4];
      // const std::string visual_mag = split[5];
      // const std::string abs_mag = split[6];
      // const std::string parallax = split[7];
      if(catalog_name == "Sun")
         continue;

      const float dist = std::stof(split[8]);
      const float galactic_l = glm::radians(std::stof(split[2]));
      const float galactic_b = glm::radians(std::stof(split[3]));
      const float theta = glm::radians(90.0f) - galactic_b;
      const float x = dist * std::cos(galactic_l) * std::sin(theta);
      const float y = dist * std::sin(galactic_l) * std::sin(theta);
      
      const float z = dist * std::cos(theta);
      

      const std::string gliese_number = get_trimmed_str(split[9]);
      const std::string proper_name = get_trimmed_str(split[10]);
      if (proper_name == "Porrima")
         int stop = 0;
      result.m_stars.push_back(
         real{
            .m_catalog_name = catalog_name,
            .m_coordinates = glm::vec3{x, y, z},
            .m_proper_name = proper_name
         }
      );
   }

   return result;
}

auto get_name_and_size(std::string name, system_size& size_target) -> std::string
{
   if (name.starts_with("big_"))
   {
      size_target = system_size::big;
      name = name.substr(4);
   }
   else if (name.starts_with("small_"))
   {
      size_target = system_size::small;
      name = name.substr(6);
   }
   else
      std::terminate();
   return name;
}


auto get_starfield_universe() -> universe
{
   const real_universe real_universe = get_real_entries();

   universe starfield_universe{};
   std::ifstream input("system_data.txt");

   for (std::string line; getline(input, line); )
   {
      const std::vector<std::string> split = get_split_string(line, ";");
      system_size size;
      const std::string name = get_name_and_size(split[0], size);
      const std::string astronomic_name = split[4];
      
      const float x = static_cast<float>(std::stod(get_trimmed_str(split[1])));
      const float y = static_cast<float>(std::stod(get_trimmed_str(split[2])));
      const float z = static_cast<float>(std::stod(get_trimmed_str(split[3])));
      starfield_universe.m_systems.emplace_back(c4d_convert(glm::vec3{ x, y, z }), name, astronomic_name, size);
   }

   auto original_sun_pos = starfield_universe.get_position_by_name("SOL");

   // 1. Sun to center
   {
      
      for (sfn::system& sys : starfield_universe.m_systems)
      {
         sys.m_position -= original_sun_pos;
      }
   }
   // 2. trafo
   {
      const auto trafo = get_trafo(
         alignment_stars{
            .m_sol = starfield_universe.get_position_by_name("SOL"),
            .m_alpha_centauri = starfield_universe.get_position_by_name("ALPHA CENTAURI"),
            .m_porrima = starfield_universe.get_position_by_name("PORRIMA")
         },
         alignment_stars{
            .m_sol = glm::vec3{},
            .m_alpha_centauri = real_universe.get_pos_by_name("Alpha Centauri"),
            .m_porrima = real_universe.get_pos_by_name("Porrima")
         }
      );

      for (sfn::system& sys : starfield_universe.m_systems)
      {
         sys.m_position = apply_trafo(trafo, sys.m_position);
      }
   }


   // check
   {
      const auto candidates_for_fictional = [&](const std::string& fictional_name)
      {
         const glm::vec3 pos0 = starfield_universe.get_position_by_name(fictional_name);
         std::vector<int> closest;
         for (int i = 0; i < std::ssize(real_universe.m_stars); ++i)
            closest.push_back(i);
         const auto pred = [&](const int i, const int j)
         {
            const float dist_i = glm::distance(pos0, real_universe.m_stars[i].m_coordinates);
            const float dist_j = glm::distance(pos0, real_universe.m_stars[j].m_coordinates);
            return dist_i < dist_j;
         };
         std::ranges::sort(closest, pred);
         for (int i = 0; i < 2; ++i)
         {
            const float dist = glm::distance(pos0, real_universe.m_stars[closest[i]].m_coordinates);
            const float relative_error = dist / glm::length(real_universe.m_stars[closest[i]].m_coordinates);
            printf(fmt::format("{}: {}, dist: {:.1f}, rel error: {:.1f}\n", i, real_universe.get_name_by_index(closest[i]), dist, 100.0f* relative_error).c_str());
         }
      };
      const auto candidates_for_real = [&](const std::string& real_name)
      {
         const glm::vec3 pos0 = real_universe.get_pos_by_name(real_name);
         std::vector<int> closest;
         for (int i = 0; i < std::ssize(starfield_universe.m_systems); ++i)
            closest.push_back(i);
         const auto pred = [&](const int i, const int j)
         {
            const float dist_i = glm::distance(pos0, starfield_universe.m_systems[i].m_position);
            const float dist_j = glm::distance(pos0, starfield_universe.m_systems[j].m_position);
            return dist_i < dist_j;
         };
         std::ranges::sort(closest, pred);
         for (int i = 0; i < 2; ++i)
         {
            const float dist = glm::distance(pos0, starfield_universe.m_systems[closest[i]].m_position);
            const float relative_error = dist / glm::length(starfield_universe.m_systems[closest[i]].m_position);
            printf(fmt::format("{}: {}, dist: {:.1f}, rel error: {:.1f}\n", i, starfield_universe.m_systems[closest[i]].m_name, dist, 100.0f * relative_error).c_str());
         }
      };

      // candidates_for_fictional("PORRIMA");
      // candidates_for_fictional("ALPHA CENTAURI");
      // candidates_for_fictional("NARION");
      // candidates_for_fictional("VOLII");
      // candidates_for_fictional("JAFFA");
      // candidates_for_fictional("CHEYENNE");
      candidates_for_fictional("User 12");
      // candidates_for_real("Porrima");
      // candidates_for_real("Alpha Centauri");
      candidates_for_real("Vega");
      // candidates_for_real("Fomalhaut");
      // candidates_for_real("Rana");
      // candidates_for_real("70 Ophiuchi");
      candidates_for_real("Wolf 359");
      candidates_for_real("Barnard");
      candidates_for_real("Sirius");
      candidates_for_real("Altair");
      candidates_for_real("61 Cygni");
      candidates_for_real("Mu Herculis");
      candidates_for_real("Luyten's Star");
      candidates_for_real("Procyon");
   }


   // sort from left to right
   const auto pred = [](const sfn::system& a, const sfn::system& b){
      return a.m_position.x < b.m_position.x;
   };
   std::ranges::sort(starfield_universe.m_systems, pred);

   starfield_universe.m_cam_info = get_and_delete_cam_info(starfield_universe.m_systems);

   auto dist_report = [&](const std::string& name_a, const std::string& name_b){
      const float dist = glm::distance(
         starfield_universe.get_position_by_name(name_a),
         starfield_universe.get_position_by_name(name_b)
      );
      printf(fmt::format("dist {}<->{}: {:.2f} LY\n", name_a, name_b, dist).c_str());
   };
   dist_report("User 64", "User 65");
   dist_report("User 62", "User 63");

   // {
   //    const float sufficient_jump_range = get_absolute_min_jump_range(starfield_universe);
   //    printf(fmt::format("sufficient_jump_range: {:.1f}\n", sufficient_jump_range).c_str());
   // }

   {
      const auto closest = get_closest_distances_for_all(starfield_universe);
      printf(fmt::format("avg of closest: {:.2f}\n", get_average(closest)).c_str());
      printf(fmt::format("max of closest: {:.2f}\n", *std::ranges::max_element(closest)).c_str());
   }

   starfield_universe.print_info();
   return starfield_universe;
}


// Disable console window in release mode
#if defined(_DEBUG) || defined(SHOW_CONSOLE)
auto main(int /*argc*/, char* /*argv*/) -> int
#else
auto CALLBACK WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/) -> int
#endif
{
   engine engine(
      config{
         .res_x = 1280, .res_y = 720,
         .opengl_major_version = 4, .opengl_minor_version = 5,
         .vsync = true,
         .window_title = fmt::format("Starfield navigator {}", sfn_version_string)
      },
      get_starfield_universe()
   );
   engine.draw_loop();

   return 0;
}

