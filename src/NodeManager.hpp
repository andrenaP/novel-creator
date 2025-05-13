#ifndef NODE_MANAGER_HPP
#define NODE_MANAGER_HPP

#include "Types.hpp"
#include "raylib.h"
#include "raygui.h"
#include <vector>
#include <memory>
#include <string>
#include <sstream>

class Draggable {
public:
    virtual bool isMouseOver(Vector2 pos, Vector2 size, Vector2 offset) const = 0;
    virtual void move(Vector2& target, Vector2 delta) = 0;
    virtual ~Draggable() = default;
};

class SimpleDrag : public Draggable {
public:
    bool isMouseOver(Vector2 pos, Vector2 size, Vector2 offset) const override {
        Vector2 mouse = GetMousePosition();
        float mx = mouse.x - offset.x;
        float my = mouse.y - offset.y;
        return (mx >= pos.x && mx <= pos.x + size.x && my >= pos.y && my <= pos.y + size.y);
    }

    void move(Vector2& target, Vector2 delta) override {
        target.x += delta.x;
        target.y += delta.y;
    }
};

class SnappingDrag : public Draggable {
public:
    bool isMouseOver(Vector2 pos, Vector2 size, Vector2 offset) const override {
        Vector2 mouse = GetMousePosition();
        float mx = mouse.x - offset.x;
        float my = mouse.y - offset.y;
        return (mx >= pos.x && mx <= pos.x + size.x && my >= pos.y && my <= pos.y + size.y);
    }

    void move(Vector2& target, Vector2 delta) override {
        target.x += delta.x;
        target.y += delta.y;
        target.x = std::round(target.x / 50.0f) * 50.0f;
        target.y = std::round(target.y / 50.0f) * 50.0f;
    }
};

class NodeManager {
public:
    NodeManager(std::vector<Scene>& scenes) : scenes(scenes), offset{0, 0}, draggingNode(-1), draggingCanvas(false), creatingConnection(false), selectedNode(-1), editingNode(false), editTextFlag(false) {
        nodes.emplace_back(Node{"Start Node", {}, {}, {100, 100}});
    }

    void update() {
        Vector2 mouse = GetMousePosition();
        Vector2 mouseDelta = GetMouseDelta();

        // Handle dragging nodes
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            draggingNode = -1;
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (isMouseOverNode(i)) {
                    draggingNode = static_cast<int>(i);
                    break;
                }
            }
        }

        if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) {
            draggingNode = -1;
        }

        if (draggingNode != -1) {
            nodes[draggingNode].position.x += mouseDelta.x;
            nodes[draggingNode].position.y += mouseDelta.y;
            if (snapToGrid) {
                nodes[draggingNode].position.x = std::round(nodes[draggingNode].position.x / 50.0f) * 50.0f;
                nodes[draggingNode].position.y = std::round(nodes[draggingNode].position.y / 50.0f) * 50.0f;
            }
        }

        // Handle canvas dragging
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !isMouseOverAnyNode()) {
            draggingCanvas = true;
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            draggingCanvas = false;
            if (creatingConnection) {
                for (size_t i = 0; i < nodes.size(); ++i) {
                    if (i != fromNode && isMouseOverNodeInput(i)) {
                        nodes[fromNode].connections.push_back({i, "Choice " + std::to_string(nodes[fromNode].connections.size() + 1)});
                        break;
                    }
                }
                creatingConnection = false;
            }
        }

        if (draggingCanvas) {
            offset.x += mouseDelta.x;
            offset.y += mouseDelta.y;
        }

        // Handle connection creation
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (isMouseOverNodeOutput(i)) {
                    creatingConnection = true;
                    fromNode = i;
                    break;
                }
            }
        }

        // Handle node editing
        if (IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON)) {
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (isMouseOverNode(i)) {
                    selectedNode = static_cast<int>(i);
                    editingNode = true;
                    strncpy(textBuffer, nodes[i].name.c_str(), 256);
                    editTextFlag = false;
                    break;
                }
            }
        }
    }

    void draw() {
        // Draw connections
        for (size_t i = 0; i < nodes.size(); ++i) {
            for (const auto& conn : nodes[i].connections) {
                Vector2 start = getNodeOutputPos(i);
                Vector2 end = getNodeInputPos(conn.toNodeIndex);
                DrawLineBezier(start, end, 2.0f, DARKGRAY);
                DrawText(conn.choiceText.c_str(), (start.x + end.x) / 2, (start.y + end.y) / 2 - 10, 10, BLACK);
            }
        }

        // Draw nodes
        for (size_t i = 0; i < nodes.size(); ++i) {
            Vector2 pos = {nodes[i].position.x + offset.x, nodes[i].position.y + offset.y};
            Rectangle rect = {pos.x, pos.y, 150, 60};
            DrawRectangleRounded(rect, 0.2f, 10, LIGHTGRAY);
            // DrawRectangleRoundedLines(rect, 0.2f, 10, 2.0f, BLACK); // Fixed: Changed 2 to 2.0f
            DrawText(nodes[i].name.c_str(), pos.x + 10, pos.y + 5, 12, BLACK);

            // Draw input/output slots
            DrawCircleV(getNodeInputPos(i), 6, BLUE);
            DrawCircleV(getNodeOutputPos(i), 6, RED);
        }

        // Draw connection in progress
        if (creatingConnection) {
            Vector2 start = getNodeOutputPos(fromNode);
            Vector2 end = GetMousePosition();
            DrawLineBezier(start, end, 2.0f, RED);
        }

        // Draw node editor UI
        if (editingNode && selectedNode != -1) {
            drawEditUI();
        }
    }

    void addNode(float x, float y) {
        nodes.emplace_back(Node{"Node " + std::to_string(nodes.size() + 1), {}, {}, {x, y}});
    }

    std::vector<Node>& getNodes() { return nodes; }

