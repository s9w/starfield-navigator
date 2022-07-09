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
#include "reality.h"

#pragma warning(push, 0)   
#include <biteopt.h>
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


struct real {
   std::string m_hip;
   glm::vec3 m_coordinates;
};

struct real_universe{
   std::vector<real> m_stars;
   [[nodiscard]] auto get_pos_by_hip(const std::string& hip) const -> glm::vec3
   {
      return m_stars[get_index_by_name(hip)].m_coordinates;
   }
   [[nodiscard]] auto get_name_by_index(const int i) const -> std::string
   {
      std::string result = m_stars[i].m_hip;
      return result;
   }
   [[nodiscard]] auto get_index_by_name(const std::string& hip) const -> int
   {
      const auto pred = [&](const real& r) {
         return r.m_hip == hip;
      };
      const auto it = std::ranges::find_if(m_stars, pred);
      return static_cast<int>(std::distance(std::cbegin(m_stars), it));
   }
};


auto get_real_entries() -> real_universe
{
   // std::ifstream input("cc_hip1997.txt");
   std::ifstream input("cc_hip2007.txt");
   real_universe result;
   for (std::string line; getline(input, line); )
   {
      if(line.starts_with("#"))
         continue;
      const auto split = get_split_string(line, ";");
      const std::string hip = get_trimmed_str(split[0]);
      const galactic_coord galactic{
         .m_l = glm::radians(std::stof(split[1])),
         .m_b = glm::radians(std::stof(split[2])),
         .m_dist = std::stof(split[3])
      };
      // TODO: magnitude

      result.m_stars.push_back(
         real{
            .m_hip = hip,
            .m_coordinates = galactic.get_cartesian()
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


[[nodiscard]] auto get_error(
   const universe& fiction,
   const ::real_universe& real,
   const std::string& fictional_name,
   const std::string& hip
) -> float
{
   const glm::vec3 fiction_pos = fiction.get_position_by_name(fictional_name);
   const glm::vec3 real_pos = real.get_pos_by_hip(hip);
   return glm::distance(fiction_pos, real_pos);
};


[[nodiscard]] auto get_metric(
   const universe& univ,
   const ::real_universe& real
) -> float
{
   std::vector<float> errors;
   errors.reserve(20);
   errors.push_back(get_error(univ, real, "PORRIMA", "61941"));
   errors.push_back(get_error(univ, real, "Luyten's Star", "36208"));
   errors.push_back(get_error(univ, real, "Sirius", "32349"));
   errors.push_back(get_error(univ, real, "61 Virginis", "64924"));
   errors.push_back(get_error(univ, real, "Kapteyn's Star", "24186"));
   errors.push_back(get_error(univ, real, "Xi Bootis", "72659"));
   errors.push_back(get_error(univ, real, "Barnard", "87937"));
   errors.push_back(get_error(univ, real, "Gliese 674", "85523"));
   errors.push_back(get_error(univ, real, "70 Ophiuchi", "88601"));
   errors.push_back(get_error(univ, real, "Delta Pavonis", "99240"));
   errors.push_back(get_error(univ, real, "18 Scorpii", "79672"));
   errors.push_back(get_error(univ, real, "Altair", "97649"));
   errors.push_back(get_error(univ, real, "e Eridani", "15510"));
   errors.push_back(get_error(univ, real, "Wolf 28", "3829"));
   return get_average(errors);
};


[[nodiscard]] auto get_generic_trafo(
   const float x_angle,
   const float y_angle,
   const float z_angle,
   const glm::vec3& scale,
   const glm::vec3& shift
) -> glm::mat4
{
   glm::mat4 trafo(1.0f);
   trafo = glm::translate(trafo, shift);
   trafo = glm::scale(trafo, scale);
   trafo = glm::rotate(trafo, x_angle, glm::vec3{ 1, 0, 0 });
   trafo = glm::rotate(trafo, y_angle, glm::vec3{ 0, 1, 0 });
   trafo = glm::rotate(trafo, z_angle, glm::vec3{ 0, 0, 1 });
   return trafo;
}


auto get_starfield_universe() -> universe
{
   // read_real_stars();

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

   // Sort from left to right before transformation is applied
   const auto pred = [](const sfn::system& a, const sfn::system& b) {
      return a.m_position.x < b.m_position.x;
   };
   std::ranges::sort(starfield_universe.m_systems, pred);




   struct CTestOpt : public CBiteOpt
   {
      const universe* fiction_ref;
      const ::real_universe* real_ref;
      // 0, 1, 2: rotation angles
      // 3, 4, 5: scale factors
      // 6, 7, 8: translation
      CTestOpt()
      {
         updateDims(9);
      }

      void getMinValues(double* const p) const override
      {
         p[0] = 0.0;
         p[1] = 0.0;
         p[2] = 0.0;
         p[3] = 0.01;
         p[4] = 0.01;
         p[5] = 0.01;
         p[6] = -100.0;
         p[7] = -100.0;
         p[8] = -100.0;
      }

      void getMaxValues(double* const p) const override
      {
         p[0] = 2.0*std::numbers::pi_v<double>;
         p[1] = 2.0*std::numbers::pi_v<double>;
         p[2] = 2.0*std::numbers::pi_v<double>;
         p[3] = 10.0;
         p[4] = 10.0;
         p[5] = 10.0;
         p[6] = 100.0;
         p[7] = 100.0;
         p[8] = 100.0;
      }
      [[nodiscard]] static auto get_trafo_from_vector(const double* const p) -> glm::mat4
      {
         return get_generic_trafo(p[0], p[1], p[2], glm::vec3{ p[3], p[4], p[5] }, glm::vec3{ p[6], p[7], p[8] });
      }

      double optcost(const double* const p) override
      {
         universe transformed_universe = *fiction_ref;
         const glm::mat4 trafo = get_trafo_from_vector(p);
         for(sfn::system& elem : transformed_universe.m_systems)
         {
            elem.m_position = apply_trafo(trafo, elem.m_position);
         }
         return static_cast<double>(get_metric(transformed_universe, *real_ref));
      }
   };
   CBiteRnd rnd;
   rnd.init(1); // Needs to be seeded with different values on each run.
   CTestOpt opt;
   opt.fiction_ref = &starfield_universe;
   opt.real_ref = &real_universe;
   opt.init(rnd);
   int i = 0;
   for (; i < 20000; i++)
   {
      opt.optimize(rnd);
      // constexpr double tol = 0.000001;
      constexpr double tol = 0.001;
      if (opt.getBestCost() < tol)
      {
         break;
      }
   }
   printf("IterCount: %i\n", i);
   printf("BestCost: %f\n", opt.getBestCost());
   // for (int p = 0; p < 9; ++p)
   //    fmt::print("best params {}: {:.2f}\n", p, opt.getBestParams()[p]);

   // test best trafo
   const glm::mat4 best_trafo = CTestOpt::get_trafo_from_vector(opt.getBestParams());
   for (sfn::system& sys : starfield_universe.m_systems)
   {
      sys.m_position = apply_trafo(best_trafo, sys.m_position);
   }
   fmt::print("metric with optimized trafo: {:.2f} LY\n",get_metric(starfield_universe, real_universe));


   const auto candidates_for_fictional = [&](const std::string& fictional_name, const std::optional<std::string>& hip = std::nullopt)
   {
      const glm::vec3 pos0 = starfield_universe.get_position_by_name(fictional_name);
      std::vector<int> real_closest;
      for (int i = 0; i < std::ssize(real_universe.m_stars); ++i)
         real_closest.push_back(i);
      const auto pred = [&](const int i, const int j)
      {
         const float dist_i = glm::distance(pos0, real_universe.m_stars[i].m_coordinates);
         const float dist_j = glm::distance(pos0, real_universe.m_stars[j].m_coordinates);
         return dist_i < dist_j;
      };
      std::ranges::sort(real_closest, pred);
      fmt::print("\n");
      for (int i = 0; i < 3; ++i)
      {
         const float dist = glm::distance(pos0, real_universe.m_stars[real_closest[i]].m_coordinates);
         const float relative_error = dist / glm::length(real_universe.m_stars[real_closest[i]].m_coordinates);
         std::string guess_str;
         if (hip.has_value() && real_universe.get_index_by_name(*hip) == real_closest[i])
            guess_str = "CHOICE";
         fmt::print(
            "{}: {}, dist: {:.1f}, rel error: {:.1f} {}\n",
            i, real_universe.get_name_by_index(real_closest[i]), dist, 100.0f* relative_error, guess_str
         );
      }
   };
   // candidates_for_fictional("User 29"); // no good match
   // candidates_for_fictional("User 17"); // no good match
   // candidates_for_fictional("User 30"); // no good match
   // candidates_for_fictional("User 40"); // no good match
   // candidates_for_fictional("User 35"); // no good match
   // candidates_for_fictional("User 50"); // no good match
   // candidates_for_fictional("User 56"); // no good match
   // candidates_for_fictional("User 52"); // no good match
   // candidates_for_fictional("User 24"); // no good match
   // candidates_for_fictional("User 20"); // no good match
   // all right ones are awful
   candidates_for_fictional("User 27"); // two good but hidden matches
   //candidates_for_fictional("User 61"); // not sure which one
   // candidates_for_fictional("User 56"); // this is Wolf 359
   
   const auto error_report = [&](const std::string& fictional_name, const std::string& hip)
   {
      const glm::vec3 fiction_pos = starfield_universe.get_position_by_name(fictional_name);
      const glm::vec3 real_pos = real_universe.get_pos_by_hip(hip);
      const float dist = glm::distance(fiction_pos, real_pos);
      fmt::print("{:<15} dist: {:>3.1f} LY\n", fictional_name, dist);
   };

   error_report("Luyten's Star", "36208");
   error_report("Sirius", "32349");
   error_report("61 Virginis", "64924");
   error_report("Kapteyn's Star", "24186");
   error_report("Xi Bootis", "72659");
   error_report("Barnard", "87937");
   error_report("Gliese 674", "85523");
   error_report("70 Ophiuchi", "88601");
   error_report("Delta Pavonis", "99240");
   error_report("18 Scorpii", "79672");
   error_report("Altair", "97649");
   error_report("e Eridani", "15510");
   error_report("Wolf 28", "3829");


   starfield_universe.m_cam_info = get_and_delete_cam_info(starfield_universe.m_systems);

   // auto dist_report = [&](const std::string& name_a, const std::string& name_b){
   //    const float dist = glm::distance(
   //       starfield_universe.get_position_by_name(name_a),
   //       starfield_universe.get_position_by_name(name_b)
   //    );
   //    fmt::print("dist {}<->{}: {:.2f} LY\n", name_a, name_b, dist);
   // };
   // dist_report("User 64", "User 65");
   // dist_report("User 62", "User 63");

   // {
   //    const float sufficient_jump_range = get_absolute_min_jump_range(starfield_universe);
   //    fmt::print("sufficient_jump_range: {:.1f}\n", sufficient_jump_range);
   // }

   // {
   //    const auto closest = get_closest_distances_for_all(starfield_universe);
   //    fmt::print("avg of closest: {:.2f}\n", get_average(closest));
   //    fmt::print("max of closest: {:.2f}\n", *std::ranges::max_element(closest));
   // }

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

