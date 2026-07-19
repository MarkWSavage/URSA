// Track Length / Chord Length Distribution Estimator
//
// Implements Approximation No. 2 (Eq. 1.38) for the cumulative chord length
// distribution C(s) of a rectangular parallelepiped, from:
//   G. C. Messenger and M. S. Ash, "Single Event Phenomena", Chapter 1,
//   Section 1.4 "Chord Distribution Functions".
//
// For a parallelepiped with dimensions a, b, c (c = smallest dimension)
// and space diagonal s_max = sqrt(a^2 + b^2 + c^2):
//
//   k = 2.37c / (1.80c + s_max)
//
//   C(s) = [1 - k] * [(s_max^3.8 - s^3.8) / (s_max^3.8 - c^3.8)] * (c/s)^2   for s >= c
//   C(s) = 1 - k * (s/c)                                                    for s <  c
//
// C(s) is the probability that a randomly oriented, isotropic particle
// track chord through the volume has length >= s.

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

// ---------------------------------------------------------------------
// Eq. (1.38): cumulative chord length distribution, Approximation No. 2
// ---------------------------------------------------------------------
static double chordDistributionC(double s, double c, double sMax)
{
    if (s <= 0.0) return 1.0;
    if (s >= sMax) return 0.0;

    const double k = 2.37 * c / (1.80 * c + sMax);

    if (s >= c) {
        const double sMax38 = std::pow(sMax, 3.8);
        const double s38    = std::pow(s, 3.8);
        const double c38    = std::pow(c, 3.8);
        const double denom  = sMax38 - c38;
        const double ratio  = (denom > 1e-12) ? (sMax38 - s38) / denom : 0.0;
        const double cOverS = c / s;
        return (1.0 - k) * ratio * cOverS * cOverS;
    } else {
        return 1.0 - k * (s / c);
    }
}

int main()
{
    if (!glfwInit()) {
        std::fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }

    const char* glslVersion = "#version 130";
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glslVersion = "#version 150";
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    GLFWwindow* window = glfwCreateWindow(
        1000, 750, "Track Length Estimator - Messenger & Ash Ch.1 Eq.1.38",
        nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glslVersion);

    // Default dimensions taken from the book's worked example (Figs. 1.5/1.6)
    float dimA = 24.0f; // um
    float dimB = 18.0f; // um
    float dimC = 16.0f; // um
    float sProbe = 5.0f; // chord length probe, um

    std::vector<double> curveX, curveY;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Track Length Estimator", nullptr,
                      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                          ImGuiWindowFlags_NoMove);

        ImGui::TextWrapped(
            "Rectangular Parallelepiped Chord Length Estimator");
        ImGui::TextDisabled(
            "Messenger & Ash, \"Single Event Phenomena\", Ch. 1 - "
            "Approximation No. 2 (Eq. 1.38)");
        ImGui::Separator();

        ImGui::SliderFloat("Length a (um)", &dimA, 0.5f, 100.0f, "%.2f");
        ImGui::SliderFloat("Width  b (um)", &dimB, 0.5f, 100.0f, "%.2f");
        ImGui::SliderFloat("Height c (um)", &dimC, 0.5f, 100.0f, "%.2f");

        const double a = dimA, b = dimB, c = dimC;
        const double volume = a * b * c;

        // The formula requires c to be the *smallest* dimension; sort so
        // this holds no matter which slider the user drags below the others.
        double dims[3] = {a, b, c};
        std::sort(dims, dims + 3);
        const double cMin = dims[0];
        const double sMax = std::sqrt(a * a + b * b + c * c);

        ImGui::Separator();
        ImGui::Text("Volume  V = a x b x c = %.3f um^3", volume);
        ImGui::Text("Max chord length (space diagonal) s_max = %.3f um", sMax);
        ImGui::Text("Smallest dimension used in formula, c = %.3f um", cMin);

        ImGui::Separator();
        if (sProbe > (float)sMax) sProbe = (float)sMax;
        ImGui::SliderFloat("Chord length s (um)", &sProbe, 0.0f, (float)sMax,
                            "%.3f");

        const double Cs = chordDistributionC(sProbe, cMin, sMax);
        ImGui::Text("C(s) = P(chord length >= %.3f um) = %.5f", sProbe, Cs);

        ImGui::Separator();
        ImGui::Text("Cumulative Chord Length Distribution C(s)");

        const int N = 400;
        curveX.resize(N);
        curveY.resize(N);
        for (int i = 0; i < N; ++i) {
            const double s = sMax * (double)i / (double)(N - 1);
            curveX[i] = s;
            curveY[i] = chordDistributionC(s, cMin, sMax);
        }

        if (ImPlot::BeginPlot("##CvsS", ImVec2(-1, 420))) {
            ImPlot::SetupAxes("Chord length s (um)",
                               "C(s)  (Probability of length >= s)");
            ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, sMax, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, 1e-4, 1.2, ImGuiCond_Always);

            ImPlot::PlotLine("C(s) - Approx. No. 2", curveX.data(),
                              curveY.data(), N);

            double markerX[1] = {sProbe};
            double markerY[1] = {std::max(Cs, 1e-4)};
            ImPlot::PlotScatter("Current s", markerX, markerY, 1);

            ImPlot::EndPlot();
        }

        ImGui::End();

        ImGui::Render();
        int displayW, displayH;
        glfwGetFramebufferSize(window, &displayW, &displayH);
        glViewport(0, 0, displayW, displayH);
        glClearColor(0.10f, 0.10f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
