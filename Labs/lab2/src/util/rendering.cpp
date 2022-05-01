#include "util/rendering.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Tracy.hpp"
#include <array>
#include <assert.h>
#include <chrono>
#include <iostream>
#include <string>
#include <unordered_map>


// Dear students;
// if you have found your way here, rest assured that understanding the rest of this file
// is *not* required to complete the lab. This file contains a lot of implementation for
// the rendering helper functionalities. Just have a look at the corresponding header file
// "rendering.h" and look at the structs and functions that are exposed there. They are
// all documented so that looking at the source code should not be necessary.
// Having said that, if you are interested in anything, of course continue browsing here

namespace {

// Holds the pointer to the active (and only) window
GLFWwindow* _window = nullptr;
struct { float r = 0.05f; float g = 0.1f; float b = 0.1f; } _backgroundColor;

// Structure to hold information to render an individual type of renderable object, in
// this case, particles, emitters, and forces
struct Renderable {
    ~Renderable() {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
        glDeleteBuffers(1, &vbo);
        vbo = 0;
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
        glDeleteShader(vertexShader);
        vertexShader = 0;
        glDeleteShader(geometryShader);
        geometryShader = 0;
        glDeleteShader(fragmentShader);
        fragmentShader = 0;
    }

    GLuint vao = 0;
    GLuint vbo = 0;

    GLuint shaderProgram = 0;
    GLuint vertexShader = 0;
    GLuint geometryShader = 0;
    GLuint fragmentShader = 0;
};

Renderable _particles;
Renderable _emitters;
Renderable _forces;


/**
 * Checks the compilation status of the shader passed into it and prints out a message in
 * case the shader was not compiled successfully. If the shader is successfully compiled,
 * no message is printed. This function does **not** compile the shader.
 * This function only performs a check if it is compiled in Debug mode. In release builds
 * these checks will be omitted.
 *
 * \param shader The GL object of the shader object that should be checked
 * \param name The human-readable name of the shader object used in the printing
 *
 * \throw std::runtime_error An exception is raised if the compilation of the shader
 *        failed
 * \pre \p shader must be a valid GL shader object
 * \pre \p name must not be empty
 * \post Only the compile status of \p shader might be modified
 */
void checkShader([[ maybe_unused ]] GLuint shader,
                 [[ maybe_unused ]] const std::string& name)
{
    ZoneScoped

    assert(shader != 0);
    assert(!name.empty());

#ifdef _DEBUG
    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> buf;
        buf.resize(logLength);
        glGetShaderInfoLog(shader, logLength, NULL, buf.data());

        std::cout << "Error compiling shader: " << name << '\n' <<
            std::string(buf.data());

        throw std::runtime_error("Error compiling shader");
    }
#endif // _DEBUG
}

/**
 * Checks the linking status of the passed shader program passed into it and prints out a
 * message in case the program was not linked successfully. If the program was linked
 * successfully, no message is printed. This function does **not** link the program.
 * This function only performs a check if it is compiled in Debug mode. In release builds
 * these checks will be omitted.
 *
 * \param program The shader program that is to be checked
 * \param name The human-readable name of the shader program used in the printing
 *
 * \throw std::runtime_error An exception is raised if the linking of the program failed
 *
 * \pre \p program must be a valid GL program object
 * \pre \p name must not be empty
 * \post Only the link status of \p program might be modified
 */
void checkProgram([[ maybe_unused ]] GLuint program,
                  [[ maybe_unused ]] const std::string& name)
{
    ZoneScoped

    assert(program != 0);
    assert(!name.empty());

#ifdef _DEBUG
    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint logLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> buf;
        buf.resize(logLength);
        glGetProgramInfoLog(program, logLength, NULL, buf.data());

        std::cout << "Error linking program: " << name << '\n' << std::string(buf.data());

        throw std::runtime_error("Error linking program");
    }
#endif // _DEBUG
}


