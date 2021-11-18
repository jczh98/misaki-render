#pragma once

#include <misaki/render/film.h>
#include <misaki/ui/imgui.h>
#include <GLFW/glfw3.h> 

namespace misaki::ui {

class MSK_EXPORT Viewer {
public:
    Viewer(Film *film);

    bool init();

    void mainloop();

    void shutdown();

private:
    Film *m_film = nullptr;
    int m_width = 0, m_height = 0;
    GLFWwindow *m_window = nullptr;
};

} // namespace misaki::ui