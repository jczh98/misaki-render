#include <misaki/render/film.h>
#include <misaki/render/imageblock.h>
#include <misaki/render/imageio.h>
#include <misaki/render/properties.h>
#include <misaki/utils/image.h>

#include <fstream>

namespace misaki::render {

class RGBFilm final : public Film {
 public:
  RGBFilm(const Properties &props) : Film(props) {
    m_storage = std::make_unique<ImageBlock>(m_size);
    m_storage->clear();
  }

  void put(const ImageBlock *block) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_storage->put(block);
  }

  void develop() {
    auto bitmap = math::Tensor<Color3, 2>::from_linear_indexed(
        m_storage->data().shape(),
        [&](int i) {
          return math::linear_to_srgb(m_storage->data().raw_data()[i].divide_by_filter_weight());
        });
    auto another = m_dest_file;
    image::write_rgb_image(another.replace_extension("jpg").string(), bitmap);
    write_float_rgb_image(m_dest_file.string(), bitmap);
  }

  void set_destination_file(const fs::path &filename) {
    m_dest_file = filename;
  }

  MSK_DECL_COMP(Film)
 private:
  std::unique_ptr<ImageBlock> m_storage;
  fs::path m_dest_file;
  std::mutex m_mutex;
};

MSK_EXPORT_PLUGIN(RGBFilm)

}  // namespace misaki::render