// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <random>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include "implot.h"
//#include "L2DFileDialog.h"
//#include "ImGuiManagerWidget.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif


static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void showPlot()
{
    static bool use_work_area = false;
    static ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar;

    // We demonstrate using the full viewport area or the work area (without
    // menu-bars, task-bars etc.) Based on your use case you may want one of the
    // other.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
    ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(0.0, 0.0));
    ImPlot::PushStyleVar(ImPlotStyleVar_MajorGridSize, ImVec2(2.0, 2.0));
    ImPlot::PushStyleVar(ImPlotStyleVar_MinorGridSize, ImVec2(1.0, 1.0));
    ImPlot::PushStyleColor(ImPlotCol_AxisGrid, ImVec4(0.5, 0.5, 0.5, 1.0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.0, 1.0));


    if (ImGui::Begin("Example: Fullscreen window", nullptr, flags)) {
        srand(0);
        static float xs1[100], ys1[100];
        for (int i = 0; i < 100; ++i) {
            xs1[i] = i * 0.01f;
            ys1[i] = xs1[i] + 0.1f * ((float)rand() / (float)RAND_MAX);
        }
        static float xs2[50], ys2[50];
        for (int i = 0; i < 50; i++) {
            xs2[i] = 0.25f + 0.2f * ((float)rand() / (float)RAND_MAX);
            ys2[i] = 0.75f + 0.2f * ((float)rand() / (float)RAND_MAX);
        }

        if (ImPlot::BeginPlot("Scatter Plot", NULL, NULL)) {
            if (ImPlot::IsPlotHovered()) printf("ok");
            ImPlot::PlotScatter("Data 1", xs1, ys1, 100);
            ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Square, 6, ImVec4(0, 1, 0, 0.5f), IMPLOT_AUTO, ImVec4(0, 1, 0, 1));
            ImPlot::PlotScatter("Data 2", xs2, ys2, 50);
            ImPlot::PopStyleVar();
            ImPlot::EndPlot();
        }

        ImGui::PopStyleVar();
    }
    ImGui::End();


}

void menubar(ImGuiStyle& style) {
    ImGui::Text("menubar");
}

void showRight(ImGuiStyle& style) {
    ImGui::Text("right");
}

void showLeft(ImGuiStyle& style) {
    ImGui::Text("left");
}

using namespace ImGui;
bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f)
{
    using namespace ImGui;
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID("##Splitter");
    ImRect bb;
    bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
    bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
    return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}

