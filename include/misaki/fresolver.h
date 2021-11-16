#pragma once

#include <filesystem>
#include <vector>

#include "platform.h"

namespace misaki {

namespace fs = std::filesystem;

class APR_EXPORT FileResolver {
public:
    using iterator       = std::vector<fs::path>::iterator;
    using const_iterator = std::vector<fs::path>::const_iterator;

    FileResolver();

    FileResolver(const FileResolver &fr);

    fs::path resolve(const fs::path &path) const;

    size_t size() const { return m_paths.size(); }

    iterator begin() { return m_paths.begin(); }

    iterator end() { return m_paths.end(); }

    const_iterator begin() const { return m_paths.begin(); }

    const_iterator end() const { return m_paths.end(); }

    bool contains(const fs::path &p) const;

    void erase(iterator it) { m_paths.erase(it); }

    void erase(const fs::path &p);

    void clear() { m_paths.clear(); }

    void prepend(const fs::path &p) { m_paths.insert(m_paths.begin(), p); }

    void append(const fs::path &p) { m_paths.emplace_back(p); }

    fs::path &operator[](size_t index) { return m_paths[index]; }

    const fs::path &operator[](size_t index) const { return m_paths[index]; }

    std::string to_string() const;

private:
    std::vector<fs::path> m_paths;
};

} // namespace misaki