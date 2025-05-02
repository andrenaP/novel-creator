#define _CRT_SECURE_NO_WARNINGS
#define RAYGUI_IMPLEMENTATION

#include "init.h"
#include <vector>
#include <string>
#include <memory>
#include <cmath>

// Interface for renderable elements
class Renderable {
public:
    virtual void render(Vector2 offset) = 0;
    virtual ~Renderable() = default;
};

// Interface for updatable elements
class Updatable {
public:
    virtual void update() = 0;
    virtual ~Updatable() = default;
};

// Interface for draggable elements
class Draggable {
public:
    virtual bool isMouseOver(Vector2 pos, Vector2 size, Vector2 offset) const = 0;
    virtual void move(Vector2 pos, Vector2& target, Vector2 delta) = 0;
    virtual ~Draggable() = default;
};

// Default dragging behavior
class SimpleDrag : public Draggable {
public:
    bool isMouseOver(Vector2 pos, Vector2 size, Vector2 offset) const override {
        Vector2 mouse = GetMousePosition();
        float mx = mouse.x - offset.x;
        float my = mouse.y - offset.y;
        return (mx >= pos.x && mx <= pos.x + size.x && my >= pos.y && my <= pos.y + size.y);
    }

    void move(Vector2 pos, Vector2& target, Vector2 delta) override {
        target.x += delta.x;
        target.y += delta.y;
    }
};

// Constants
#define WIDTH 100
#define HEIGHT 40
#define SLOT_RADIUS 6

// Base class for nodes
class BasicNode : public Renderable {
public:
    BasicNode(float x, float y, const std::string& title = "BasicNode")
        : _pos{x, y}, _size{WIDTH, HEIGHT}, _title(title), _draggable(std::make_unique<SimpleDrag>()) {
        setupSlots();
    }

    virtual ~BasicNode() = default;

    void setDraggable(std::unique_ptr<Draggable> draggable) {
        _draggable = std::move(draggable);
    }

    bool isMouseOver(Vector2 offset) const {
        return _draggable->isMouseOver(_pos, _size, offset);
    }

    void move(Vector2 delta) {
        _draggable->move(_pos, _pos, delta);
    }

    void setupSlots() {
        _inputs.clear();
        _outputs.clear();
        _inputs.push_back({0, HEIGHT / 2});
        _outputs.push_back({WIDTH, HEIGHT / 2});
    }

    Vector2 getInputSlotWorldPos(int index, Vector2 offset) const {
        return {_pos.x + _inputs[index].x + offset.x, _pos.y + _inputs[index].y + offset.y};
    }

    Vector2 getOutputSlotWorldPos(int index, Vector2 offset) const {
        return {_pos.x + _outputs[index].x + offset.x, _pos.y + _outputs[index].y + offset.y};
    }

    int inputCount() const { return _inputs.size(); }
    int outputCount() const { return _outputs.size(); }

    void render(Vector2 offset) override {
        Rectangle rect = {_pos.x + offset.x + 1, _pos.y + offset.y + 1, _size.x - 1, _size.y - 1};
        DrawRectangleRoundedLines(rect, 0.4f, 10, BLACK);
        DrawText(_title.c_str(), rect.x + 10, rect.y + 5, 11, DARKGRAY);

        for (const auto& input : _inputs) {
            Vector2 p = {_pos.x + input.x + offset.x, _pos.y + input.y + offset.y};
            DrawCircleV(p, SLOT_RADIUS, BLUE);
        }

        for (const auto& output : _outputs) {
            Vector2 p = {_pos.x + output.x + offset.x, _pos.y + output.y + offset.y};
            DrawCircleV(p, SLOT_RADIUS, RED);
        }
    }

protected:
    Vector2 _pos;
    Vector2 _size;
    std::string _title;
    std::vector<Vector2> _inputs;
    std::vector<Vector2> _outputs;
    std::unique_ptr<Draggable> _draggable;
};

// Derived node class
class Node : public BasicNode {
public:
    Node(float x, float y) : BasicNode(x, y, "Node") {}
};

// Custom dragging behavior (snaps to grid)
class SnappingDrag : public Draggable {
public:
    bool isMouseOver(Vector2 pos, Vector2 size, Vector2 offset) const override {
        Vector2 mouse = GetMousePosition();
        float mx = mouse.x - offset.x;
        float my = mouse.y - offset.y;
        return (mx >= pos.x && mx <= pos.x + size.x && my >= pos.y && my <= pos.y + size.y);
    }

    void move(Vector2 pos, Vector2& target, Vector2 delta) override {
        target.x += delta.x;
        target.y += delta.y;
        target.x = std::round(target.x / 50.0f) * 50.0f;
        target.y = std::round(target.y / 50.0f) * 50.0f;
    }
};

