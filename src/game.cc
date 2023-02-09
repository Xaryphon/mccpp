#include "game.hh"

#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include "cvar.hh"
#include "input/input.hh"
#include "renderer/renderer.hh"

namespace mccpp {

class game_impl final : public game {
public:
    game_impl(application &);

    void on_frame() override;
    float delta_time() override { return m_frame_time; }

private:
    cvar::manager &m_cvar_manager;
    input::manager &m_input_manager;
    renderer::renderer &m_renderer;

    std::chrono::steady_clock::time_point m_frame_last;
    float m_frame_time = 0.f;

    struct {
        struct {
            input::input_ref xn;
            input::input_ref xp;
            input::input_ref yn;
            input::input_ref yp;
        } look;
        struct {
            input::input_ref xn;
            input::input_ref xp;
            input::input_ref yn;
            input::input_ref yp;
        } move;
        input::input_ref jump;
        input::input_ref sneak;
    } m_input;
};

std::unique_ptr<game> game::create(application &app) {
    return std::make_unique<game_impl>(app);
}

game_impl::game_impl(application &app)
: m_cvar_manager(app.cvar_manager())
, m_input_manager(app.input_manager())
, m_renderer(app.renderer())
, m_input {
        .look = {
            .xn = m_input_manager.get("look.x-"),
            .xp = m_input_manager.get("look.x+"),
            .yn = m_input_manager.get("look.y-"),
            .yp = m_input_manager.get("look.y+"),
        },
        .move = {
            .xn = m_input_manager.get("move.x-"),
            .xp = m_input_manager.get("move.x+"),
            .yn = m_input_manager.get("move.y-"),
            .yp = m_input_manager.get("move.y+"),
        },
        .jump = m_input_manager.get("jump"),
        .sneak = m_input_manager.get("sneak"),
    }
{
    m_input_manager.bind_mouse_x(m_input.look.xn, m_input.look.xp);
    m_input_manager.bind_mouse_y(m_input.look.yn, m_input.look.yp);
    m_input_manager.bind_keyboard(SDL_SCANCODE_A, m_input.move.xn);
    m_input_manager.bind_keyboard(SDL_SCANCODE_D, m_input.move.xp);
    m_input_manager.bind_keyboard(SDL_SCANCODE_W, m_input.move.yn);
    m_input_manager.bind_keyboard(SDL_SCANCODE_S, m_input.move.yp);
    m_input_manager.bind_keyboard(SDL_SCANCODE_SPACE, m_input.jump);
    m_input_manager.bind_keyboard(SDL_SCANCODE_LSHIFT, m_input.sneak);

    m_frame_last = std::chrono::steady_clock::now();
}

void game_impl::on_frame() {
    const float MOUSE_SENSITIVITY = 0.36f;
    const float MOVE_SPEED = 20.0f;

    glm::vec3 &camera_position = m_renderer.camera().position;
    glm::vec3 &look = m_renderer.camera().rotation;

    look.x += MOUSE_SENSITIVITY * (m_input.look.xp->raw() - m_input.look.xn->raw()) * m_frame_time;
    look.y += MOUSE_SENSITIVITY * (m_input.look.yp->raw() - m_input.look.yn->raw()) * m_frame_time;

    look.x = glm::mod(look.x, 2.0f * glm::pi<float>());
    if (look.y >  0.5f * glm::pi<float>())
        look.y =  0.5f * glm::pi<float>();
    if (look.y < -0.5f * glm::pi<float>())
        look.y = -0.5f * glm::pi<float>();

    glm::vec2 move_input {
        m_input.move.xp->value() - m_input.move.xn->value(),
        m_input.move.yp->value() - m_input.move.yn->value(),
    };

    glm::vec2 move {
        move_input.y * cos(-look.x) + move_input.x * sin(look.x),
        move_input.y * sin(-look.x) + move_input.x * cos(look.x),
    };

    if (glm::dot(move, move) > 1) {
        move = glm::normalize(move);
    }

    camera_position.x += MOVE_SPEED * move.y * m_frame_time;
    camera_position.z += MOVE_SPEED * move.x * m_frame_time;
    float fly = 0.f;
    if (m_input.jump->pressed())
        fly += 1.f;
    if (m_input.sneak->pressed())
        fly -= 1.f;
    camera_position.y += MOVE_SPEED * fly * m_frame_time;

    ImGui::SetNextWindowPos(ImVec2{0, 0}, ImGuiCond_Always, {0, 0});
    if (ImGui::Begin("##DebugTopLeft", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus))
    {
        ImGui::Text("%.0f fps %.3f ms", 1 / m_frame_time, m_frame_time * 1000.f);

        ImGui::Text("move : %f, %f", move.x, move.y);
        ImGui::Text("move_input : %f, %f", move_input.x, move_input.y);
        ImGui::NewLine();

        for (auto &[name, input] : m_input_manager) {
            ImGui::Text("%.*s : %f", static_cast<int>(name.size()), name.data(), input.raw());
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
    glm::vec3 move_speed = (camera_position - last_camera_position) / m_frame_time;
    last_camera_position = camera_position;
    ImGui::DragFloat3("Velocity", glm::value_ptr(move_speed));
    float velocity = glm::length(move_speed);
    ImGui::DragFloat("|Velocity|", &velocity);

    for (auto &var : m_cvar_manager) {
        float v = var.value();
        if (ImGui::InputFloat(var.name().c_str(), &v,
                0.f, 0.f, "%.5f",
                ImGuiInputTextFlags_EnterReturnsTrue)) {
            var.set_value(v);
        }
        std::string_view help = var.help();
        if (!help.empty() && ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted(help.data(), help.data() + help.size());
            ImGui::EndTooltip();
        }
    }

    auto now = std::chrono::steady_clock::now();
    m_frame_time = std::chrono::duration_cast<std::chrono::duration<float>>(now - m_frame_last).count();
    m_frame_last = now;
}

}
