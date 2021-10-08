#include <GLFW/glfw3.h>
#include <math.h>
#include <stdio.h>

#include <iostream>
#include <string>
#include <vector>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "extra.h"
#include "font/opensans.cpp"

//-----------------------------------------------------------------------------
// ANCHOR OBJECTS
//-----------------------------------------------------------------------------

ImVec2 GetRegionWithPadding() {
  ImVec2 ret;
  ImVec2 region = ImGui::GetWindowContentRegionMax();
  ImVec2 padding = ImGui::GetStyle().WindowPadding;
  ret.x = region.x + padding.x;
  ret.y = region.y + padding.y;
  return ret;
}

struct MainWindow {};

class Window {
 public:
  std::string name;
  bool state = true;
  ImVec2 pos;
  ImVec2 size;
  ImVec2 default_pos;
  ImVec2 default_size;
  ImVec2 cursor;

  void SetPos(ImVec2 p) {
    pos = p;
    ImGui::SetNextWindowPos(pos);
  }

  void SetSize(ImVec2 s) {
    size = s;
    ImGui::SetNextWindowSize(size);
  }

  void Begin(std::string title, ImGuiWindowFlags flags = 0) {
    name = title;
    ImGui::Begin(name.c_str(), &state, flags);
  }

  void End() { ImGui::End(); }

  // We use GetWindowContentRegionMax() to get the focused window size. A bit
  // tricky when WindowPadding is not 0,0
  void Varsizer() { size = GetRegionWithPadding(); }

  void UpdateCursor(ImGuiIO io) {
    cursor = ImVec2(io.MousePos.x-pos.x, io.MousePos.y-pos.y);
  }

  ImVec2 GetCursor(ImGuiIO io) {
    cursor = ImVec2(io.MousePos.x-pos.x, io.MousePos.y-pos.y);
    return cursor;
  }

  Window(ImVec2 def_size = ImVec2(0, 0), ImVec2 def_pos = ImVec2(0, 0)) {
    default_size = def_size;
    size = def_size;
    default_pos = def_pos;
    pos = def_pos;
  }
};

void metrics(Window w,int w_w, int w_h, ImGuiIO io) {
  ImVec2 padding = ImGui::GetStyle().WindowPadding;
  ImGui::NewLine();
  ImGui::NewLine();
  ImGui::NewLine();
  ImGui::Text("%dx%d", w_w, w_h);
  ImGui::Text("name: %s", w.name.c_str());
  ImGui::Text("state: %d", w.state);
  ImGui::Text("pos: %f,%f", w.pos.x, w.pos.y);
  ImGui::Text("size: %f,%f", w.size.x, w.size.y);
  ImGui::Text("region: %f,%f", GetRegionWithPadding().x,
              GetRegionWithPadding().y);
  ImGui::Text("padding: %fx%f", padding.x, padding.y);
  ImGui::Text("Mouse SetPos: (%g, %g)", io.MousePos.x, io.MousePos.y);
}


//-----------------------------------------------------------------------------
// ANCHOR GLFW FUNCS
//-----------------------------------------------------------------------------

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && \
    !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

//-----------------------------------------------------------------------------
// SECTION MAIN FUNC()
// ANCHOR GLFW BOILERPLATE
//-----------------------------------------------------------------------------

int main(int argc, char *argv[]) {
  int w_w = 900;
  int w_h = 600;
  bool resizing = false;

  //---------------------------------------------------
  // ANCHOR ARGS
  //---------------------------------------------------
  std::vector<std::string> args(argv, argv + argc);

  for (size_t i = 1; i < args.size(); ++i) {
    if (args[i] == "-x") {
    }
  }
  //---------------------------------------------------

  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) return 1;

