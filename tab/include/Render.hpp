#ifndef RENDER_HPP
#define RENDER_HPP

// #define RAYGUI_IMPLEMENTATION
// #include "raygui.h"
#include "Types.hpp"
// #include "raylib.h"
// #include "raygui.h"
#include <vector>
#include <string>


class Render {
public:
    Render(std::vector<Element>& elements, std::vector<Scene>& scenes, std::vector<Node>& nodes);
    void update(float currentTime, int currentSlide);
    void draw();
    void setCurrentNodeIndex(int index);
    int getCurrentNodeIndex() const;
    void nextSlide(); // New: Advance to next slide
    void prevSlide(); // New: Go back to previous slide
    void resetSlide(); // New: Reset slide to 1
    int getCurrentSlide() const; // New: Get current slide
    bool canGoNext() const; // New: Check if next slide is available
    bool canGoPrev() const; // New: Check if previous slide is available

private:
    std::vector<Element>& elements;
    std::vector<Scene>& scenes;
    std::vector<Node>& nodes;
    int currentNodeIndex;
    int currentSlide; // New: Track slide internally
    Vector2 scrollOffset;
    float buttonSpacing;
    bool showButtons;

    void drawScene(const Scene& scene, float currentTime, int currentSlide);
    void drawElement(const SceneElement& sceneElement, const Element& element, float currentTime, int currentSlide, int characterCount, int currentCharacterIndex, float spacing, float margin);
};

#endif // RENDER_HPP