/**
 * Checks whether any of the GL functions called since the last call to this function have
 * raised any errors. If there were multiple errors, only the first error will be printed.
 * If there haven't been any errors, no message is printed. If this function was compiled
 * in a release build, all error messages are omitted.
 *
 * \param name The human-readable name that is used in the logging
 *
 * \throw std::runtime_error An exception is raised if there was an OpenGL error
 *
 * \pre \p name must not be empty
 * \post If compiling this function in Debug mode, the OpenGL error will be GL_NO_ERROR
 */
void checkOpenGLError([[ maybe_unused ]] const std::string& name) {
#ifdef _DEBUG
    GLenum e = glGetError();
    if (e != GL_NO_ERROR) {
        std::cout << "OpenGL error occurred( " << name << ") " << e << '\n';
        throw std::runtime_error("OpenGL error occurred");
    }
#endif // _DEBUG
}

/**
 * Creates the shaders and the program objects for rendering the particle data.
 *
 * \param shader The program object that will be created
 * \param vertex The vertex shader object that will be created
 * \param fragment The fragment shader object that will be created
 *
 * \throw std::runtime_error Might raise an exception if the compilation or linking of the 
 *        shader or program fails
 *
 * \pre \p shader must not already be a valid program object
 * \pre \p vertex must not already be a valid shader object
 * \pre \p fragment must not already be a valid shader object
 * \post \p shader is a valid and linked program object
 * \post \p vertex is a valid and compiled shader object
 * \post \p fragment is a valid and compiled shader object
 */
void createParticleShader(GLuint& shader, GLuint& vertex, GLuint& fragment) {
    ZoneScoped

    constexpr const char* ParticleVertexShader[1] = { R"(
        #version 330
        layout(location = 0) in vec2 in_position;
        layout(location = 1) in float in_radius;
        layout(location = 2) in vec3 in_color;
        layout(location = 3) in float in_lifetime;

        out vec3 vs_color;
        out float vs_lifetime;

        void main() {
            vs_color = in_color;
            vs_lifetime = in_lifetime;
            gl_PointSize = in_radius;
            gl_Position = vec4(in_position, 0.0, 1.0);
        }
    )" };

    constexpr const char* ParticleFragmentShader[1] = { R"(
        #version 330
        in vec3 vs_color;
        in float vs_lifetime;
        out vec4 out_color;

        void main() {
            // Shift the origin to the center of the point
            vec2 p = gl_PointCoord * vec2(2.0) - vec2(1.0); 
            // Approximate the length of the vector by its square
            float len = p.x * p.x + p.y * p.y;
            // The transparency of the particle fragment with distance from the center
            float alpha = (1.0 - len * len) * min(1.0, vs_lifetime);
            out_color = vec4(vs_color, alpha);
        }
    )" };

    assert(vertex == 0);
    assert(fragment == 0);
    assert(shader == 0);


    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, ParticleVertexShader, nullptr);
    glCompileShader(vertex);
    checkShader(vertex, "particles-vertex");

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, ParticleFragmentShader, nullptr);
    glCompileShader(fragment);
    checkShader(fragment, "particles-fragment");

    shader = glCreateProgram();
    glAttachShader(shader, vertex);
    glAttachShader(shader, fragment);
    glLinkProgram(shader);
    checkProgram(shader, "particles");


    assert(vertex != 0);
    assert(fragment != 0);
    assert(shader != 0);
}

/**
 * Creates the vertex array object and vertex buffer object used to render the particle
 * data. The vertex attributes of the vertex buffer are configured based on the
 * #rendering::ParticleInfo structure.
 *
 * \param vao The vertex array object that will be created
 * \param vbo The vertex buffer object that will be created
 *
 * \pre \p vao must not be a valid GL object
 * \pre \p vbo must not be a valid GL object
 * \post \p vao is a valid, but unbound vertex array object
 * \post \p vbo is a valid vertex buffer object bound to the \p vao array
 */