#if defined(IMGUI_IMPL_OPENGL_ES2)
  const char *glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
  const char *glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

  //-----------------------------------------------------------------------------
  // ANCHOR CREATE glwindow
  //-----------------------------------------------------------------------------

  glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
  glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER,
                 GLFW_TRUE);  // glwindow to transparent; handle color through
                              // (internal) ImGui Window;

  GLFWwindow *glwindow = glfwCreateWindow(w_w, w_h, "OpenGL", NULL, NULL);

  if (glwindow == NULL) return 1;

  glfwGetWindowSize(glwindow, &w_w, &w_h);

  glfwSetWindowUserPointer(glwindow, &resizing);

  glfwSetWindowSizeCallback(
      glwindow, [](GLFWwindow *window, int width, int height) {
        bool *resizing = static_cast<bool *>(glfwGetWindowUserPointer(window));
        *resizing = true;
      });

  glfwMakeContextCurrent(glwindow);
  glfwSwapInterval(1);  // Enable vsync
  extra::glfwSetWindowCenter(glwindow);

  //-----------------------------------------------------------------------------
  // ANCHOR CONTEXT
  //-----------------------------------------------------------------------------

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(glwindow, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  //-----------------------------------------------------------------------------
  // ANCHOR DEFAULT LAYOUT | Define relationships between windows
  //-----------------------------------------------------------------------------

  float mb_Sx = w_w;
  float mb_Sy = w_h / 24;
  float sb_Sx = w_w / 8;
  float sb_Sy = w_h - mb_Sy;
  // float vp_Sx = w_w - sb_Sx;
  // float vp_Sy = w_h - mb_Sy;

  float mb_Px = 0;
  float mb_Py = 0;
  float sb_Px = 0;
  float sb_Py = mb_Sy;
  // float vp_Px = sb_Sx;
  // float vp_Py = sb_Py;

  //-----------------------------------------------------------------------------
  // ANCHOR IMGUI WINDOWS
  //-----------------------------------------------------------------------------

  Window menubar(ImVec2(mb_Sx, mb_Sy), ImVec2(mb_Px, mb_Py));
  Window sidebar(ImVec2(sb_Sx, sb_Sy), ImVec2(sb_Px, sb_Py));
  Window viewport;  // We don't need a hardcoded layout size and pos for
                    // viewport since it is entirely dynamic and dependant on
                    // sidebar and menu

  //-----------------------------------------------------------------------------
  // ANCHOR STYLES
  //-----------------------------------------------------------------------------

  // io.Fonts->Build();
  io.IniFilename = NULL;
  ImVec4 bg = ImVec4(0.123f, 0.123f, 0.123, 1.00f);  // Main bg color
  //ImGuiStyle &style = ImGui::GetStyle();

  //-----------------------------------------------------------------------------
  // ANCHOR VARS
  //-----------------------------------------------------------------------------

  bool dbg = false;
  bool sty = false;
  bool met = false;
  bool exit = false;

  //-----------------------------------------------------------------------------
  // SECTION MAIN LOOP >>>>
  //-----------------------------------------------------------------------------

  while (!glfwWindowShouldClose(glwindow)) {
    if (exit) {
      break;
    }
    glfwPollEvents();
    glfwGetWindowSize(glwindow, &w_w, &w_h);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::SetNextWindowBgAlpha(0.00f);

    //-----------------------------------------------------------------------------
    // SECTION GUI
    //-----------------------------------------------------------------------------

    // ANCHOR MENUBAR
    if (menubar.state) {
      menubar.SetPos(menubar.pos);
      ImGui::SetNextWindowSizeConstraints(ImVec2(-1, 0), ImVec2(-1, FLT_MAX));
      menubar.SetSize(menubar.size);
      menubar.Begin("menubar",
                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar);
      // menubar.Varsizer();

      if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Debug")) {
          ImGui::MenuItem("Settings", NULL, &dbg);
          ImGui::MenuItem("Style Editor", NULL, &sty);
          ImGui::MenuItem("Metrics", NULL, &met);
          ImGui::MenuItem("Exit", NULL, &exit);
          ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
      }

      menubar.End();
    }

    // ANCHOR SIDEBAR
    if (sidebar.state) {
      sidebar.SetPos(sidebar.pos);
      ImGui::SetNextWindowSizeConstraints(
          ImVec2(0, -1),
          ImVec2(FLT_MAX, -1));  // Constrain resizing to horizontal only
      sidebar.SetSize(
          sidebar.size);  // Sidebar size will be variable/changable but with a
                          // default size/layout [initial_size in constructor()]

      sidebar.Begin("sidebar", ImGuiWindowFlags_NoTitleBar);
      sidebar.Varsizer();  // Update any changes to size
      if (resizing) {
        sidebar.size = sidebar.default_size;
        resizing = false;
      }
      

      // cursor();

      ImGui::ColorEdit3("Color", (float *)&bg, ImGuiColorEditFlags_Float);
      if (ImGui::Button("Export")) {
        std::string exp = "ImVec4 col = ImVec4(" + std::to_string(bg.x) + "f," +
                          std::to_string(bg.y) + "f," + std::to_string(bg.z) +
                          "f,1.00f);";
        ImGui::LogToClipboard();
        ImGui::LogText(exp.c_str());
        ImGui::LogFinish();
      }

      //cursor(sidebar);
      metrics(sidebar,w_w,w_h,io);

      if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape))) {
        break;
      }

      // sidebar.SetSize(ImGui::GetWindowContentRegionMax());

      // div.y = w_h;
      // div = ImGui::GetWindowContentRegionMax();

      // sidebar.SetSize(div);
      // sidebar.SetSize(div.x,div.y);

      sidebar.End();
    }

    // ANCHOR VIEWPORT
    if (viewport.state) {
      viewport.SetPos(ImVec2(sidebar.size.x, sidebar.pos.y));
      viewport.SetSize(ImVec2(w_w - sidebar.size.x, w_h - menubar.size.y));
      viewport.Begin("Viewport", ImGuiWindowFlags_NoResize |
                                     ImGuiWindowFlags_NoBringToFrontOnFocus);

      metrics(viewport,w_w, w_h, io);

      viewport.End();
    }

    // ANCHOR DEBUG
    if (dbg) {
      // ImGui::SetNextWindowPos(ImVec2(w_w/2,w_h/2));
      ImGui::SetNextWindowBgAlpha(0.35f);
      if (ImGui::Begin("dbg", &dbg,
                       ImGuiWindowFlags_AlwaysAutoResize |
                           ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::Text("hello");
        ImGui::End();
      }
    }

    if (sty) {
      ImGui::ShowStyleEditor();
    }

    if(met) {
      ImGui::ShowMetricsWindow(&met);
    }

    //! SECTION GUI End
    //-----------------------------------------------------------------------------
    // ANCHOR RENDER
    //-----------------------------------------------------------------------------

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(glwindow, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(bg.x * bg.w, bg.y * bg.w, bg.z * bg.w, bg.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(glwindow);
  }

  //! SECTION MAIN LOOP End <<<<
  //-----------------------------------------------------------------------------

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(glwindow);
  glfwTerminate();

  return 0;

  //! SECTION MAIN FUNC End()
  //-----------------------------------------------------------------------------
}