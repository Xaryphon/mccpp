#include "application.hh"

#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui_impl_sdl.h>
#include <imgui.h>
#include <SDL.h>

#include "logger.hh"
#include "input/input.hh"
#include "renderer/renderer.hh"

namespace mccpp::application {

struct {
    bool should_quit = false;
    bool capture_mouse;
    unsigned frame_count = 0;

    struct {
        struct {
            input::axis x { "look.x" };
            input::axis y { "look.y" };
        } look;
        struct {
            input::axis x { "move.x" };
            input::axis y { "move.y" };
        } move;
        input::button jump  { "jump" };
        input::button sneak { "sneak" };
        input::button unlock_cursor { "unlock_cursor" };
    } input;
} g_app = {};

static void set_capture_mouse(bool state)
{
    g_app.capture_mouse = state;
    SDL_SetRelativeMouseMode(state ? SDL_TRUE : SDL_FALSE);
}

// This a is pretty genius way of doing this if I do say so myshelf ;)
[[nodiscard]]
static bool _init_or_quit(bool init)
{
    auto &I = g_app.input;

    if (!init)
        goto quit;

    if (SDL_Init(SDL_INIT_VIDEO)) {
        MCCPP_F("SDL_Init failed: {}", SDL_GetError());
        goto sdl_init_failed;
    }

    input::mouse::assign(I.look.x, I.look.y);
    input::keyboard::assign(I.move.x, SDL_SCANCODE_D, SDL_SCANCODE_A);
    input::keyboard::assign(I.move.y, SDL_SCANCODE_S, SDL_SCANCODE_W);
    input::keyboard::assign(I.jump, SDL_SCANCODE_SPACE);
    input::keyboard::assign(I.sneak, SDL_SCANCODE_LSHIFT);
    input::keyboard::assign(I.unlock_cursor, SDL_SCANCODE_LALT);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    if (!renderer::init()) {
        goto renderer_init_failed;
    }

    set_capture_mouse(true);

    return true;

quit:
    renderer::destroy();
renderer_init_failed:
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
sdl_init_failed:
    ImGui::DestroyContext();
    return false;
}

[[nodiscard]]
static bool init()
{
    return _init_or_quit(true);
}

static void quit()
{
    (void) _init_or_quit(false);
}

static void poll_events()
{
    ImGuiIO &io = ImGui::GetIO();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        switch (event.type) {
        case SDL_QUIT:
            g_app.should_quit = true;
            break;
        case SDL_WINDOWEVENT:
            switch (event.window.event) {
            case SDL_WINDOWEVENT_CLOSE:
                g_app.should_quit = true;
                break;
            default:
                break;
            }
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            if (io.WantCaptureKeyboard)
                break;
            input::manager::handle_event(event);
            break;
        case SDL_MOUSEMOTION:
            if (g_app.capture_mouse) {
                input::manager::handle_event(event);
            }
            break;
        default:
            break;
        }
    }
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    if (!init())
        return 1;

    unsigned frame_count = 0;
    auto frame_last = std::chrono::steady_clock::now();
    float frame_time = 0.f;

    while (!g_app.should_quit) {
        input::manager::reset_deltas();
        poll_events();

        if (g_app.input.unlock_cursor.down())
            set_capture_mouse(!g_app.capture_mouse);

        const float MOUSE_SENSITIVITY = 2.0f;
        const float MOVE_SPEED = 20.0f;

        glm::vec3 &camera_position = renderer::camera::position();
        glm::vec3 &look = renderer::camera::rotation();

        look.x += MOUSE_SENSITIVITY * g_app.input.look.x * frame_time;
        look.y += MOUSE_SENSITIVITY * g_app.input.look.y * frame_time;

        look.x = glm::mod(look.x, 2.0f * glm::pi<float>());
        if (look.y >  0.5f * glm::pi<float>())
            look.y =  0.5f * glm::pi<float>();
        if (look.y < -0.5f * glm::pi<float>())
            look.y = -0.5f * glm::pi<float>();

        glm::vec2 move {
            g_app.input.move.y * cos(-look.x) + g_app.input.move.x * sin(look.x),
            g_app.input.move.y * sin(-look.x) + g_app.input.move.x * cos(look.x),
        };

        if (glm::dot(move, move) > 1) {
            move = glm::normalize(move);
        }

        camera_position.x += MOVE_SPEED * move.y * frame_time;
        camera_position.z += MOVE_SPEED * move.x * frame_time;
        float fly = 0.f;
        if (g_app.input.jump)
            fly += 1.f;
        if (g_app.input.sneak)
            fly -= 1.f;
        camera_position.y += MOVE_SPEED * fly * frame_time;

        renderer::frame_start();
        ImGui_ImplSDL2_NewFrame();
        // ImGui_ImplSDL2_NewFrame gets global mouse position so we have to disable mouse here
        if (g_app.capture_mouse) {
            ImGui::GetIO().AddMousePosEvent(-FLT_MAX, -FLT_MAX);
        }
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2{0, 0}, ImGuiCond_Always, {0, 0});
        if (ImGui::Begin("##DebugTopLeft", nullptr,
                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus))
        {
            ImGui::Text("%.0f fps %.3f ms", 1 / frame_time, frame_time * 1000.f);
            for (uint16_t i = 0, c = input::manager::get_input_count(); i < c; i++) {
                std::string_view n = input::manager::get_input_name(i);
                float v = input::manager::get_input_value(i);
                ImGui::Text("%.*s = %.03f", (int)n.length(), n.data(), v);
            }
        }
        ImGui::End();

        //ImGui::DragFloat2("input.move", glm::value_ptr(g_app.input.move));
        //ImGui::DragFloat2("input.look", glm::value_ptr(g_app.input.look));
        //ImGui::DragFloat("input.fly", &g_app.input.fly);

        ImGui::DragFloat3("Camera Position", glm::value_ptr(camera_position), 0.1f);
        glm::vec3 look_degrees = look / glm::pi<float>() * 180.0f;
        if (ImGui::DragFloat3("Camera Rotation", glm::value_ptr(look_degrees), 0.1f))
            look = look_degrees / 180.0f * glm::pi<float>();

        static glm::vec3 last_camera_position = {};
        glm::vec3 move_speed = (camera_position - last_camera_position) / frame_time;
        last_camera_position = camera_position;
        ImGui::DragFloat3("Velocity", glm::value_ptr(move_speed));
        float velocity = glm::length(move_speed);
        ImGui::DragFloat("|Velocity|", &velocity);

        renderer::frame_end();

        auto now = std::chrono::steady_clock::now();
        frame_time = std::chrono::duration_cast<std::chrono::duration<float>>(now - frame_last).count();
        frame_last = now;
        frame_count++;
        g_app.frame_count = frame_count;
    }

    quit();
    return 0;
}

unsigned frame_count()
{
    return g_app.frame_count;
}

}