bool LoadTextureFromFile(const char* filename, GLuint* out_texture,
    int* out_width, int* out_height) {
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height,
        NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

void funShowSettings() {

    float width = 100;
    static bool antiAliasedLines;
    static float line_weight;
    static float chart_width;
    static float r_limit;

    ImGui::SetCursorPosY(16);

    if (ImGui::TreeNode("Configuration##1")) {
        ImGui::BeginGroup();

        ImGuiStyle& style = ImGui::GetStyle();

        ImPlotStyle& stylePlot = ImPlot::GetStyle();
        ImPlotInputMap& map = ImPlot::GetInputMap();

        static float d0 = 9.001;

        ImGui::BeginChild("##text1", ImVec2(200, 30), false);
        Text("Scroll Speed");
        EndChild();
        SameLine();

        ImGui::BeginChild("##text2", ImVec2(200, 30), false);
        ImGui::InputFloat("##ScrollSpeed", &map.WheelRate, 0.01f, 1.0f, "%.3f");
        EndChild();

        ImGui::BeginChild("##text3", ImVec2(200, 30), false);
        Text("Zoom Rate");
        EndChild();

        SameLine();
        ImGui::BeginChild("##text4", ImVec2(200, 30), false);
        ImGui::InputFloat("##ZoomRate", &map.ZoomRate, 0.01f, 1.0f, "%.3f");
        EndChild();
        
        ImGui::BeginChild("##text5", ImVec2(200, 30), false);
        Text("Anti-aliased lines");
        EndChild();
        SameLine();
        ImGui::BeginChild("##text6", ImVec2(200, 30), false);
        ImGui::Checkbox("##antialiased", &antiAliasedLines);
        EndChild();

        //stylePlot.AntiAliasedLines = antiAliasedLines;
        //style.AntiAliasedLines = stylePlot.AntiAliasedLines;
        ImGui::SetNextItemWidth(width);
        //colorPicker(&logViewerSettings.background_color, "Background Color");
        ImGui::BeginChild("##text7", ImVec2(200, 30), false);
        ImGui::Text("Font Selector");
        EndChild();
        SameLine();
        ImGui::BeginChild("##text8", ImVec2(200, 30), false);
        ImGui::ShowFontSelector("Font Selector");
        EndChild();


        ImGui::BeginChild("##text9", ImVec2(200, 30), false);
        ImGui::Text("Line Thickness");
        EndChild();
        SameLine();

        ImGui::BeginChild("##text10", ImVec2(200, 30), false);
        ImGui::InputFloat("##ZoomRate", &map.LineRate, 0.01f, 1.0f, "%.3f");
        //ImGui::DragFloat("##linethickness", &line_weight, 0.1f, 0.0f, 3.0f, "%.2f");
        EndChild();


        ImGui::BeginChild("##text11", ImVec2(200, 30), false);
        ImGui::Text("Track Width");
        EndChild();

        SameLine();

        ImGui::BeginChild("##text12", ImVec2(200, 30), false);
        ImGui::InputFloat("##ZoomRate", &map.TrackRate, 0.01f, 1.0f, "%.3f");
        //ImGui::DragFloat("##trackwidth", &chart_width, 5.0f, 150.0f, 500.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        EndChild();

        ImGui::BeginChild("##text13", ImVec2(200, 30), false);
        ImGui::Text("Range Limit");
        EndChild();

        SameLine();
        ImGui::BeginChild("##text14", ImVec2(200, 30), false);
        ImGui::InputFloat("##ZoomRate", &map.RangeRate, 0.01f, 1.0f, "%.3f");
        //if (ImGui::DragFloat("##rangelimit", &r_limit, 10.0f, 200.0f, 2000.0f,
        //    "%.2f", ImGuiSliderFlags_AlwaysClamp)) {
        //    //logViewerSettings.markerClicked.first = true;
        //    //logViewerSettings.markerClicked.second = logViewerSettings.r_limit + lims->Y.Min;
        //}
        EndChild();
        ImGui::EndGroup();
        ImGui::TreePop();
    }
}

void colorPicker(ImVec4* tagColor, std::string name) {
    bool saved_palette_init = true;
    ImVec4 color = *tagColor;
    ImVec4 saved_palette[32] = { };
    if (saved_palette_init) {
        for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++) {
            ImGui::ColorConvertHSVtoRGB(n / 31.0f, 0.8f, 0.8f,
                saved_palette[n].x, saved_palette[n].y, saved_palette[n].z);
            saved_palette[n].w = 1.0f; // Alpha
        }
        saved_palette_init = false;
    }
    ImVec4 backup_color;
    bool open_popup = ImGui::ColorButton(("MyColor##3b" + name).c_str(), color);
    ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
    open_popup |= ImGui::Button((name + "##" + name).c_str());
    if (open_popup) {
        ImGui::OpenPopup(("mypicker" + name).c_str());
        backup_color = color;
    }
    if (ImGui::BeginPopup(("mypicker" + name).c_str())) {
        ImGui::Text("MY CUSTOM COLOR PICKER WITH AN AMAZING PALETTE!");
        ImGui::Separator();
        ImGui::ColorPicker4(("##picker" + name).c_str(), (float*)&color,
            ImGuiColorEditFlags_NoSidePreview
            | ImGuiColorEditFlags_NoSmallPreview);
        ImGui::SameLine();

        ImGui::BeginGroup(); // Lock X position
        ImGui::Text("Current");
        ImGui::ColorButton(("##current" + name).c_str(), color,
            ImGuiColorEditFlags_NoPicker
            | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40));
        ImGui::Text("Previous");
        if (ImGui::ColorButton(("##previous" + name).c_str(), backup_color,
            ImGuiColorEditFlags_NoPicker
            | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40)))
            color = backup_color;
        ImGui::Separator();
        ImGui::Text("Pick Your Color");
        for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++) {
            ImGui::PushID(n);
            if ((n % 8) != 0)
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.y);

            ImGuiColorEditFlags palette_button_flags =
                ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker
                | ImGuiColorEditFlags_NoTooltip;
            if (ImGui::ColorButton(("##palette" + name).c_str(),
                saved_palette[n], palette_button_flags, ImVec2(20, 20)))
                color = ImVec4(saved_palette[n].x, saved_palette[n].y,
                    saved_palette[n].z, color.w); // Preserve alpha!

            // Allow user to drop colors into each palette entry. Note that ColorButton() is already a
            // drag source by default, unless specifying the ImGuiColorEditFlags_NoDragDrop flag.
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(
                    IMGUI_PAYLOAD_TYPE_COLOR_3F))
                    memcpy((float*)&saved_palette[n], payload->Data,
                        sizeof(float) * 3);
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(
                    IMGUI_PAYLOAD_TYPE_COLOR_4F))
                    memcpy((float*)&saved_palette[n], payload->Data,
                        sizeof(float) * 4);

                ImGui::EndDragDropTarget();
            }

            ImGui::PopID();
        }
        if (ImGui::Button(("Close ##" + name).c_str())) {
            ImGui::CloseCurrentPopup();
        }
        *tagColor = color;
        ImGui::EndGroup();
        ImGui::EndPopup();
    }
}

