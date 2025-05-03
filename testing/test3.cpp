#define _CRT_SECURE_NO_WARNINGS
#define RAYGUI_IMPLEMENTATION

#include "init.h"
#include <vector>
#include <string>
#include <memory>
#include <cmath>

class BasicNode;
// Utility function (same as in your snippet)
std::string OpenFileDialog() {
    char filename[1024] = "";
    FILE *f = popen("zenity --file-selection", "r");
    if (f) {
        fgets(filename, 1024, f);
        pclose(f);
        size_t len = strlen(filename);
        if (len > 0 && filename[len - 1] == '\n') filename[len - 1] = '\0';
    }
    return std::string(filename);
}



// Interface for renderable elements
class Renderable {
public:
    virtual void render(Vector2 offset) = 0;
    virtual ~Renderable() = default;
};

class Editable {
    public:
        virtual void editUI(BasicNode* node, int id, bool& editing, int& selectedId, char* buffer, bool& editTextFlag) = 0;
        virtual ~Editable() = default;
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

    void setEditable(std::unique_ptr<Editable> editable) {
        _editable = std::move(editable);
    }

    Editable* getEditable() {
        return _editable.get();
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
    std::unique_ptr<Editable> _editable;
};

// Derived node class
// class Node : public BasicNode {
// public:
//     Node(float x, float y) : BasicNode(x, y, "Node") {}
// };
class Node : public BasicNode {
    public:
        Node(float x, float y) : BasicNode(x, y, "Node") {}
    
        std::string backgroundImage;
        std::string characterImage;
        Texture2D bgTexture = {0};
        Texture2D charTexture = {0};
        std::vector<std::string> textLines;
    
        void loadTextures() {
            if (!backgroundImage.empty()) {
                bgTexture = LoadTexture(backgroundImage.c_str());
            }
            if (!characterImage.empty()) {
                charTexture = LoadTexture(characterImage.c_str());
            }
        }
    };

// A concrete Editable type
    class ImageTextEditable : public Editable {
        public:
            void editUI(BasicNode* base, int id, bool& editing, int& selectedId, char* buffer, bool& editTextFlag) override {                Node* node = dynamic_cast<Node*>(base);
                if (!node) return;
        
                DrawRectangle(600, 50, 180, 300, LIGHTGRAY);
                DrawText("Edit Node", 610, 60, 20, BLACK);
                GuiLabel({610, 90, 160, 20}, "Background Image:");
                if (GuiButton({610, 110, 160, 30}, "Select File")) {
                    node->backgroundImage = OpenFileDialog();
                    node->loadTextures();
                }
                GuiLabel({610, 140, 160, 20}, "Character Image:");
                if (GuiButton({610, 160, 160, 30}, "Select File")) {
                    node->characterImage = OpenFileDialog();
                    node->loadTextures();
                }
                GuiLabel({610, 190, 160, 20}, "Text:");
                if (GuiTextBox({610, 210, 160, 20}, buffer, 256, editTextFlag)) {
                    editTextFlag = !editTextFlag;
                }
                if (GuiButton({610, 250, 160, 30}, "Save")) {
                    node->textLines.clear();
                    node->textLines.push_back(buffer);
                }
                if (GuiButton({610, 290, 160, 30}, "Close")) {
                    editing = false;
                    selectedId = -1;
                }
            }
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
            addNode(100, 150, nullptr, std::make_unique<ImageTextEditable>());
            addNode(400, 300, nullptr, std::make_unique<ImageTextEditable>());
            addNode(100, 100, nullptr, std::make_unique<ImageTextEditable>());
            addNode(200, 200, std::make_unique<SnappingDrag>(), std::make_unique<ImageTextEditable>());
            addNode(100, 150, nullptr, std::make_unique<ImageTextEditable>());


        _connections.push_back(std::make_unique<Connection>(_nodes[0].get(), 0, _nodes[1].get(), 0));
    }

    // void addNode(float x, float y, std::unique_ptr<Draggable> draggable = nullptr) {
    //     auto node = std::make_unique<Node>(x, y);
    //     if (draggable) {
    //         node->setDraggable(std::move(draggable));
    //     }
    //     _nodes.push_back(std::move(node));
    // }
    void addNode(float x, float y, std::unique_ptr<Draggable> draggable = nullptr, std::unique_ptr<Editable> editable = nullptr) {
        auto node = std::make_unique<Node>(x, y);
        if (draggable) node->setDraggable(std::move(draggable));
        if (editable) node->setEditable(std::move(editable));
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

        for (size_t i = 0; i < _nodes.size(); ++i) {
            if (_nodes[i]->isMouseOver(_offset) && IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON)) {
                _editingNode = true;
                _selectedNodeIndex = static_cast<int>(i);
                Node* node = dynamic_cast<Node*>(_nodes[i].get());
                if (node && !node->textLines.empty()) {
                    strncpy(_textBuffer, node->textLines[0].c_str(), 256);
                } else {
                    _textBuffer[0] = '\0';
                }
            }
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

        if (_editingNode && _selectedNodeIndex != -1) {
            BasicNode* node = _nodes[_selectedNodeIndex].get();
            if (node->getEditable()) {
                node->getEditable()->editUI(node, _selectedNodeIndex, _editingNode, _selectedNodeIndex, _textBuffer, _editText);
            }
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

    bool _editingNode = false;
    int _selectedNodeIndex = -1;
    char _textBuffer[256] = "";
    bool _editText = false;

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
