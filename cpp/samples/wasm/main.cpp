/**
 * @file main.cpp
 * @brief Emscripten/WebGL sample (Vista SDK Gmod Explorer)
 * @details Renders an interactive Gmod tree browser with VIS version selector,
 *          compiled to WebAssembly via Emscripten.
 */

#include <dnv/VistaSDK.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>
#include <GLFW/emscripten_glfw3.h>

#include <emscripten.h>
#include <emscripten/html5.h>

#include <algorithm>
#include <cctype>
#include <functional>
#include <optional>
#include <string>
#include <vector>

using namespace dnv::vista::sdk;

//==============================================================================
// Application state
//==============================================================================

struct App
{
    GLFWwindow* window = nullptr;
    const VIS* vis = nullptr;
    VisVersion selectedVersion{};
    const GmodNode* selectedNode = nullptr;

    char searchBuf[128] = {};

    std::vector<VisVersion> versions;
};

static App g_app;

//==============================================================================
// Helpers
//==============================================================================

static std::string visVersionLabel(VisVersion v)
{
    return "VIS " + std::string{ VisVersions::toString(v) };
}

static int naturalCompare(std::string_view a, std::string_view b)
{
    size_t i = 0, j = 0;
    while (i < a.size() && j < b.size())
    {
        if (std::isdigit(static_cast<unsigned char>(a[i])) && std::isdigit(static_cast<unsigned char>(b[j])))
        {
            size_t ni = i, nj = j;
            while (ni < a.size() && std::isdigit(static_cast<unsigned char>(a[ni])))
            {
                ++ni;
            }
            while (nj < b.size() && std::isdigit(static_cast<unsigned char>(b[nj])))
            {
                ++nj;
            }
            size_t la = ni - i, lb = nj - j;
            if (la != lb)
            {
                return la < lb ? -1 : 1;
            }
            int cmp = a.substr(i, la).compare(b.substr(j, lb));
            if (cmp != 0)
            {
                return cmp;
            }
            i = ni;
            j = nj;
        }
        else
        {
            if (a[i] != b[j])
            {
                return a[i] < b[j] ? -1 : 1;
            }
            ++i;
            ++j;
        }
    }
    if (a.size() == b.size())
    {
        return 0;
    }
    return a.size() < b.size() ? -1 : 1;
}

static bool naturalLess(const GmodNode* a, const GmodNode* b)
{
    return naturalCompare(a->code(), b->code()) < 0;
}

static bool containsIcase(std::string_view haystack, std::string_view needle)
{
    if (needle.empty())
    {
        return true;
    }
    auto it = std::search(
        haystack.begin(), haystack.end(), needle.begin(), needle.end(), [](unsigned char a, unsigned char b) {
            return std::tolower(a) == std::tolower(b);
        });
    return it != haystack.end();
}

static bool subtreeMatches(const GmodNode& node, std::string_view filter, std::vector<const GmodNode*>& visited)
{
    for (const auto* v : visited)
    {
        if (v == &node)
        {
            return false;
        }
    }
    visited.push_back(&node);

    const std::string_view code = node.code();
    const std::string_view name = node.metadata().name();
    if (containsIcase(code, filter) || containsIcase(name, filter))
    {
        return true;
    }
    for (const GmodNode* child : node.children())
    {
        if (subtreeMatches(*child, filter, visited))
        {
            return true;
        }
    }
    return false;
}

static void renderNode(
    const GmodNode& node, std::string_view filter, std::vector<const GmodNode*>& visited, bool forceOpen = false)
{
    // Cycle guard (Gmod is a DAG, not a strict tree)
    for (const auto* v : visited)
    {
        if (v == &node)
        {
            return;
        }
    }
    visited.push_back(&node);

    const std::string_view code = node.code();
    const std::string_view name = node.metadata().name();

    bool matchesSelf = filter.empty() || containsIcase(code, filter) || containsIcase(name, filter);

    // Check if any descendant matches so we can keep the parent visible
    bool matchesSubtree = matchesSelf;
    if (!filter.empty() && !matchesSelf)
    {
        std::vector<const GmodNode*> tmp;
        tmp.push_back(&node);
        for (const GmodNode* child : node.children())
        {
            if (subtreeMatches(*child, filter, tmp))
            {
                matchesSubtree = true;
                break;
            }
        }
    }

    if (!filter.empty() && !matchesSubtree)
    {
        return;
    }

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
    if (node.children().isEmpty())
    {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }
    if (&node == g_app.selectedNode)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (forceOpen || (!filter.empty() && matchesSubtree))
    {
        flags |= ImGuiTreeNodeFlags_DefaultOpen;
    }

    // Dim non-matching nodes (parent visible only because a child matches)
    if (!filter.empty() && !matchesSelf)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    }

    std::string label = std::string{ code } + "  " + std::string{ name };
    bool open = ImGui::TreeNodeEx(label.c_str(), flags);

    if (!filter.empty() && !matchesSelf)
    {
        ImGui::PopStyleColor();
    }

    if (ImGui::IsItemClicked())
    {
        g_app.selectedNode = &node;
    }

    if (open && !(flags & ImGuiTreeNodeFlags_Leaf))
    {
        std::vector<const GmodNode*> sorted;
        sorted.reserve(node.children().size());
        for (const GmodNode* child : node.children())
        {
            sorted.push_back(child);
        }
        std::sort(sorted.begin(), sorted.end(), naturalLess);

        for (const GmodNode* child : sorted)
        {
            renderNode(*child, filter, visited);
        }
        ImGui::TreePop();
    }
}

