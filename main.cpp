
#define _CRT_SECURE_NO_WARNINGS
#define RAYGUI_IMPLEMENTATION

#include "init.h"

#include "raylib.h"
#include "raygui.h"

#include <vector>
#include <string>
#include <cstdio>
#include <cerrno>
#include <cstring>


struct Node {
    std::string backgroundImage;
    std::string characterImage;
    Texture2D bgTexture = { 0 };
    Texture2D charTexture = { 0 };
    std::vector<std::string> textLines;
    Vector2 position;
    std::vector<int> connections;
};

std::vector<Node> nodes;
int selectedNodeIndex = -1;
bool movingNode = false;
Vector2 dragOffset = { 0, 0 };
bool editingNode = false;
char textBuffer[256] = "";
bool editText = false;
bool connectingNodes = false;
int firstSelectedNode = -1;


std::string OpenFileDialog() {
    char filename[1024] = "";
    FILE* f = OPEN("zenity --file-selection", "r");
    if (f) {
        fgets(filename, 1024, f);
        CLOSE(f);
        size_t len = strlen(filename);
        if (len > 0 && filename[len - 1] == '\n') filename[len - 1] = '\0';
    }
    return std::string(filename);
}

void LoadNodeTextures(Node& node) {
    if (!node.backgroundImage.empty()) {
        node.bgTexture = LoadTexture(node.backgroundImage.c_str());
    }
    if (!node.characterImage.empty()) {
        node.charTexture = LoadTexture(node.characterImage.c_str());
    }
}
Vector2 cameraOffset = { 0, 0 };
int main() {
    Vector2 dragStart = { 0, 0 };
    bool dragging = false;
    InitWindow(800, 600, "Novel Editor");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();

        // Handle node movement
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (size_t i = 0; i < nodes.size(); i++) {
                Rectangle nodeRect = { nodes[i].position.x + cameraOffset.x, nodes[i].position.y + cameraOffset.y, 200, 100 };
                if (CheckCollisionPointRec(mousePos, nodeRect)) {
                    if (connectingNodes) {
                        if (firstSelectedNode == -1) {
                            firstSelectedNode = i;
                        }
                        else {
                            nodes[firstSelectedNode].connections.push_back(i);
                            connectingNodes = false;
                            firstSelectedNode = -1;
                        }
                    }
                    else {
                        selectedNodeIndex = i;
                        movingNode = true;
                        editingNode = true;
                        if (!nodes[i].textLines.empty()) {
                            strcpy(textBuffer, nodes[i].textLines[0].c_str());
                        }
                        else {
                            textBuffer[0] = '\0';
                        }
                        dragOffset = { mousePos.x - nodes[i].position.x, mousePos.y - nodes[i].position.y };
                    }
                    break;
                }
            }
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && movingNode && selectedNodeIndex != -1) {
            nodes[selectedNodeIndex].position.x = mousePos.x - dragOffset.x;
            nodes[selectedNodeIndex].position.y = mousePos.y - dragOffset.y;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            movingNode = false;
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            dragging = true;
            dragStart = mousePos;
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && dragging) {
            cameraOffset.x += (mousePos.x - dragStart.x);
            cameraOffset.y += (mousePos.y - dragStart.y);
            dragStart = mousePos;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw connections
        for (auto& node : nodes) {
            for (int connIndex : node.connections) {
                // DrawLineV(node.position, nodes[connIndex].position+cameraOffset, BLACK);


                DrawLineV({ node.position.x + cameraOffset.x, node.position.y + 25 + cameraOffset.y },
                    { nodes[connIndex].position.x + cameraOffset.x, nodes[connIndex].position.y + cameraOffset.y }, BLACK);
            }
        }

        // Draw nodes
        for (auto& node : nodes) {
            DrawRectangle(node.position.x + cameraOffset.x, node.position.y + cameraOffset.y, 200, 100, LIGHTGRAY);
            DrawText("Node", node.position.x + cameraOffset.x + 10, node.position.y + cameraOffset.y + 10, 20, BLACK);

            if (node.bgTexture.id > 0) {
                DrawTexturePro(node.bgTexture, { 0, 0, (float)node.bgTexture.width, (float)node.bgTexture.height }, { node.position.x + cameraOffset.x, node.position.y + cameraOffset.y, 50, 50 }, { 0, 0 }, 0, WHITE);
            }

            if (node.charTexture.id > 0) {
                DrawTexturePro(node.charTexture, { 0, 0, (float)node.charTexture.width, (float)node.charTexture.height }, { node.position.x + 60 + cameraOffset.x, node.position.y + 20 + cameraOffset.y, 50, 50 }, { 0, 0 }, 0, WHITE);
            }
        }

        // GUI Controls
        if (GuiButton({ 10, 10, 150, 30 }, "Add Node")) {
            nodes.push_back({ "", "", {0}, {0}, {}, {400, 300}, {} });
        }

        if (GuiButton({ 10, 50, 150, 30 }, "Connect Nodes")) {
            connectingNodes = true;
            firstSelectedNode = -1;
        }

        if (editingNode && selectedNodeIndex != -1) {
            DrawRectangle(600, 50, 180, 300, LIGHTGRAY);
            DrawText("Edit Node", 610, 60, 20, BLACK);
            GuiLabel({ 610, 90, 160, 20 }, "Background Image:");
            if (GuiButton({ 610, 110, 160, 30 }, "Select File")) {
                nodes[selectedNodeIndex].backgroundImage = OpenFileDialog();
                LoadNodeTextures(nodes[selectedNodeIndex]);
            }
            GuiLabel({ 610, 140, 160, 20 }, "Character Image:");
            if (GuiButton({ 610, 160, 160, 30 }, "Select File")) {
                nodes[selectedNodeIndex].characterImage = OpenFileDialog();
                LoadNodeTextures(nodes[selectedNodeIndex]);
            }
            GuiLabel({ 610, 190, 160, 20 }, "Text:");
            if (GuiTextBox({ 610, 210, 160, 20 }, textBuffer, 256, editText)) {
                editText = !editText;
            }
            if (GuiButton({ 610, 250, 160, 30 }, "Save")) {
                nodes[selectedNodeIndex].textLines.clear();
                nodes[selectedNodeIndex].textLines.push_back(textBuffer);
            }
            if (GuiButton({ 610, 290, 160, 30 }, "Close")) {
                editingNode = false;
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