void createParticleGLObjects(GLuint& vao, GLuint& vbo) {
    ZoneScoped

    assert(vao == 0);
    assert(vbo == 0);


    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(rendering::ParticleInfo),
        reinterpret_cast<GLvoid*>(offsetof(rendering::ParticleInfo, position))
    );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(rendering::ParticleInfo),
        reinterpret_cast<GLvoid*>(offsetof(rendering::ParticleInfo, radius))
    );
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, sizeof(rendering::ParticleInfo),
        reinterpret_cast<GLvoid*>(offsetof(rendering::ParticleInfo, color))
    );
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(rendering::ParticleInfo),
        reinterpret_cast<GLvoid*>(offsetof(rendering::ParticleInfo, lifetime))
    );
    glBindVertexArray(0);


    assert(vao != 0);
    assert(vbo != 0);
}

/**
 * Creates the shaders and the program objects for rendering the emitter data.
 *
 * \param shader The program object that will be created
 * \param vertex The vertex shader object that will be created
 * \param geometry The geometry shader object that will be created
 * \param fragment The fragment shader object that will be created
 *
 * \throw std::runtime_error Might raise an exception if the compilation or linking of the
 *        shader or program fails
 *
 * \pre \p shader must not already be a valid program object
 * \pre \p vertex must not already be a valid shader object
 * \pre \p geometry must not already be a valid shader object
 * \pre \p fragment must not already be a valid shader object
 * \post \p shader is a valid and linked program object
 * \post \p vertex is a valid and compiled shader object
 * \post \p geometry is a valid and compiled shader object
 * \post \p fragment is a valid and compiled shader object
 */
void createEmitterShader(GLuint& shader,
                         GLuint& vertex, GLuint& geometry, GLuint& fragment)
{
    ZoneScoped

    constexpr const char* EmitterVertexShader[1] = { R"(
        #version 330
        layout(location = 0) in vec2 in_position;
        layout(location = 1) in float in_size;
        layout(location = 2) in vec3 in_color;

        out float vs_size;
        out vec3 vs_color;

        void main() {
            vs_size = in_size;
            vs_color = in_color;
            gl_Position = vec4(in_position, 0.0, 1.0);
        }
    )" };

    constexpr const char* EmitterGeometryShader[1] = { R"(
        #version 330
        layout(points) in;
        layout(triangle_strip, max_vertices = 4) out;

        in float vs_size[];
        in vec3 vs_color[];

        out vec3 ge_color;

        uniform ivec2 windowSize;

        void main() {
            vec2 p = gl_in[0].gl_Position.xy;
            vec2 s = vec2(vs_size[0] / windowSize.x, vs_size[0] / windowSize.y);
            vec3 c = vs_color[0];

            // Create vertex for the lower left corner
            gl_Position = vec4(p.x - s.x, p.y - s.y, 0.0, 1.0);
            ge_color = c;
            EmitVertex();

            // Create vertex for the lower right corner
            gl_Position = vec4(p.x + s.x, p.y - s.y, 0.0, 1.0);
            ge_color = c;
            EmitVertex();

            // Create vertex for the top left corner
            gl_Position = vec4(p.x - s.x, p.y + s.y, 0.0, 1.0);
            ge_color = c;
            EmitVertex();

            // Create vertex for the top right corner
            gl_Position = vec4(p.x + s.x, p.y + s.y, 0.0, 1.0);
            ge_color = c;
            EmitVertex();
        }
    )" };

    constexpr const char* EmitterFragmentShader[1] = { R"(
        #version 330
        in vec3 ge_color;
        out vec4 out_color;

        void main() {
            out_color = vec4(ge_color, 1.0);
        }
    )" };

    assert(vertex == 0);
    assert(geometry == 0);
    assert(fragment == 0);
    assert(shader == 0);


    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, EmitterVertexShader, nullptr);
    glCompileShader(vertex);
    checkShader(vertex, "emitters-vertex");

    geometry = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometry, 1, EmitterGeometryShader, nullptr);
    glCompileShader(geometry);
    checkShader(geometry, "emitters-geometry");

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, EmitterFragmentShader, nullptr);
    glCompileShader(fragment);
    checkShader(fragment, "emitters-fragment");

    shader = glCreateProgram();
    glAttachShader(shader, vertex);
    glAttachShader(shader, geometry);
    glAttachShader(shader, fragment);
    glLinkProgram(shader);
    checkProgram(shader, "emitters");


    assert(vertex != 0);
    assert(geometry != 0);
    assert(fragment != 0);
    assert(shader != 0);
}