//==============================================================================
// UI
//==============================================================================

static void renderUI()
{
    const ImGuiIO& io = ImGui::GetIO();

    // Left panel: version selector + search + Gmod tree
    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize({ io.DisplaySize.x * 0.45f, io.DisplaySize.y });
    ImGui::Begin(
        "##tree",
        nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse);

    // Version selector
    ImGui::Text("VIS version");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(160.f);
    if (ImGui::BeginCombo("##ver", visVersionLabel(g_app.selectedVersion).c_str()))
    {
        for (VisVersion v : g_app.versions)
        {
            bool sel = (v == g_app.selectedVersion);
            if (ImGui::Selectable(visVersionLabel(v).c_str(), sel))
            {
                g_app.selectedVersion = v;
                g_app.selectedNode = nullptr;
            }
            if (sel)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Separator();

    // Search bar
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputTextWithHint("##search", "Filter by code or name...", g_app.searchBuf, sizeof(g_app.searchBuf));

    ImGui::Separator();

    // Gmod tree
    const Gmod& gmod = g_app.vis->gmod(g_app.selectedVersion);
    std::string_view filter{ g_app.searchBuf };

    ImGui::BeginChild("##treeScroll", { 0, 0 }, false, ImGuiWindowFlags_HorizontalScrollbar);
    {
        std::vector<const GmodNode*> visited;
        visited.reserve(64);
        renderNode(gmod.rootNode(), filter, visited, true);
    }
    ImGui::EndChild();

    ImGui::End();

    // Right panel: selected node details
    ImGui::SetNextWindowPos({ io.DisplaySize.x * 0.45f, 0 });
    ImGui::SetNextWindowSize({ io.DisplaySize.x * 0.55f, io.DisplaySize.y });
    ImGui::Begin(
        "Node details", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    if (g_app.selectedNode != nullptr)
    {
        const GmodNode& node = *g_app.selectedNode;
        const GmodNodeMetadata& metadata = node.metadata();

        ImGui::Text("Code     : %s", std::string{ node.code() }.c_str());
        ImGui::Text("Name     : %s", std::string{ metadata.name() }.c_str());
        ImGui::Text("Category : %s", std::string{ metadata.category() }.c_str());
        ImGui::Text("Type     : %s", std::string{ metadata.type() }.c_str());

        ImGui::Separator();

        ImGui::Text("Mappable : %s", node.isMappable() ? "yes" : "no");
        ImGui::Text("Leaf     : %s", node.isLeafNode() ? "yes" : "no");
        ImGui::Text("Asset    : %s", node.isAsset() ? "yes" : "no");
        ImGui::Text("Function : %s", node.isFunctionNode() ? "yes" : "no");

        if (metadata.commonName().has_value())
        {
            ImGui::Separator();
            ImGui::Text("Common name: %s", metadata.commonName()->c_str());
        }

        if (metadata.definition().has_value())
        {
            ImGui::Separator();
            ImGui::TextWrapped("Definition: %s", metadata.definition()->c_str());
        }

        ImGui::Separator();
        ImGui::Text("Children : %zu", node.children().size());
        ImGui::Text("Parents  : %zu", node.parents().size());

        if (node.productType().has_value())
        {
            ImGui::Separator();
            ImGui::Text(
                "Product type: %s - %s",
                std::string{ node.productType().value()->code() }.c_str(),
                std::string{ node.productType().value()->metadata().name() }.c_str());
        }

        if (node.productSelection().has_value())
        {
            ImGui::Separator();
            ImGui::Text(
                "Product selection: %s - %s",
                std::string{ node.productSelection().value()->code() }.c_str(),
                std::string{ node.productSelection().value()->metadata().name() }.c_str());
        }
    }
    else
    {
        ImGui::TextDisabled("Select a node in the tree to see its details.");
    }

    ImGui::End();
}

//==============================================================================
// Main loop
//==============================================================================

static void mainLoopIteration()
{
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    renderUI();

    ImGui::Render();

    const ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y));
    glClearColor(0.10f, 0.10f, 0.12f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(g_app.window);
}

//==============================================================================
// Entry point
//==============================================================================

int main()
{
    // GLFW + OpenGL ES 3 init
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    emscripten_glfw_set_next_window_canvas_selector("#canvas");

    g_app.window = glfwCreateWindow(1280, 720, "Vista SDK - Gmod Explorer", nullptr, nullptr);
    glfwMakeContextCurrent(g_app.window);
    glfwSwapInterval(1);

    emscripten_glfw_make_canvas_resizable(g_app.window, "window", nullptr);

    // ImGui init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(g_app.window, true);
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(g_app.window, "#canvas");
    ImGui_ImplOpenGL3_Init("#version 100");

    // Vista SDK init
    g_app.vis = &VIS::instance();
    g_app.selectedVersion = g_app.vis->latest();

    for (VisVersion v : g_app.vis->versions())
    {
        g_app.versions.push_back(v);
    }
    std::sort(g_app.versions.begin(), g_app.versions.end(), [](VisVersion a, VisVersion b) {
        return static_cast<uint8_t>(a) > static_cast<uint8_t>(b);
    });

    // Run
    emscripten_set_main_loop(mainLoopIteration, 0, true);

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(g_app.window);
    glfwTerminate();

    return 0;
}