private:
    bool isMouseOverNode(size_t index) {
        Vector2 pos = {nodes[index].position.x + offset.x, nodes[index].position.y + offset.y};
        Vector2 size = {150, 60};
        Vector2 mouse = GetMousePosition();
        return (mouse.x >= pos.x && mouse.x <= pos.x + size.x && mouse.y >= pos.y && mouse.y <= pos.y + size.y);
    }

    bool isMouseOverNodeInput(size_t index) {
        Vector2 inputPos = getNodeInputPos(index);
        Vector2 mouse = GetMousePosition();
        return CheckCollisionPointCircle(mouse, inputPos, 6);
    }

    bool isMouseOverNodeOutput(size_t index) {
        Vector2 outputPos = getNodeOutputPos(index);
        Vector2 mouse = GetMousePosition();
        return CheckCollisionPointCircle(mouse, outputPos, 6);
    }

    bool isMouseOverAnyNode() {
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (isMouseOverNode(i)) return true;
        }
        return false;
    }

    Vector2 getNodeInputPos(size_t index) {
        return {nodes[index].position.x + offset.x, nodes[index].position.y + offset.y + 30};
    }

    Vector2 getNodeOutputPos(size_t index) {
        return {nodes[index].position.x + offset.x + 150, nodes[index].position.y + offset.y + 30};
    }

    void drawEditUI() {
        DrawRectangle(600, 50, 180, 300, LIGHTGRAY);
        DrawText("Edit Node", 610, 60, 20, BLACK);

        // Node name
        DrawText("Name:", 610, 90, 12, BLACK);
        if (GuiTextBox({610, 110, 160, 20}, textBuffer, 256, editTextFlag)) {
            editTextFlag = !editTextFlag;
        }

        // Scene selection
        DrawText("Scenes:", 610, 140, 12, BLACK);
        static int selectedScene = 0;
        std::vector<std::string> sceneNames;
        for (const auto& scene : scenes) {
            sceneNames.push_back(scene.name);
        }
        std::string dropdownText;
        for (size_t i = 0; i < sceneNames.size(); ++i) {
            dropdownText += sceneNames[i];
            if (i < sceneNames.size() - 1) dropdownText += ";";
        }
        if (GuiDropdownBox({610, 160, 160, 20}, dropdownText.c_str(), &selectedScene, false)) {
            if (selectedScene >= 0 && selectedScene < static_cast<int>(scenes.size())) {
                if (std::find(nodes[selectedNode].sceneIndices.begin(), nodes[selectedNode].sceneIndices.end(), selectedScene) == nodes[selectedNode].sceneIndices.end()) {
                    nodes[selectedNode].sceneIndices.push_back(selectedScene);
                }
            }
        }

        // Display selected scenes
        std::string sceneList = "Selected Scenes: ";
        for (size_t idx : nodes[selectedNode].sceneIndices) {
            if (idx < scenes.size()) {
                sceneList += scenes[idx].name + ", ";
            }
        }
        DrawText(sceneList.c_str(), 610, 190, 10, BLACK);

        // Save and Close buttons
        if (GuiButton({610, 220, 160, 20}, "Save")) {
            nodes[selectedNode].name = textBuffer;
        }
        if (GuiButton({610, 250, 160, 20}, "Close")) {
            editingNode = false;
            selectedNode = -1;
        }
    }

    std::vector<Node> nodes;
    std::vector<Scene>& scenes;
    Vector2 offset;
    int draggingNode;
    bool draggingCanvas;
    bool creatingConnection;
    size_t fromNode;
    int selectedNode;
    bool editingNode;
    char textBuffer[256] = "";
    bool editTextFlag;
    bool snapToGrid = true;
};

#endif // NODE_MANAGER_HPP