/**
 * Creates the vertex array object and vertex buffer object used to render the emitter
 * data. The vertex attributes of the vertex buffer are configured based on the
 * #rendering::EmitterInfo structure.
 *
 * \param vao The vertex array object that will be created
 * \param vbo The vertex buffer object that will be created
 *
 * \pre \p vao must not be a valid GL object
 * \pre \p vbo must not be a valid GL object
 * \post \p vao is a valid, but unbound vertex array object
 * \post \p vbo is a valid vertex buffer object bound to the \p vao array
 */
void createEmitterGLObjects(GLuint& vao, GLuint& vbo) {
    ZoneScoped

    assert(vao == 0);
    assert(vbo == 0);


    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(rendering::EmitterInfo),
        reinterpret_cast<GLvoid*>(offsetof(rendering::EmitterInfo, position))
    );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(rendering::EmitterInfo),
        reinterpret_cast<GLvoid*>(offsetof(rendering::EmitterInfo, size))
    );
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, sizeof(rendering::EmitterInfo),
        reinterpret_cast<GLvoid*>(offsetof(rendering::EmitterInfo, color))
    );
    glBindVertexArray(0);


    assert(vao != 0);
    assert(vbo != 0);
}

/**
 * Creates the shaders and the program objects for rendering the force data.
 *
 * \param shader The shader program that will be created
 * \param vertex The vertex shader that will be created
 * \param geometry The geometry shader object that will be created
 * \param fragment The fragment shader that will be created
 *
 * \throw std::runtime_error Might raise an exception if the compilation or linking of the
 *        shader or program fails
 *
 * \pre \p shader must not already be a valid program object
 * \pre \p vertex must not already be a valid shader object
 * \pre \p geometry must not already be a valid shader object
 * \pre \p fragment must not already be a valid shader object
 * \post \p shader is a valid and linked program object
 * \post \p vertex is a valid and compiled shader object
 * \post \p geometry is a valid and compiled shader object
 * \post \p fragment is a valid and compiled shader object
 */
void createForceShader(GLuint& shader, GLuint& vertex, GLuint& geometry, GLuint& fragment)
{
    ZoneScoped

    constexpr const char* ForceVertexShader[1] = { R"(
        #version 330
        layout(location = 0) in vec2 in_position;
        layout(location = 1) in float in_size;
        layout(location = 2) in vec3 in_color;

        out float vs_size;
        out vec3 vs_color;

        void main() {
            vs_size = in_size;
            vs_color = in_color;
            gl_Position = vec4(in_position, 0.0, 1.0);
        }
    )" };

    constexpr const char* ForceGeometryShader[1] = { R"(
        #version 330
        layout(points) in;
        layout(triangle_strip, max_vertices = 4) out;

        in float vs_size[];
        in vec3 vs_color[];

        out vec3 ge_color;

        uniform ivec2 windowSize;

        void main() {
            vec2 p = gl_in[0].gl_Position.xy;
            vec2 s = vec2(vs_size[0] / windowSize.x, vs_size[0] / windowSize.y);
            vec3 c = vs_color[0];

            // Create vertex for the top corner
            gl_Position = vec4(p.x, p.y + s.y, 0.0, 1.0);
            ge_color = c;
            EmitVertex();

            // Create vertex for the lower left corner
            gl_Position = vec4(p.x - s.x, p.y, 0.0, 1.0);
            ge_color = c;
            EmitVertex();

            // Create vertex for the lower right corner
            gl_Position = vec4(p.x + s.x, p.y, 0.0, 1.0);
            ge_color = c;
            EmitVertex();

            // Create vertex for the bottom corner
            gl_Position = vec4(p.x, p.y - s.y, 0.0, 1.0);
            ge_color = c;
            EmitVertex();
        }
    )" };

    constexpr const char* ForceFragmentShader[1] = { R"(
        #version 330
        in vec3 ge_color;
        out vec4 out_color;

        void main() {
            out_color = vec4(ge_color, 1.0);
        }
    )" };

    assert(vertex == 0);
    assert(geometry == 0);
    assert(fragment == 0);
    assert(shader == 0);


    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, ForceVertexShader, nullptr);
    glCompileShader(vertex);
    checkShader(vertex, "forces-vertex");

    geometry = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometry, 1, ForceGeometryShader, nullptr);
    glCompileShader(geometry);
    checkShader(geometry, "forces-geometry");

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, ForceFragmentShader, nullptr);
    glCompileShader(fragment);
    checkShader(fragment, "forces-fragment");

    shader = glCreateProgram();
    glAttachShader(shader, vertex);
    glAttachShader(shader, geometry);
    glAttachShader(shader, fragment);
    glLinkProgram(shader);
    checkProgram(shader, "forces");


    assert(vertex != 0);
    assert(geometry != 0);
    assert(fragment != 0);
    assert(shader != 0);
}

