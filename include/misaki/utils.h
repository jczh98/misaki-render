#pragma once

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>

namespace misaki {

class ProgressBar {
public:
    ProgressBar(size_t tot, size_t width) : m_total(tot), m_width(width) {
        show();
    }
    void update() {
        m_count++;
        show();
    }
    void done() { std::cout << std::endl; }

private:
    void show() {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::cout << "[";
        float progress = float(m_count) / m_total;
        size_t pos     = size_t(progress * m_width);
        for (size_t i = 0; i < m_width; ++i) {
            if (i < pos)
                std::cout << "=";
            else if (i == pos)
                std::cout << ">";
            else
                std::cout << " ";
        }
        std::cout << "] " << int(progress * 100.0) << "% (" << m_count << "/"
                  << m_total << ")"
                  << " \r";
        std::cout.flush();
    }
    std::atomic<size_t> m_count = 0;
    std::mutex m_mutex;
    size_t m_total, m_width;
};

class Timer {
public:
    Timer() { start = std::chrono::system_clock::now(); }

    size_t value() const {
        auto now = std::chrono::system_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
        return (size_t) duration.count();
    }

    size_t reset() {
        auto now = std::chrono::system_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
        start = now;
        return (size_t) duration.count();
    }

private:
    std::chrono::system_clock::time_point start;
};

extern std::string time_string(float time, bool precise = false);
extern std::string mem_string(size_t size, bool precise = false);

} // namespace misaki
