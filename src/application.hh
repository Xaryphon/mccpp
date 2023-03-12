#pragma once

namespace mccpp {

namespace cvar {
    class manager;
}

namespace vfs {
    class vfs;
}

namespace resource {
    class manager;
}

namespace input {
    class manager;
}

namespace renderer {
    class renderer;
}

namespace client {
    class client;
}

class application {
public:
    virtual ~application() = default;

    virtual cvar::manager &cvar_manager() = 0;
    virtual vfs::vfs &assets() = 0;
    virtual resource::manager &resource_manager() = 0;
    virtual class input::manager &input_manager() = 0;
    virtual renderer::renderer &renderer() = 0;
    virtual class game &game() = 0;
    virtual class client::client &client() = 0;

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
