#include <misaki/render/film.h>
#include <misaki/render/imageblock.h>
#include <misaki/render/properties.h>

namespace misaki::render {

class RGBFilm final : public Film {
 public:
  RGBFilm(const Properties &props) : Film(props) {
  }

  void put(const ImageBlock *block) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_storage->put(block);
  }

  void develop() {
  }

  void set_destination_file(const fs::path &filename) {
    m_dest_file = filename;
  }

 private:
  std::unique_ptr<ImageBlock> m_storage;
  fs::path m_dest_file;
  std::mutex m_mutex;
};

MSK_EXPORT_PLUGIN(RGBFilm)

}  // namespace misaki::render