/**
 * Creates the vertex array object and vertex buffer object used to render the force
 * data. The vertex attributes of the vertex buffer are configured based on the
 * #rendering::ForceInfo structure.
 *
 * \param vao The vertex array object that will be created
 * \param vbo The vertex buffer object that will be created
 *
 * \pre \p vao must not be a valid GL object
 * \pre \p vbo must not be a valid GL object
 * \post \p vao is a valid, but unbound vertex array object
 * \post \p vbo is a valid vertex buffer object bound to the \p vao array
 */
void createForceGLObjects(GLuint& vao, GLuint& vbo) {
    ZoneScoped

    assert(vao == 0);
    assert(vbo == 0);
    

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(rendering::ForceInfo),
        reinterpret_cast<GLvoid*>(offsetof(rendering::ForceInfo, position))
    );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(rendering::ForceInfo),
        reinterpret_cast<GLvoid*>(offsetof(rendering::ForceInfo, size))
    );
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, sizeof(rendering::ForceInfo),
        reinterpret_cast<GLvoid*>(offsetof(rendering::ForceInfo, color))
    );
    glBindVertexArray(0);


    assert(vao != 0);
    assert(vbo != 0);
}

void updateWindowSize([[ maybe_unused ]] GLFWwindow* window, int width, int height) {
    ZoneScoped

    assert(window == _window); // Just in case something things to just add a window

    // Update render window
    glViewport(0, 0, width, height);

    {
        assert(_emitters.shaderProgram);
        glUseProgram(_emitters.shaderProgram);
        const GLint loc = glGetUniformLocation(_emitters.shaderProgram, "windowSize");
        assert(loc != -1);
        glUniform2i(loc, width, height);
    }
    {
        assert(_forces.shaderProgram);
        glUseProgram(_forces.shaderProgram);
        const GLint loc = glGetUniformLocation(_forces.shaderProgram, "windowSize");
        assert(loc != -1);
        glUniform2i(loc, width, height);
    }
}

// Stores the timestamp when the previous frame was finished
std::chrono::time_point<std::chrono::high_resolution_clock> prevFrameTime;

} // namespace