// Connection class
class Connection : public Renderable {
public:
    Connection(BasicNode* fromNode, int fromSlot, BasicNode* toNode, int toSlot)
        : _fromNode(fromNode), _fromSlot(fromSlot), _toNode(toNode), _toSlot(toSlot) {}

    void render(Vector2 offset) override {
        Vector2 start = _fromNode->getOutputSlotWorldPos(_fromSlot, offset);
        Vector2 end = _toNode->getInputSlotWorldPos(_toSlot, offset);
        DrawLineBezier(start, end, 2.0f, DARKGRAY);

        Vector2 dir = Vector2Normalize(Vector2Subtract(end, start));
        Vector2 left = {end.x - dir.x * 10.0f + dir.y * 5.0f, end.y - dir.y * 10.0f - dir.x * 5.0f};
        Vector2 right = {end.x - dir.x * 10.0f - dir.y * 5.0f, end.y - dir.y * 10.0f + dir.x * 5.0f};
        DrawTriangle(end, left, right, DARKGRAY);
    }

private:
    BasicNode* _fromNode;
    int _fromSlot;
    BasicNode* _toNode;
    int _toSlot;
};

// Scene class
class Scene : public Renderable, public Updatable {
public:
    Scene()
        : _offset{0, 0}, _draggingNode(nullptr), _draggingScene(false), _creatingConnection(false) {
        addNode(100, 150);
        addNode(400, 300);
        addNode(100, 100);
        addNode(200, 200, std::make_unique<SnappingDrag>());

        _connections.push_back(std::make_unique<Connection>(_nodes[0].get(), 0, _nodes[1].get(), 0));
    }

    void addNode(float x, float y, std::unique_ptr<Draggable> draggable = nullptr) {
        auto node = std::make_unique<Node>(x, y);
        if (draggable) {
            node->setDraggable(std::move(draggable));
        }
        _nodes.push_back(std::move(node));
    }

    void update() override {
        Vector2 mouse = GetMousePosition();
        Vector2 mouseDelta = GetMouseDelta();

        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            _draggingNode = nullptr;
            for (auto& node : _nodes) {
                if (node->isMouseOver(_offset)) {
                    _draggingNode = node.get();
                    break;
                }
            }
        }

        if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON)) {
            _draggingNode = nullptr;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            bool clickedSlot = false;
            for (size_t i = 0; i < _nodes.size(); ++i) {
                auto& node = _nodes[i];
                for (int j = 0; j < node->outputCount(); ++j) {
                    Vector2 p = node->getOutputSlotWorldPos(j, _offset);
                    if (CheckCollisionPointCircle(mouse, p, SLOT_RADIUS)) {
                        _creatingConnection = true;
                        _fromNode = i;
                        _fromSlot = j;
                        clickedSlot = true;
                        break;
                    }
                }
            }
            if (!clickedSlot) {
                _draggingScene = true;
            }
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            if (_creatingConnection) {
                for (size_t i = 0; i < _nodes.size(); ++i) {
                    auto& node = _nodes[i];
                    for (int j = 0; j < node->inputCount(); ++j) {
                        Vector2 p = node->getInputSlotWorldPos(j, _offset);
                        if (CheckCollisionPointCircle(mouse, p, SLOT_RADIUS)) {
                            _connections.push_back(std::make_unique<Connection>(_nodes[_fromNode].get(), _fromSlot, node.get(), j));
                            break;
                        }
                    }
                }
            }
            _creatingConnection = false;
            _draggingScene = false;
        }

        if (_draggingNode) {
            _draggingNode->move(mouseDelta);
        } else if (_draggingScene) {
            _offset.x += mouseDelta.x;
            _offset.y += mouseDelta.y;
        }
    }

    void render(Vector2 offset) override {
        for (const auto& conn : _connections) {
            conn->render(_offset);
        }

        for (auto& node : _nodes) {
            node->render(_offset);
        }

        if (_creatingConnection) {
            Vector2 start = _nodes[_fromNode]->getOutputSlotWorldPos(_fromSlot, _offset);
            Vector2 end = GetMousePosition();
            DrawLineBezier(start, end, 2.0f, RED);
        }
    }

private:
    std::vector<std::unique_ptr<BasicNode>> _nodes;
    std::vector<std::unique_ptr<Connection>> _connections;
    Vector2 _offset;
    BasicNode* _draggingNode;
    bool _draggingScene;
    bool _creatingConnection;
    size_t _fromNode;
    int _fromSlot;
};

// Entry point
int main() {
    InitWindow(800, 600, "Novel Editor - Nodes with Connections");
    SetTargetFPS(60);

    Scene scene;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        scene.update();
        scene.render({0, 0});

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
