#include "Render.hpp"
#include <algorithm>
#include "raylib.h"
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
        TraceLog(LOG_WARNING, "No valid scene for node %d, showing buttons", currentNodeIndex);
    }

    // Handle spacebar for next slide
    if (IsKeyPressed(KEY_SPACE) && canGoNext()) {
        nextSlide();
        TraceLog(LOG_INFO, "Spacebar pressed, advanced to slide %d", currentSlide);
    }

    if (showButtons) {
        float mouseWheelMove = GetMouseWheelMove();
        if (mouseWheelMove != 0) {
            scrollOffset.y -= mouseWheelMove * 20.0f;
            float maxScroll = nodes[currentNodeIndex].connections.size() * buttonSpacing - GetScreenHeight() * 0.7f;
            if (maxScroll < 0) maxScroll = 0;
            scrollOffset.y = std::max(0.0f, std::min(scrollOffset.y, maxScroll));
            TraceLog(LOG_INFO, "Scroll offset updated to %.2f (maxScroll: %.2f)", scrollOffset.y, maxScroll);
        }
    }
}

void Render::draw() {
    if (currentNodeIndex < 0 || currentNodeIndex >= (int)nodes.size()) {
        DrawText("No node selected", 10, 10, 20, RED);
        TraceLog(LOG_ERROR, "Cannot draw: invalid node index %d", currentNodeIndex);
        return;
    }

    const Node& currentNode = nodes[currentNodeIndex];
    if (currentNode.sceneIndex >= 0 && currentNode.sceneIndex < (int)scenes.size()) {
        drawScene(scenes[currentNode.sceneIndex], GetTime(), currentSlide);
    } else {
        TraceLog(LOG_WARNING, "Invalid scene index %d for node %d", currentNode.sceneIndex, currentNodeIndex);
    }

    // Draw navigation and reset buttons
    float buttonWidth = 100.0f;
    float buttonHeight = 30.0f;
    float totalNavWidth = buttonWidth * 3 + 40.0f; // Three buttons with 20px gaps
    float navButtonX = (GetScreenWidth() - totalNavWidth) / 2.0f; // Center horizontally
    float navButtonY = (GetScreenHeight() - buttonHeight) / 2.0f - 120.0f; // Center vertically, above choice buttons

    if (canGoPrev()) {
        if (GuiButton({navButtonX, navButtonY, buttonWidth, buttonHeight}, "Previous Slide")) {
            prevSlide();
            TraceLog(LOG_INFO, "Previous slide button clicked, moved to slide %d", currentSlide);
        }
    }
    if (canGoNext()) {
        if (GuiButton({navButtonX + buttonWidth + 20.0f, navButtonY, buttonWidth, buttonHeight}, "Next Slide")) {
            nextSlide();
            TraceLog(LOG_INFO, "Next slide button clicked, advanced to slide %d", currentSlide);
        }
    }
    if (GuiButton({navButtonX + buttonWidth * 2 + 40.0f, navButtonY, buttonWidth, buttonHeight}, "Reset")) {
        resetSlide();
        TraceLog(LOG_INFO, "Reset button clicked, reset to node %d and slide 1", currentNodeIndex);
    }

    if (showButtons) {
        if (nodes[currentNodeIndex].connections.empty()) {
            float textWidth = MeasureText("No choices available", 20);
            DrawText("No choices available", (GetScreenWidth() - textWidth) / 2, GetScreenHeight() / 2 - 100, 20, RED);
            TraceLog(LOG_WARNING, "Node %d has no connections", currentNodeIndex);
        } else {
            float buttonWidth = 200.0f;
            float buttonHeight = 30.0f;
            float boxWidth = buttonWidth + 20.0f;
            size_t connectionsSize = nodes[currentNodeIndex].connections.size();
            float totalChoicesHeight = connectionsSize * buttonSpacing;
            float boxHeight = std::min((float)GetScreenHeight() * 0.7f, totalChoicesHeight + 40.0f);
            float boxX = (GetScreenWidth() - boxWidth) / 2.0f;
            float boxY = (GetScreenHeight() - boxHeight) / 2.0f; // Center vertically
            GuiGroupBox({boxX, boxY, boxWidth, boxHeight}, "Choices");
            BeginScissorMode(boxX, boxY, boxWidth, boxHeight);
            TraceLog(LOG_INFO, "Rendering %zu choice buttons for node %d (boxHeight: %.2f, totalChoicesHeight: %.2f)",
                     connectionsSize, currentNodeIndex, boxHeight, totalChoicesHeight);
            for (size_t i = 0; i < connectionsSize; ++i) {
                if (i >= nodes[currentNodeIndex].connections.size()) {
                    TraceLog(LOG_ERROR, "Index %zu exceeds connections size %zu for node %d", i, nodes[currentNodeIndex].connections.size(), currentNodeIndex);
                    break;
                }
                float yPos = boxY + 10.0f + i * buttonSpacing - scrollOffset.y;
                Rectangle buttonRect = {boxX + 10.0f, yPos, buttonWidth, buttonHeight};
                if (yPos + buttonHeight > boxY && yPos < boxY + boxHeight) {
                    if (GuiButton(buttonRect, nodes[currentNodeIndex].connections[i].choiceText.c_str())) {
                        int newNodeIndex = nodes[currentNodeIndex].connections[i].toNodeIndex;
                        if (newNodeIndex >= 0 && newNodeIndex < (int)nodes.size()) {
                            TraceLog(LOG_INFO, "Switching to node %d (sceneIndex: %d) via choice %zu (text: %s)",
                                     newNodeIndex, nodes[newNodeIndex].sceneIndex, i,
                                     nodes[currentNodeIndex].connections[i].choiceText.c_str());
                            currentNodeIndex = newNodeIndex;
                            currentSlide = 1;
                            scrollOffset.y = 0.0f;
                            break; // Exit loop to prevent further accesses after node change
                        } else {
                            TraceLog(LOG_ERROR, "Invalid node index %d in connection %zu for node %d", newNodeIndex, i, currentNodeIndex);
                        }
                    }
                    TraceLog(LOG_INFO, "Rendered choice %zu at yPos: %.2f (text: %s) for node %d", i, yPos,
                             nodes[currentNodeIndex].connections[i].choiceText.c_str(), currentNodeIndex);
                } else {
                    TraceLog(LOG_WARNING, "Choice %zu at yPos: %.2f is outside visible area (boxY: %.2f, boxHeight: %.2f) for node %d",
                             i, yPos, boxY, boxHeight, currentNodeIndex);
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
        currentSlide = 1; // Reset slide when changing nodes
        scrollOffset.y = 0.0f;
        TraceLog(LOG_INFO, "Set current node to %d (sceneIndex: %d)", currentNodeIndex, nodes[currentNodeIndex].sceneIndex);
    } else {
        TraceLog(LOG_ERROR, "Attempted to set invalid node index %d", index);
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
    // Find the start node
    bool foundStartNode = false;
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (nodes[i].isStartNode) {
            currentNodeIndex = i;
            foundStartNode = true;
            break;
        }
    }
    // Fallback to first node if no start node is found
    if (!foundStartNode && !nodes.empty()) {
        currentNodeIndex = 0;
        TraceLog(LOG_WARNING, "No start node found, falling back to node 0 (sceneIndex: %d)", nodes[0].sceneIndex);
    } else if (!foundStartNode) {
        currentNodeIndex = -1;
        TraceLog(LOG_ERROR, "No nodes available for reset");
    }
    currentSlide = 1;
    scrollOffset.y = 0.0f;
    if (currentNodeIndex >= 0 && nodes[currentNodeIndex].sceneIndex >= 0 && nodes[currentNodeIndex].sceneIndex < (int)scenes.size()) {
        TraceLog(LOG_INFO, "Reset to node %d (sceneIndex: %d) and slide 1", currentNodeIndex, nodes[currentNodeIndex].sceneIndex);
    } else {
        TraceLog(LOG_WARNING, "Reset failed: invalid node %d or sceneIndex %d", currentNodeIndex, currentNodeIndex >= 0 ? nodes[currentNodeIndex].sceneIndex : -1);
    }
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