namespace rendering {

void createWindow() {
    ZoneScoped

    //
    // Initialize GLFW for window handling
    //
    constexpr const int Width = 850;
    constexpr const int Height = 850;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    assert(_window == nullptr);
    _window = glfwCreateWindow(Width, Height, "Particle System", nullptr, nullptr);
    glfwMakeContextCurrent(_window);
    glfwSwapInterval(0);
    glfwSetWindowSizeCallback(_window, updateWindowSize);

    //
    // Initialize the GLAD OpenGL wrapper
    // 
    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));

    //
    // Set the default OpenGL state
    //
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    //
    // Initialize ImGui UI library
    //
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    ImGui_ImplOpenGL3_Init();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.f;
    style.WindowTitleAlign.x = 0.5f;
    style.WindowMenuButtonPosition = 0;


    //
    // Setup OpenGL objects for particles
    //
    createParticleShader(
        _particles.shaderProgram,
        _particles.vertexShader, _particles.fragmentShader
    );
    createParticleGLObjects(_particles.vao, _particles.vbo);


    //
    // Setup OpenGL objects for emitters
    //
    createEmitterShader(
        _emitters.shaderProgram,
        _emitters.vertexShader, _emitters.geometryShader, _emitters.fragmentShader
    );
    createEmitterGLObjects(_emitters.vao, _emitters.vbo);


    //
    // Setup OpenGL objects for forces
    //
    createForceShader(
        _forces.shaderProgram,
        _forces.vertexShader, _forces.geometryShader, _forces.fragmentShader
    );
    createForceGLObjects(_forces.vao, _forces.vbo);


    // Sets the uniforms for the window height and width to specify the size of emitters
    // and forces
    updateWindowSize(_window, Width, Height);
    prevFrameTime = std::chrono::high_resolution_clock::now();
    checkOpenGLError("postInit");
}

void destroyWindow() {
    ZoneScoped
    // Destroy the GL objects for the three different renderable types
    _particles = {};
    _emitters = {};
    _forces = {};

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Cleanup GLFW
    glfwDestroyWindow(_window);
    glfwTerminate();
}

void setBackgroundColor(float r, float g, float b) {
    assert(r >= 0.f && r <= 1.f);
    assert(g >= 0.f && g <= 1.f);
    assert(b >= 0.f && b <= 1.f);

    _backgroundColor = { r, g, b };
}