void ToggleButton(const char* str_id, bool* v)
{
    ImVec4* colors = ImGui::GetStyle().Colors;
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float width = height * 1.55f;
    float radius = height * 0.50f;

    ImGui::InvisibleButton(str_id, ImVec2(width, height));
    if (ImGui::IsItemClicked()) *v = !*v;
    ImGuiContext& gg = *GImGui;
    float ANIM_SPEED = 0.085f;
    if (gg.LastActiveId == gg.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
        float t_anim = ImSaturate(gg.LastActiveIdTimer / ANIM_SPEED);
    if (ImGui::IsItemHovered())
        draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_ButtonActive] : ImVec4(0.78f, 0.78f, 0.78f, 1.0f)), height * 0.5f);
    else
        draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_Button] : ImVec4(0.85f, 0.85f, 0.85f, 1.0f)), height * 0.50f);
    draw_list->AddCircleFilled(ImVec2(p.x + radius + (*v ? 1 : 0) * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
}

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlotContext* ctx = ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    //ImGuiManagerWidget w;

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = NULL;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImPlot::SetCurrentContext(ctx);

        //showPlot();
        //ImGui::ShowDemoWindow();
        bool showPlot;
        static int showSettings = 0;
        ImVec2 windowSize(500, 600.0f);

        ImGuiStyle& style = ImGui::GetStyle();

        static ImGuiWindowFlags winflags = ImGuiWindowFlags_None;
        static ImGuiWindowFlags loseflags = ImGuiWindowFlags_AlwaysHorizontalScrollbar;
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

        
        ImGui::SetNextWindowSize(windowSize);

        ImFont* font1 = ImGui::GetFont();
        font1->Scale = 1.5f;

        static bool tabselection[3] = { false, true, false };
        
        //-----------------------------------------------------------------------------------
        float logView_thickness = 1.0f, logView_opacity = 1.0f;
        static bool logView_isGlobalThickness = false;
        static bool positiveFill = false;
        static bool negativeFill = false;
        static bool LogScale = false;
        ImVec4 pos =  ImVec4(0, 255, 255, 1);
        ImVec4 line =  ImVec4(0, 255, 255, 1);
        ImVec4 neg = ImVec4(0, 255, 255, 1);
        ImVec2 imButton = ImVec2(200, 200);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        GLuint my_image_texture = 1;
        //if (ImGui::BeginPopup(("Lname"))){
            if (ImGui::Begin("Settings", &showPlot, winflags)) {
                //ImGui::PushItemWidth(100);
                ImGui::SetCursorPosY(30);
                
                
                if (ImGui::TreeNode("Configuration##1")) {
                    ImGui::BeginGroup();

                    ImGuiStyle& style = ImGui::GetStyle();
                    ImPlotStyle& stylePlot = ImPlot::GetStyle();
                    ImPlotInputMap& map = ImPlot::GetInputMap();

                    ImGui::BeginChild("##text1", ImVec2(300, 300), false);
                    ImGui::ImageButton(" ", (void*)(intptr_t)my_image_texture, imButton);
                    ImGui::EndChild();
                    SameLine();
                    ImGui::BeginChild("##text2", ImVec2(200, 30), false);
                    ImGui::InputFloat("##Line Thickness", &map.LineRate, 0.01f, 1.0f, "%.3f");
                    ImGui::EndChild();

                    ImGui::BeginChild("##text3", ImVec2(200, 30), false);
                    Text("Global thickness");
                    ImGui::EndChild();
                    SameLine();
                    ImGui::BeginChild("##text4", ImVec2(200, 30), false);
                    ToggleButton("##Global thickness", &logView_isGlobalThickness);
                    ImGui::EndChild();

                    ImGui::BeginChild("##text5", ImVec2(200, 30), false);
                    Text("Fill opacity");
                    ImGui::EndChild();
                    SameLine();
                    ImGui::BeginChild("##text6", ImVec2(200, 30), false);
                    ImGui::InputFloat("##Fill opacity", &map.OpacityRate, 0.01f, 1.0f, "%.3f");
                    ImGui::EndChild();

                    ImGui::BeginChild("##text7", ImVec2(200, 30), false);
                    Text("Reference");
                    ImGui::EndChild();
                    SameLine();
                    ImGui::BeginChild("##text8", ImVec2(200, 30), false);
                    ImGui::InputFloat("##Reference", &map.ReferenceRate, 0.01f, 1.0f, "%.3f");
                    ImGui::EndChild();

                    ImGui::EndGroup();
                    ImGui::TreePop();
                }

                //-------------------------------------------------------------------------------
                // ---------------Color treeNode ----------------------
                
                /*if (ImGui::TreeNode("Color##2")) {
                    ImGui::BeginGroup();

                    ImGuiStyle& style = ImGui::GetStyle();
                    ImPlotStyle& stylePlot = ImPlot::GetStyle();
                    ImPlotInputMap& map = ImPlot::GetInputMap();

                    ImGui::BeginChild("##text1", ImVec2(200, 30), false);
                    colorPicker(&line, "LineColor##1");
                    ImGui::EndChild();

                    ImGui::BeginChild("##text2", ImVec2(200, 30), false);
                    colorPicker(&pos, "Possitive Fill##2");
                    ImGui::EndChild();
                    ImGui::SameLine();
                    ImGui::BeginChild("##text3", ImVec2(150, 30), false);
                    Text("Possitive Fill");
                    ImGui::EndChild();
                    ImGui::SameLine();
                    ImGui::BeginChild("##text4", ImVec2(200, 30), false);
                    ImGui::Checkbox("##Possitive Fill", &positiveFill);
                    ImGui::EndChild();

                    ImGui::BeginChild("##text5", ImVec2(200, 30), false);
                    colorPicker(&neg, "Negative Fill##5");
                    ImGui::EndChild();
                    ImGui::SameLine();
                    ImGui::BeginChild("##text6", ImVec2(150, 30), false);
                    Text("Negative Fill");
                    ImGui::EndChild();
                    ImGui::SameLine();
                    ImGui::BeginChild("##text7", ImVec2(200, 30), false);
                    ImGui::Checkbox("##Negative Fill", &negativeFill);
                    ImGui::EndChild();
                    
                    ImGui::BeginChild("##text8", ImVec2(200, 30), false);
                    Text("Log Scale");
                    ImGui::EndChild();
                    ImGui::SameLine();
                    ImGui::BeginChild("##text9", ImVec2(200, 30), false);
                    ImGui::Checkbox("##Log Scale", &LogScale);
                    ImGui::EndChild();

                    ImGui::BeginChild("##text10", ImVec2(200, 30), false);
                    Text("Min");
                    ImGui::EndChild();
                    SameLine();
                    ImGui::BeginChild("##text11", ImVec2(200, 30), false);
                    ImGui::InputFloat("##Min", &map.Min, 0.01f, 1.0f, "%.3f");
                    ImGui::EndChild();
                   
                    ImGui::BeginChild("##text12", ImVec2(200, 30), false);
                    Text("Max");
                    ImGui::EndChild();
                    SameLine();
                    ImGui::BeginChild("##text13", ImVec2(200, 30), false);
                    ImGui::InputFloat("##Max", &map.Max, 0.01f, 1.0f, "%.3f");
                    ImGui::EndChild();
                    
                                    
                    ImGui::EndGroup();
                    ImGui::TreePop();
                }*/
                End();
                
                //---------------------------------------------------------------------------------------------
                

                static int logViewerSettings_histogramBins = 0;
                static bool logViewerSettings_histogramDensity = 0;
                static bool logViewerSettings_histogramCumulative = 0;
                static bool logViewerSettings_histogramOutliers = false;
                float width_size = 200, height_size = 30;
                ImGui::SetNextItemOpen(true);
                if (ImGui::TreeNode("Configuration##plot")) {
                    ImGui::SetNextItemWidth(200);
                    //--------------------------------------
                    Checkbox("##", &logViewerSettings_histogramDensity);
                    if (logViewerSettings_histogramDensity) {
                        ImGui::SameLine();
                        ImGui::Image((void*)(intptr_t)10, ImVec2(16, 16));
                        
                    }
                    ImGui::SameLine();
                    Text("AM_");

                    ImGui::BeginChild("##text1", ImVec2(width_size, height_size), false);
                    if (ImGui::RadioButton("Sqrt", logViewerSettings_histogramBins == ImPlotBin_Sqrt)) { logViewerSettings_histogramBins = ImPlotBin_Sqrt; } //ImGui::SameLine();
                    EndChild();
                   
                    ImGui::BeginChild("##text2", ImVec2(width_size, height_size), false);
                    if (ImGui::RadioButton("Sturges", logViewerSettings_histogramBins == ImPlotBin_Sturges)) { logViewerSettings_histogramBins = ImPlotBin_Sturges; }// ImGui::SameLine();
                    EndChild();
                    
                    ImGui::BeginChild("##text3", ImVec2(width_size, height_size), false);
                    if (ImGui::RadioButton("Rice", logViewerSettings_histogramBins == ImPlotBin_Rice)) { logViewerSettings_histogramBins = ImPlotBin_Rice; } //ImGui::SameLine();
                    EndChild();
                
                    ImGui::BeginChild("##text4", ImVec2(width_size, height_size), false);
                    if (ImGui::RadioButton("Scott", logViewerSettings_histogramBins == ImPlotBin_Scott)) { logViewerSettings_histogramBins = ImPlotBin_Scott; } //ImGui::SameLine();
                    EndChild();
                    
                    ImGui::BeginChild("##text5", ImVec2(width_size, height_size), false);
                    if (ImGui::RadioButton("N Bins", logViewerSettings_histogramBins >= 0)) { logViewerSettings_histogramBins = 50; }
                    EndChild();
                    //--------------------------------------------
                    ImGui::BeginChild("##text6", ImVec2(width_size, height_size*2), false);
                    if (logViewerSettings_histogramBins >= 0) {
                        //ImGui::SameLine();
                        ImGui::SetNextItemWidth(150);
                        ImGui::InputInt("##Bins", &logViewerSettings_histogramBins, 1, 100);

                        //ImGui::SliderInt("##Bins", &logViewerSettings_histogramBins, 1, 100);
                    }
                    EndChild();
                    //--------------------------------------------
                    ImGui::BeginChild("##text7", ImVec2(width_size/2 +10, height_size), false);
                    Text("Density");
                    EndChild();
                    SameLine();
                    ImGui::BeginChild("##text8", ImVec2(width_size, height_size), false);
                    ToggleButton("##Density", &logViewerSettings_histogramDensity);
                    if (logViewerSettings_histogramDensity) {
                        ImPlot::SetNextAxisToFit(ImAxis_X1);
                        ImPlot::SetNextAxisToFit(ImAxis_Y1);
                    }
                    EndChild();
                    
                   
                    //------------------------------------------
                    
                    ImGui::BeginChild("##text9", ImVec2(width_size / 2 +10, height_size), false);
                    Text("Cumulative");
                    EndChild();
                    SameLine();
                    ImGui::BeginChild("##text10", ImVec2(width_size , height_size), false);
                    ToggleButton("##Cumulative", &logViewerSettings_histogramCumulative);
                    if (logViewerSettings_histogramCumulative)
                    {
                        ImPlot::SetNextAxisToFit(ImAxis_X1);
                        ImPlot::SetNextAxisToFit(ImAxis_Y1);
                    }
                    EndChild();

                    //---------------------------------------------
                    ImGui::BeginChild("##text11", ImVec2(width_size / 2 +10, height_size), false);
                    static bool range = false;
                    Text("Range");
                    EndChild();
                    SameLine();
                    ImGui::BeginChild("##text12", ImVec2(width_size, height_size), false);
                    ToggleButton("##Range", &range);
                    EndChild();
                    static float rmin = -3;
                    static float rmax = 13;
                    if (range) {
                        //ImGui::SameLine();
                        ImGui::SetNextItemWidth(200);
                        ImGui::BeginChild("##text13", ImVec2(width_size, height_size), false);
                        ImGui::DragFloat2("##Range", &rmin, 0.1f, -3, 13);
                        EndChild();
                        
                        //ImGui::SameLine();
                        ImGui::BeginChild("##text14", ImVec2(width_size/2 + 10, height_size), false);
                        Text("Outliers");
                        EndChild();
                        SameLine();
                        ImGui::BeginChild("##text15", ImVec2(width_size, height_size), false);
                        ToggleButton("##Outliers", &logViewerSettings_histogramOutliers);
                        EndChild();
                    }
                    
                    ImGui::TreePop();
                }
                //--------------------------------------------------------------------------------------------
                // filling option

                /*ImGui::SliderFloat("Line Thickness##",&logView_thickness, 0.0f, 3.0f, "ratio = %.2f");

                ImGui::Checkbox("Global thickness ##",&logView_isGlobalThickness);
                ImGui::SliderFloat(("Fill opacity##" , &logView_opacity, 0.0f, 1.0f, "ratio = %.2f");*/
                //logData->logView->updateFillColor();


                //color picker
                
                
               /* if (ImGui::Checkbox(("Possitive Fill ##checkbox" +
                    logView->logName).c_str(), &logView->positiveFill)) {
                    if (logView->positiveFill)
                        logView->negativeFill = true;
                }
                colorPicker(&logView->negativeFillColor,
                    "Negative Fill##" + lname);
                ImGui::SameLine();
                ImGui::Checkbox(("Negative Fill ##checkbox" + logView->logName).c_str(),
                    &logView->negativeFill);

                if (ImGui::Checkbox(("Log Scale ##" + logView->logName).c_str(),
                    &logView->logScalse)) {
                    logView->activeChart->updateFlag(logView);
                }


                ImGui::DragScalar(
                    ("Min##" + logView->logName).c_str(),
                    ImGuiDataType_Double, &logView->globalAttrMin,
                    (logView->globalAttrMax - logView->globalAttrMin) / 20.0f, &min,
                    &max, "%0.3f");
                ImGui::DragScalar(
                    ("Max##" + logView->logName).c_str(),
                    ImGuiDataType_Double, &logView->globalAttrMax,
                    (logView->globalAttrMax - logView->globalAttrMin) / 20.0f, &min,
                    &max, "%0.3f");*/
                //ImGui::PopItemWidth();
                //ImGui::EndPopup();
             
            }
        //}          
        ImGui::PopStyleVar();

#if 0
        float h = 200;
        static float sz1 = 100;
        static float sz2 = 250;
        bool showPlot = true;

        static bool use_work_area = true;
        float leftPanelWidth = 100.0f;
        float menubarHeight = 35;
        float leftWidth = 350;
        float scrollWidth = 20;
        float wwidth = 900;
        float wheight = 600;
        float padding;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
        ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

        ImGuiStyle& style = ImGui::GetStyle();

        static ImGuiWindowFlags winflags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration;


        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        if (ImGui::Begin("Plot a well logs", &showPlot, winflags)) {
            if (ImGui::BeginChild("##menu", ImVec2(-1, 32), false,
                ImGuiWindowFlags_NoScrollbar)) {
                //menubar(style);
                ImGui::EndChild();
            }

            if (ImGui::BeginChild("##container", ImVec2(-2, 0), true)) {
                //float sz2 = 0;
                //Splitter(true, 8.0f, &leftWidth, &sz2, 8, 8, -1);
                if (ImGui::BeginChild("##left", ImVec2(leftWidth, 0), true)) {
                    //showLeftChildView();
                    ImGui::BeginChild("#left1", ImVec2(sz1, 0), true);
                    ImGui::EndChild();

                    //ImGui::SameLine();
                    //ImGui::InvisibleButton("vsplitter", ImVec2(8.0f, 0));
                    //Splitter(true, 8.0f, &sz1, &sz2, 8, 8, -1);
                    //ImGui::SameLine();
                    ImGui::SetCursorPosX(leftPanelWidth);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetWindowHeight() - style.FramePadding.y);
                    ImGui::BeginChild("#lef2", ImVec2(sz2, 0), true);
                    ImGui::EndChild();
                    ImGui::EndChild();
                }


                //ImGui::SameLine();


                ImGui::SetCursorPosX(leftWidth);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetWindowHeight() - 5);

                if (ImGui::BeginChild("##right", ImVec2(ImGui::GetWindowWidth() - leftWidth, 0), true)) {
                    if (ImGui::BeginChild("##chartheader", ImVec2(0, 80), true)) {
                        //chartHeader();
                        ImGui::EndChild();
                    }
                    if (ImGui::BeginChild("##chartplot", ImVec2(0, 0), false)) {
                        //showRightChildView();
                        ImGui::EndChild();
                    }
                    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - scrollWidth);
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetWindowHeight() - 5 + 80);
                    if (ImGui::BeginChild("##scroll", ImVec2(scrollWidth, 0), true)) {
                        //showSliderScrollChart();
                        ImGui::EndChild();
                    }
                    ImGui::EndChild(); // end right
                }
                ImGui::EndChild(); // end container
            }
            ImGui::End(); // end plot
        }
        ImGui::PopStyleVar();
#endif
#if 0
        ImGui::Begin("Splitter test");

        static float w = 200.0f;
        static float h = 300.0f;
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::BeginChild("child1", ImVec2(w, h), true);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::InvisibleButton("vsplitter", ImVec2(8.0f, h));
        if (ImGui::IsItemActive())
            w += ImGui::GetIO().MouseDelta.x;
        ImGui::SameLine();
        ImGui::BeginChild("child2", ImVec2(0, h), true);
        ImGui::EndChild();
        ImGui::InvisibleButton("hsplitter", ImVec2(-1, 8.0f));
        if (ImGui::IsItemActive())
            h += ImGui::GetIO().MouseDelta.y;
        ImGui::BeginChild("child3", ImVec2(0, 0), true);
        ImGui::EndChild();
        ImGui::PopStyleVar();

        ImGui::End();
#endif
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
