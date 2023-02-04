#pragma once

namespace mccpp {

namespace renderer {
    class renderer;
}

class application {
public:
    virtual ~application() = default;

    virtual renderer::renderer &renderer() = 0;
    virtual class game &game() = 0;

    virtual bool capture_mouse() = 0;
    virtual void capture_mouse(bool) = 0;

    virtual bool should_exit() = 0;
    virtual void should_exit(bool) = 0;

    // FIXME: remove deprecated frame_count()
    static unsigned frame_count();
};

[[nodiscard]]
int main(int argc, char **argv);

}