float beginFrame() {
    ZoneScoped
    checkOpenGLError("beginFrame (begin)");
    // Compute how much time has passed since the beginning of the last frame
    auto currentTime = std::chrono::high_resolution_clock::now();
    const std::chrono::nanoseconds ns = currentTime - prevFrameTime;
    const double milliseconds = ns.count() / 1e9;
    const double dt = milliseconds;
    TracyPlot("deltatime", dt);
    prevFrameTime = currentTime;

    // Query the events from the operating system, such as input from mouse or keyboards
    glfwPollEvents();

    // Clear the rendering buffer with the selected background color
    glClearColor(_backgroundColor.r, _backgroundColor.g, _backgroundColor.b, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    checkOpenGLError("beginFrame (end)");
    return static_cast<float>(dt);
}

void renderParticles(const std::vector<ParticleInfo>& particleData) {
    ZoneScoped
    checkOpenGLError("updateParticles (begin)");

    assert(_particles.vao);
    assert(_particles.vbo);
    assert(_particles.shaderProgram);

    // Upload the passed particle information to the GPU
    glBindBuffer(GL_ARRAY_BUFFER, _particles.vbo);
    glBufferData(GL_ARRAY_BUFFER, particleData.size() * sizeof(ParticleInfo), particleData.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Plot the number of particles and make them available through Tracy
    TracyPlot("Particles", int64_t(_particles.number));

    glBindVertexArray(_particles.vao);
    glUseProgram(_particles.shaderProgram);
    glDrawArrays(GL_POINTS, 0, static_cast<int>(particleData.size()));
    glUseProgram(0);
    glBindVertexArray(0);

    checkOpenGLError("updateParticles (end)");
}

void renderEmitters(const std::vector<EmitterInfo>& emitterData) {
    ZoneScoped
    checkOpenGLError("updateEmitters (begin)");

    assert(_emitters.vao);
    assert(_emitters.vbo);
    assert(_emitters.shaderProgram);

    // Upload the passed emitter information to the GPU
    glBindBuffer(GL_ARRAY_BUFFER, _emitters.vbo);
    glBufferData(GL_ARRAY_BUFFER, emitterData.size() * sizeof(EmitterInfo), emitterData.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Plot the number of emitters and make them available through Tracy
    TracyPlot("Emitters", int64_t(_emitters.number));

    glBindVertexArray(_emitters.vao);
    glUseProgram(_emitters.shaderProgram);
    glDrawArrays(GL_POINTS, 0, static_cast<int>(emitterData.size()));
    glUseProgram(0);
    glBindVertexArray(0);

    checkOpenGLError("updateEmitters (end)");
}

void renderForces(const std::vector<ForceInfo>& forceData) {
    ZoneScoped
    checkOpenGLError("renderForces (begin)");

    assert(_forces.vao);
    assert(_forces.vbo);
    assert(_forces.shaderProgram);

    // Upload the passed forces information to the GPU
    glBindBuffer(GL_ARRAY_BUFFER, _forces.vbo);
    glBufferData(GL_ARRAY_BUFFER, forceData.size() * sizeof(ForceInfo), forceData.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);    

    // Plot the number of forces and make them available through Tracy
    TracyPlot("Forces", int64_t(forceData.size()));

    glBindVertexArray(_forces.vao);
    glUseProgram(_forces.shaderProgram);
    glDrawArrays(GL_POINTS, 0, static_cast<int>(forceData.size()));
    glUseProgram(0);
    glBindVertexArray(0);

    checkOpenGLError("renderForces (end)");
}

bool endFrame() {
    ZoneScoped
    checkOpenGLError("endFrame (begin)");

    {
        // Swapping the front and back buffer. Since we are doing v-sync is enabled, this
        // call will block until its our turn to swap the buffers (usually every 16.6 ms
        // on a 60 Hz monitor
        ZoneScopedN("Swap buffers")
        glfwSwapBuffers(_window);
    }

    checkOpenGLError("endFrame (end)");
    const bool shouldClose = glfwWindowShouldClose(_window);
    FrameMark
    return !shouldClose;
}

} // namespace rendering

namespace ui {


GuiScope::GuiScope() {
    // Signal to ImGui that we are starting with a new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // The start of a new (and only) window
    ImGui::Begin("UI");
}

GuiScope::~GuiScope() {
    // Finalize the ImGui ui elements and render them to the screen
    ZoneScopedN("Render UI")
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void text(const char* text, Color color) {
    assert(text);
    ZoneScoped
    ImGui::TextColored({color.r, color.g, color.b, 1.0f}, "%s", text);
}

bool sliderFloat(const char* label, float& value, float minValue, float maxValue) {
    assert(label);
    ZoneScoped
    return ImGui::SliderFloat(label, &value, minValue, maxValue);
}

bool sliderInt(const char* label, int& value, int minValue, int maxValue) {
    assert(label);
    ZoneScoped
    return ImGui::SliderInt(label, &value, minValue, maxValue);
}

bool sliderVec2(const char* label, vec2& value, float minValue, float maxValue) {
    assert(label);
    ZoneScoped
    const float speed = (maxValue - minValue) / 2000.f;
    float* v = &value.x;
    return ImGui::DragFloat2(label, v, speed, minValue, maxValue);
}

bool colorPicker(const char* label, Color& color) {
    assert(label);
    ZoneScoped
    float* v = &color.r;
    return ImGui::ColorEdit3(label, v);
}

bool button(const char* label) {
    assert(label);
    ZoneScoped
    return ImGui::Button(label);
}

bool checkbox(const char* label, bool& value) {
    assert(label);
    ZoneScoped
    return ImGui::Checkbox(label, &value);
}

void beginGroup(const char* label) {
    ZoneScoped
    assert(label);
    ImGui::PushID(label);
    ImGui::Text("%s", label);
}

void endGroup() {
    ImGui::Separator();
    ImGui::PopID();
}

} // namespace ui
