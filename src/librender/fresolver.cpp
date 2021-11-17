#include <misaki/core/fresolver.h>

#include <sstream>

namespace misaki {

FileResolver::FileResolver() { m_paths.emplace_back(fs::current_path()); }

FileResolver::FileResolver(const FileResolver &fr) : m_paths(fr.m_paths) {}

fs::path FileResolver::resolve(const fs::path &path) const {
    if (!path.is_absolute()) {
        for (auto const &base : m_paths) {
            fs::path combined = base / path;
            if (fs::exists(combined))
                return combined;
        }
    }
    return path;
}

bool FileResolver::contains(const fs::path &p) const {
    return std::find(m_paths.begin(), m_paths.end(), p) != m_paths.end();
}

void FileResolver::erase(const fs::path &p) {
    m_paths.erase(std::remove(m_paths.begin(), m_paths.end(), p),
                  m_paths.end());
}

std::string FileResolver::to_string() const {
    std::ostringstream oss;
    oss << "FileResolver[" << std::endl;
    for (size_t i = 0; i < m_paths.size(); ++i) {
        oss << "  \"" << m_paths[i] << "\"";
        if (i + 1 < m_paths.size())
            oss << ",";
        oss << std::endl;
    }
    oss << "]";
    return oss.str();
}

} // namespace misaki