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
    NodeManager(std::vector<Scene>& scenes) : scenes(scenes), offset{0, 0}, draggingNode(-1), draggingCanvas(false), creatingConnection(false), selectedNode(-1), editingNode(false), editTextFlag(false), sceneDropdownEditMode(false), dragDropdownEditMode(false), colorDropdownEditMode(false), connDropdownEditMode(false), selectedConnection(-1), editChoiceTextFlag(false) {
        nodes.emplace_back(Node{"Start Node", -1, {}, {100, 100}, DragType::SIMPLE, LIGHTGRAY});
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
            auto& node = nodes[draggingNode];
            if (node.dragType == DragType::SIMPLE) {
                SimpleDrag().move(node.position, mouseDelta);
            } else {
                SnappingDrag().move(node.position, mouseDelta);
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
                        char buffer[256] = "Enter choice text";
                        nodes[fromNode].connections.push_back({i, buffer});
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
                    sceneDropdownEditMode = false;
                    dragDropdownEditMode = false;
                    colorDropdownEditMode = false;
                    connDropdownEditMode = false;
                    selectedConnection = -1;
                    editChoiceTextFlag = false;
                    break;
                }
            }
        }

        // Add node with 'N' key
        if (IsKeyPressed(KEY_N)) {
            addNode(mouse.x - offset.x, mouse.y - offset.y);
        }

        // Delete node with 'DELETE' key when a node is selected
        if (IsKeyPressed(KEY_DELETE) && selectedNode != -1) {
            deleteNode(selectedNode);
            selectedNode = -1;
            editingNode = false;
        }
    }

    void draw() {
        // Draw connections
        for (size_t i = 0; i < nodes.size(); ++i) {
            for (size_t j = 0; j < nodes[i].connections.size(); ++j) {
                const auto& conn = nodes[i].connections[j];
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
            DrawRectangleRounded(rect, 0.2f, 10, nodes[i].color);
            // DrawRectangleRoundedLines(rect, 0.2f, 10, 2.0f, BLACK);
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

        // Draw instructions
        DrawText("N: Add Node, DELETE: Remove Selected Node", 10, 580, 10, DARKGRAY);
    }

    void addNode(float x, float y) {
        nodes.emplace_back(Node{"Node " + std::to_string(nodes.size() + 1), -1, {}, {x, y}, DragType::SIMPLE, LIGHTGRAY});
    }

    void deleteNode(size_t index) {
        if (index >= nodes.size()) return;
        nodes.erase(nodes.begin() + index);
        for (auto& node : nodes) {
            auto it = node.connections.begin();
            while (it != node.connections.end()) {
                if (it->toNodeIndex == index || it->toNodeIndex > index) {
                    it = node.connections.erase(it);
                } else {
                    if (it->toNodeIndex > index) it->toNodeIndex--;
                    ++it;
                }
            }
        }
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
        DrawRectangle(600, 50, 180, 450, LIGHTGRAY);
        DrawText("Edit Node", 610, 60, 20, BLACK);

        // Node name
        DrawText("Name:", 610, 90, 12, BLACK);
        if (GuiTextBox({610, 110, 160, 20}, textBuffer, 256, editTextFlag)) {
            editTextFlag = !editTextFlag;
        }

        // Scene selection
        DrawText("Scene:", 610, 140, 12, BLACK);
        static int selectedScene = nodes[selectedNode].sceneIndex;
        std::string dropdownText = "None";
        if (!scenes.empty()) {
            dropdownText.clear();
            for (size_t i = 0; i < scenes.size(); ++i) {
                dropdownText += scenes[i].name;
                if (i < scenes.size() - 1) dropdownText += ";";
            }
        }
        if (GuiDropdownBox({610, 160, 160, 20}, dropdownText.c_str(), &selectedScene, sceneDropdownEditMode)) {
            sceneDropdownEditMode = !sceneDropdownEditMode;
            if (!sceneDropdownEditMode && selectedScene >= -1 && selectedScene < static_cast<int>(scenes.size())) {
                nodes[selectedNode].sceneIndex = selectedScene;
                TraceLog(LOG_INFO, "Scene selected: %d", selectedScene);
            }
        }

        // Drag type selection
        DrawText("Drag Type:", 610, 190, 12, BLACK);
        static int dragTypeIndex = static_cast<int>(nodes[selectedNode].dragType);
        if (GuiDropdownBox({610, 210, 160, 20}, "Simple;Snapping", &dragTypeIndex, dragDropdownEditMode)) {
            dragDropdownEditMode = !dragDropdownEditMode;
            if (!dragDropdownEditMode) {
                nodes[selectedNode].dragType = static_cast<DragType>(dragTypeIndex);
                TraceLog(LOG_INFO, "Drag type selected: %d", dragTypeIndex);
            }
        }

        // Color selection
        DrawText("Color:", 610, 240, 12, BLACK);
        static int colorIndex = 0;
        // Map current node color to dropdown index
        if (nodes[selectedNode].color.r == LIGHTGRAY.r && nodes[selectedNode].color.g == LIGHTGRAY.g && nodes[selectedNode].color.b == LIGHTGRAY.b) {
            colorIndex = 0;
        } else if (nodes[selectedNode].color.r == BLUE.r && nodes[selectedNode].color.g == BLUE.g && nodes[selectedNode].color.b == BLUE.b) {
            colorIndex = 1;
        } else if (nodes[selectedNode].color.r == GREEN.r && nodes[selectedNode].color.g == GREEN.g && nodes[selectedNode].color.b == GREEN.b) {
            colorIndex = 2;
        } else if (nodes[selectedNode].color.r == RED.r && nodes[selectedNode].color.g == RED.g && nodes[selectedNode].color.b == RED.b) {
            colorIndex = 3;
        }
        if (GuiDropdownBox({610, 260, 160, 20}, "Light Gray;Blue;Green;Red", &colorIndex, colorDropdownEditMode)) {
            colorDropdownEditMode = !colorDropdownEditMode;
            if (!colorDropdownEditMode) {
                switch (colorIndex) {
                    case 0: nodes[selectedNode].color = LIGHTGRAY; break;
                    case 1: nodes[selectedNode].color = BLUE; break;
                    case 2: nodes[selectedNode].color = GREEN; break;
                    case 3: nodes[selectedNode].color = RED; break;
                }
                TraceLog(LOG_INFO, "Color selected: %d", colorIndex);
            }
        }

        // Connection management
        DrawText("Connections:", 610, 290, 12, BLACK);
        static int connectionIndex = selectedConnection;
        std::string connDropdownText = "None";
        if (!nodes[selectedNode].connections.empty()) {
            connDropdownText.clear();
            for (size_t i = 0; i < nodes[selectedNode].connections.size(); ++i) {
                connDropdownText += "To " + nodes[nodes[selectedNode].connections[i].toNodeIndex].name;
                if (i < nodes[selectedNode].connections.size() - 1) connDropdownText += ";";
            }
        }
        if (GuiDropdownBox({610, 310, 160, 20}, connDropdownText.c_str(), &connectionIndex, connDropdownEditMode)) {
            connDropdownEditMode = !connDropdownEditMode;
            if (!connDropdownEditMode) {
                selectedConnection = connectionIndex;
                if (selectedConnection >= 0 && selectedConnection < static_cast<int>(nodes[selectedNode].connections.size())) {
                    strncpy(choiceTextBuffer, nodes[selectedNode].connections[selectedConnection].choiceText.c_str(), 256);
                    TraceLog(LOG_INFO, "Connection selected: %d", selectedConnection);
                } else {
                    selectedConnection = -1;
                    choiceTextBuffer[0] = '\0';
                }
            }
        }

        // Choice text editing
        if (selectedConnection != -1 && selectedConnection < static_cast<int>(nodes[selectedNode].connections.size())) {
            DrawText("Choice Text:", 610, 340, 12, BLACK);
            if (GuiTextBox({610, 360, 160, 20}, choiceTextBuffer, 256, editChoiceTextFlag)) {
                editChoiceTextFlag = !editChoiceTextFlag;
            }
        }

        // Delete connection
        if (selectedConnection != -1 && GuiButton({610, 390, 160, 20}, "Delete Connection")) {
            nodes[selectedNode].connections.erase(nodes[selectedNode].connections.begin() + selectedConnection);
            selectedConnection = -1;
            choiceTextBuffer[0] = '\0';
            TraceLog(LOG_INFO, "Connection deleted");
        }

        // Save button
        if (GuiButton({610, 420, 160, 20}, "Save")) {
            nodes[selectedNode].name = textBuffer;
            if (selectedConnection >= 0 && selectedConnection < static_cast<int>(nodes[selectedNode].connections.size())) {
                nodes[selectedNode].connections[selectedConnection].choiceText = choiceTextBuffer;
                TraceLog(LOG_INFO, "Choice text saved: %s", choiceTextBuffer);
            }
        }

        // Delete node button
        if (GuiButton({610, 450, 160, 20}, "Delete Node")) {
            deleteNode(selectedNode);
            editingNode = false;
            selectedNode = -1;
            selectedConnection = -1;
            TraceLog(LOG_INFO, "Node deleted");
        }

        // Add node button
        if (GuiButton({610, 480, 160, 20}, "Add New Node")) {
            Vector2 mouse = GetMousePosition();
            addNode(mouse.x - offset.x, mouse.y - offset.y);
            TraceLog(LOG_INFO, "Node added at (%f, %f)", mouse.x - offset.x, mouse.y - offset.y);
        }

        // Close button
        if (GuiButton({610, 510, 20}, "Close")) {
            editingNode = false;
            selectedNode = -1;
            selectedConnection = -1;
            sceneDropdownEditMode = false;
            dragDropdownEditMode = false;
            colorDropdownEditMode = false;
            connDropdownEditMode = false;
            editChoiceTextFlag = false;
            TraceLog(LOG_INFO, "Edit UI closed");
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
    bool sceneDropdownEditMode;
    bool dragDropdownEditMode;
    bool colorDropdownEditMode;
    bool connDropdownEditMode;
    int selectedConnection;
    char choiceTextBuffer[256] = "";
    bool editChoiceTextFlag;
};

#endif // NODE_MANAGER_HPP
