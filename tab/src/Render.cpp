#include "Render.hpp"
#include <algorithm>
#include "raygui.h"

Render::Render(std::vector<Element>& elements, std::vector<Scene>& scenes, std::vector<Node>& nodes)
    : elements(elements), scenes(scenes), nodes(nodes), currentNodeIndex(-1), currentSlide(1),
      scrollOffset({0, 0}), buttonSpacing(40.0f), showButtons(false) {}

void Render::update(float currentTime, int currentSlide) {
    this->currentSlide = currentSlide;
    if (currentNodeIndex < 0 || currentNodeIndex >= (int)nodes.size()) {
        showButtons = false;
        TraceLog(LOG_WARNING, "No valid node selected (index: %d)", currentNodeIndex);
        return;
    }

    const Node& currentNode = nodes[currentNodeIndex];
    showButtons = true;

    if (currentNode.sceneIndex >= 0 && currentNode.sceneIndex < (int)scenes.size()) {
        const Scene& scene = scenes[currentNode.sceneIndex];
        bool allElementsDone = true;
        float maxEndTime = 0.0f;
        for (const auto& sceneElement : scene.elements) {
            if (sceneElement.elementIndex < elements.size()) {
                maxEndTime = std::max(maxEndTime, sceneElement.endTime);
                if (currentSlide >= sceneElement.startTime && currentSlide <= sceneElement.endTime) {
                    allElementsDone = false;
                    TraceLog(LOG_INFO, "Element %zu (name: %s) is active: startTime=%.2f, endTime=%.2f",
                             sceneElement.elementIndex,
                             elements[sceneElement.elementIndex].name.c_str(),
                             sceneElement.startTime,
                             sceneElement.endTime);
                }
            }
        }
        showButtons = allElementsDone || currentSlide >= maxEndTime;
        TraceLog(LOG_INFO, "Slide %d, Max EndTime: %.2f, All Elements Done: %d, Show Buttons: %d",
                 currentSlide, maxEndTime, allElementsDone, showButtons);
    } else {
        showButtons = true;
        TraceLog(LOG_INFO, "No valid scene for node %d, showing buttons", currentNodeIndex);
    }

    if (showButtons) {
        float mouseWheelMove = GetMouseWheelMove();
        if (mouseWheelMove != 0) {
            scrollOffset.y -= mouseWheelMove * 20.0f;
            float maxScroll = nodes[currentNodeIndex].connections.size() * buttonSpacing - GetScreenHeight() * 0.3f;
            if (maxScroll < 0) maxScroll = 0;
            scrollOffset.y = std::max(0.0f, std::min(scrollOffset.y, maxScroll));
        }

        Vector2 mousePos = GetMousePosition();
        for (size_t i = 0; i < nodes[currentNodeIndex].connections.size(); ++i) {
            float yPos = GetScreenHeight() - 150.0f + i * buttonSpacing - scrollOffset.y;
            Rectangle buttonRect = {GetScreenWidth() - 220.0f, yPos, 200.0f, 30.0f};
            if (CheckCollisionPointRec(mousePos, buttonRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentNodeIndex = nodes[currentNodeIndex].connections[i].toNodeIndex;
                resetSlide();
                scrollOffset.y = 0.0f;
                TraceLog(LOG_INFO, "Switched to node %d", currentNodeIndex);
                break;
            }
        }
    }
}

void Render::draw() {
    if (currentNodeIndex < 0 || currentNodeIndex >= (int)nodes.size()) {
        DrawText("No node selected", 10, 10, 20, RED);
        return;
    }

    const Node& currentNode = nodes[currentNodeIndex];
    if (currentNode.sceneIndex >= 0 && currentNode.sceneIndex < (int)scenes.size()) {
        drawScene(scenes[currentNode.sceneIndex], GetTime(), currentSlide);
    }

    if (showButtons) {
        if (nodes[currentNodeIndex].connections.empty()) {
            DrawText("No choices available", GetScreenWidth() - 220, GetScreenHeight() - 180, 20, RED);
            TraceLog(LOG_WARNING, "Node %d has no connections", currentNodeIndex);
        } else {
            GuiGroupBox({GetScreenWidth() - 230.0f, GetScreenHeight() - 200.0f, 220.0f, 180.0f}, "Choices");
            BeginScissorMode(GetScreenWidth() - 230, GetScreenHeight() - 180, 220, 160);
            for (size_t i = 0; i < nodes[currentNodeIndex].connections.size(); ++i) {
                float yPos = GetScreenHeight() - 150.0f + i * buttonSpacing - scrollOffset.y;
                if (yPos > GetScreenHeight() - 180.0f && yPos < GetScreenHeight() - 20.0f) {
                    if (GuiButton({GetScreenWidth() - 220.0f, yPos, 200.0f, 30.0f},
                                  nodes[currentNodeIndex].connections[i].choiceText.c_str())) {
                        currentNodeIndex = nodes[currentNodeIndex].connections[i].toNodeIndex;
                        resetSlide();
                        scrollOffset.y = 0.0f;
                        TraceLog(LOG_INFO, "Switched to node %d via button", currentNodeIndex);
                    }
                }
            }
            EndScissorMode();
        }
    }
}

void Render::drawScene(const Scene& scene, float currentTime, int currentSlide) {
    // Collect all elements to render
    std::vector<std::pair<const SceneElement*, const Element*>> renderElements;
    for (const auto& sceneElement : scene.elements) {
        if (sceneElement.elementIndex < elements.size() &&
            currentSlide >= sceneElement.startTime && currentSlide <= sceneElement.endTime) {
            renderElements.emplace_back(&sceneElement, &elements[sceneElement.elementIndex]);
        }
    }

    // Separate characters and sort by positionIndex
    std::vector<std::pair<const SceneElement*, const Element*>> characterElements;
    for (const auto& [sceneElement, element] : renderElements) {
        if (element->type == ElementType::CHARACTER) {
            characterElements.emplace_back(sceneElement, element);
        }
    }
    std::sort(characterElements.begin(), characterElements.end(),
              [](const auto& a, const auto& b) {
                  const auto& charA = std::get<CharacterElement>(a.second->data);
                  const auto& charB = std::get<CharacterElement>(b.second->data);
                  return charA.positionIndex < charB.positionIndex;
              });

    // Calculate spacing for characters
    int characterCount = characterElements.size();
    float screenWidth = (float)GetScreenWidth();
    float characterWidth = 0.0f; // Approximate width per character (will be updated)
    float spacing = 50.0f; // Fixed spacing between characters
    if (characterCount > 0) {
        // Estimate character width based on the first character's texture
        const auto& firstCharacter = std::get<CharacterElement>(characterElements[0].second->data);
        for (size_t i = 0; i < firstCharacter.textures.size(); ++i) {
            if (firstCharacter.textures[i].id > 0) {
                characterWidth = firstCharacter.textures[i].width * 0.5f; // Scale is 0.5f
                break;
            }
        }
    }
    float totalWidth = characterCount * characterWidth + (characterCount > 1 ? (characterCount - 1) * spacing : 0);
    float startX = (screenWidth - totalWidth) / 2.0f; // Center the group

    // Render non-character elements, sorted by renderlevel
    std::vector<std::pair<const SceneElement*, const Element*>> nonCharacterElements;
    for (const auto& [sceneElement, element] : renderElements) {
        if (element->type != ElementType::CHARACTER) {
            nonCharacterElements.emplace_back(sceneElement, element);
        }
    }
    std::sort(nonCharacterElements.begin(), nonCharacterElements.end(),
              [](const auto& a, const auto& b) { return a.first->renderlevel < b.first->renderlevel; });

    for (const auto& [sceneElement, element] : nonCharacterElements) {
        drawElement(*sceneElement, *element, currentTime, currentSlide, characterCount, 0, spacing, startX);
    }

    // Render characters in positionIndex order
    for (size_t i = 0; i < characterElements.size(); ++i) {
        drawElement(*characterElements[i].first, *characterElements[i].second, currentTime, currentSlide, characterCount, i, spacing, startX);
    }
}

void Render::drawElement(const SceneElement& sceneElement, const Element& element, float currentTime, int currentSlide, int characterCount, int currentCharacterIndex, float spacing, float startX) {
    if (element.type == ElementType::TEXT) {
        const auto& text = std::get<TextElement>(element.data);
        int textWidth = MeasureText(text.content.c_str(), 20);
        DrawText(text.content.c_str(),
                 (GetScreenWidth() - textWidth) / 2,
                 GetScreenHeight() - 50,
                 20,
                 BLACK);
    } else if (element.type == ElementType::BACKGROUND) {
        const auto& bg = std::get<BackgroundElement>(element.data);
        if (bg.texture.id > 0) {
            float scaleX = (float)GetScreenWidth() / bg.texture.width;
            float scaleY = (float)GetScreenHeight() / bg.texture.height;
            float scale = std::max(scaleX, scaleY);
            DrawTextureEx(bg.texture,
                          {0, 0},
                          0.0f,
                          scale,
                          WHITE);
        }
    } else if (element.type == ElementType::CHARACTER) {
        const auto& character = std::get<CharacterElement>(element.data);
        for (size_t i = 0; i < character.images.size(); ++i) {
            if (character.images[i].first == sceneElement.selectedPose && i < character.textures.size() &&
                character.textures[i].id > 0) {
                float scale = 0.5f;
                float characterWidth = character.textures[i].width * scale;
                // Calculate posX: startX + index * (characterWidth + spacing)
                float posX = startX + currentCharacterIndex * (characterWidth + spacing);
                float posY = GetScreenHeight() - character.textures[i].height * scale;
                DrawTextureEx(character.textures[i],
                              {posX, posY},
                              0.0f,
                              scale,
                              WHITE);
                TraceLog(LOG_INFO, "Rendering character '%s' at posX=%.2f, posY=%.2f, positionIndex=%d",
                         character.name.c_str(), posX, posY, character.positionIndex);
                break;
            }
        }
    }
}

void Render::setCurrentNodeIndex(int index) {
    if (index >= 0 && index < (int)nodes.size()) {
        currentNodeIndex = index;
        resetSlide();
        scrollOffset.y = 0.0f;
        TraceLog(LOG_INFO, "Set current node to %d", currentNodeIndex);
    }
}

int Render::getCurrentNodeIndex() const {
    return currentNodeIndex;
}

void Render::nextSlide() {
    currentSlide++;
    TraceLog(LOG_INFO, "Advanced to slide %d", currentSlide);
}

void Render::prevSlide() {
    if (currentSlide > 1) {
        currentSlide--;
        TraceLog(LOG_INFO, "Moved back to slide %d", currentSlide);
    }
}

void Render::resetSlide() {
    currentSlide = 1;
    TraceLog(LOG_INFO, "Reset slide to 1");
}

int Render::getCurrentSlide() const {
    return currentSlide;
}

bool Render::canGoNext() const {
    if (currentNodeIndex < 0 || currentNodeIndex >= (int)nodes.size()) return false;
    if (nodes[currentNodeIndex].sceneIndex < 0 || nodes[currentNodeIndex].sceneIndex >= (int)scenes.size()) return false;
    const Scene& scene = scenes[nodes[currentNodeIndex].sceneIndex];
    for (const auto& sceneElement : scene.elements) {
        if (sceneElement.elementIndex < elements.size() && sceneElement.endTime > currentSlide) {
            return true;
        }
    }
    return false;
}

bool Render::canGoPrev() const {
    return currentSlide > 1;
}
