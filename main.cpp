#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <vector>
#include <string>

struct Shape {
    Rectangle rect;
    bool isCircle;
};

struct Connection {
    int startIndex;
    int endIndex;
};

int main() {
    InitWindow(800, 600, "Shape Editor");
    SetTargetFPS(60);
    
    std::vector<Shape> shapes;
    std::vector<Connection> connections;
    Vector2 cameraOffset = {0, 0};
    bool dragging = false;
    Vector2 dragStart = {0, 0};
    bool movingShape = false;
    int selectedShapeIndex = -1;
    int shapeToConnectIndex = -1;
    std::string connectionMessage = "Select first shape to connect";
    
    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();
        
        // Pan scene with right mouse drag
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            dragging = true;
            dragStart = mousePos;
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && dragging) {
            cameraOffset.x += (mousePos.x - dragStart.x);
            cameraOffset.y += (mousePos.y - dragStart.y);
            dragStart = mousePos;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) dragging = false;
        
        // Move shapes with left mouse button
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (size_t i = 0; i < shapes.size(); i++) {
                Rectangle adjustedRect = {shapes[i].rect.x + cameraOffset.x, shapes[i].rect.y + cameraOffset.y, shapes[i].rect.width, shapes[i].rect.height};
                if (CheckCollisionPointRec(mousePos, adjustedRect)) {
                    selectedShapeIndex = i;
                    movingShape = true;
                    break;
                }
            }
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && movingShape && selectedShapeIndex != -1) {
            shapes[selectedShapeIndex].rect.x = mousePos.x - cameraOffset.x - shapes[selectedShapeIndex].rect.width / 2;
            shapes[selectedShapeIndex].rect.y = mousePos.y - cameraOffset.y - shapes[selectedShapeIndex].rect.height / 2;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            movingShape = false;
        }
        
        // Add shapes via GUI buttons
        if (GuiButton({10, 10, 100, 30}, "Add Square")) {
            shapes.push_back({{400 - cameraOffset.x, 300 - cameraOffset.y, 50, 50}, false});
        }
        if (GuiButton({120, 10, 100, 30}, "Add Circle")) {
            shapes.push_back({{400 - cameraOffset.x, 300 - cameraOffset.y, 50, 50}, true});
        }
        
        // Connect shapes via GUI button
        if (GuiButton({230, 10, 100, 30}, "Connect")) {
            if (shapeToConnectIndex != -1 && selectedShapeIndex != -1 && shapeToConnectIndex != selectedShapeIndex) {
                connections.push_back({shapeToConnectIndex, selectedShapeIndex});
                shapeToConnectIndex = -1;
                connectionMessage = "Connection created!";
            } else {
                shapeToConnectIndex = selectedShapeIndex;
                connectionMessage = shapeToConnectIndex != -1 ? "Select second shape to connect" : "Select first shape to connect";
            }
        }
        
        // Draw scene
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        for (auto &conn : connections) {
            DrawLineV({shapes[conn.startIndex].rect.x + 25 + cameraOffset.x, shapes[conn.startIndex].rect.y + 25 + cameraOffset.y},
                      {shapes[conn.endIndex].rect.x + 25 + cameraOffset.x, shapes[conn.endIndex].rect.y + 25 + cameraOffset.y}, BLACK);
        }
        
        for (auto &shape : shapes) {
            if (shape.isCircle) {
                DrawCircleV({shape.rect.x + 25 + cameraOffset.x, shape.rect.y + 25 + cameraOffset.y}, 25, BLUE);
            } else {
                DrawRectangle(shape.rect.x + cameraOffset.x, shape.rect.y + cameraOffset.y, shape.rect.width, shape.rect.height, RED);
            }
        }
        
        // Display connection status message
        DrawText(connectionMessage.c_str(), 10, 50, 20, DARKGRAY);
        
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
