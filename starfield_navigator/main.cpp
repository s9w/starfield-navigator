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
   // std::ifstream input("cc_hip2007.txt");
   std::ifstream input("cc_hyg.txt");
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
   errors.reserve(50);

   for (const sfn::system& system : univ.m_systems)
   {
      if (system.m_astronomic_name.empty())
         continue;
      errors.push_back(get_error(univ, real, system.m_astronomic_name, system.m_catalog_lookup));
   }
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

[[nodiscard]] auto get_system_size(const std::string& size_str) -> system_size
{
   if (size_str == "big")
      return system_size::big;
   if (size_str == "small")
      return system_size::small;
   std::terminate();
}


auto get_starfield_universe() -> universe
{
   // read_real_stars();

   const real_universe real_universe = get_real_entries();

   universe starfield_universe{};
   std::ifstream input("system_data.txt");

   bool reading = true;
   std::unordered_map<std::string, glm::vec3> read_data;
   for (std::string line; getline(input, line); )
   {
      {
         const size_t comment_begin = line.find('#');
         if(comment_begin != std::string::npos)
         {
            line = line.substr(0, comment_begin);
         }
      }
      line = get_trimmed_str(line);

      if (reading == true && line.empty())
      {
         reading = false;
         continue;
      }
      if (reading)
      {
         const std::vector<std::string> split = get_split_string(line, ";");
         const std::string name = split[0];
         const float x = static_cast<float>(std::stod(get_trimmed_str(split[1])));
         const float y = static_cast<float>(std::stod(get_trimmed_str(split[2])));
         const float z = static_cast<float>(std::stod(get_trimmed_str(split[3])));
         read_data.emplace(name, c4d_convert(glm::vec3{ x, y, z }));
      }
      else
      {
         const std::vector<std::string> key_value = get_split_string(line, ":");
         
         const std::vector<std::string> values = get_split_string(key_value[1], ";");
         std::string astronomical_name;
         std::string catalog_entry;
         if (values[2].empty() == false)
         {
            const auto astro_split = get_split_string(values[2], "_");
            if (astro_split.size() == 1)
               std::terminate();
            astronomical_name = astro_split[0];
            catalog_entry = astro_split[1];
            catalog_entry = get_split_string(catalog_entry, " ")[1];
         }


         const std::string starfield_name = values[1];

         std::string name = key_value[0];
         const glm::vec3 pos = read_data.at(name);
         if (starfield_name.empty() == false)
            name = starfield_name;
         starfield_universe.m_systems.emplace_back(pos, name, astronomical_name, catalog_entry, get_system_size(values[0]));
      }
   }
   int stop = 0;
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
   for (; i < 10000; i++)
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
   for (int p = 0; p < 9; ++p)
      fmt::print("best params {}: {:.2f}\n", p, opt.getBestParams()[p]);

   // test best trafo
   const glm::mat4 best_trafo = CTestOpt::get_trafo_from_vector(opt.getBestParams());
   for (sfn::system& sys : starfield_universe.m_systems)
   {
      sys.m_position = apply_trafo(best_trafo, sys.m_position);
   }
   fmt::print("metric with optimized trafo: {:.2f} LY\n",get_metric(starfield_universe, real_universe));

   // galactic_coord gc{ .m_l = glm::radians(195.844640), .m_b = glm::radians(-48.0512), .m_dist = 10.501583 };
   // starfield_universe.m_systems.push_back(sfn::system(gc.get_cartesian(), "EE", "EE", system_size::big));


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
         std::string guess_str;
         if (hip.has_value() && real_universe.get_index_by_name(*hip) == real_closest[i])
            guess_str = "CHOICE";
         fmt::print(
            "{}: {}, dist: {:.2f}, {}\n",
            i, real_universe.get_name_by_index(real_closest[i]), dist,  guess_str
         );
      }
   };
   const auto candidates_for_real = [&](const std::string& real_name)
   {
      const glm::vec3 target_pos = real_universe.get_pos_by_hip(real_name);
      std::vector<int> real_closest;
      for (int i = 0; i < std::ssize(starfield_universe.m_systems); ++i)
         real_closest.push_back(i);
      const auto pred = [&](const int i, const int j)
      {
         const float dist_i = glm::distance(target_pos, starfield_universe.m_systems[i].m_position);
         const float dist_j = glm::distance(target_pos, starfield_universe.m_systems[j].m_position);
         return dist_i < dist_j;
      };
      std::ranges::sort(real_closest, pred);
      fmt::print("\n");
      for (int i = 0; i < 3; ++i)
      {
         const float dist = glm::distance(target_pos, starfield_universe.m_systems[real_closest[i]].m_position);
         fmt::print("{}: {:.2f} {}\n", i, dist, starfield_universe.m_systems[i].get_name());
      }
   };
   candidates_for_real("91262"); // vega

   // candidates_for_fictional("User 30");
   // candidates_for_fictional("User 50");
   // candidates_for_fictional("User 35");
   // candidates_for_fictional("User 40");
   // candidates_for_fictional("ADN");
   // candidates_for_fictional("ADQ");
   // candidates_for_fictional("ADU");
   // candidates_for_fictional("ADV");
   // candidates_for_fictional("AEB");
   // candidates_for_fictional("ABX");
   
   const auto error_report = [&](const std::string& fictional_name, const std::string& hip)
   {
      const glm::vec3 fiction_pos = starfield_universe.get_position_by_name(fictional_name);
      const glm::vec3 real_pos = real_universe.get_pos_by_hip(hip);
      const float dist = glm::distance(fiction_pos, real_pos);
      fmt::print("{:<16} deviation: {:>5.2f} LY\n", fictional_name, dist);
   };

   for(const sfn::system& system : starfield_universe.m_systems)
   {
      if (system.m_astronomic_name.empty())
         continue;
      error_report(system.m_astronomic_name, system.m_catalog_lookup);
   }
   // error_report("HD 125455", "70016");


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

