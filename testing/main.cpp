#define _CRT_SECURE_NO_WARNINGS
#define RAYGUI_IMPLEMENTATION

#include "init.h"
#include <vector>
#include <string>
#include <cmath>

struct Connection
{
    int fromNode;
    int fromSlot;
    int toNode;
    int toSlot;
};

struct Point2D
{
    int x;
    int y;
};

class Element
{
public:
    virtual void render() { }
    virtual void update() { }
protected:
};

#define WIDTH 100
#define HEIGHT 40
#define SLOT_RADIUS 6

class BasicNode : protected Element
{
public:
    BasicNode()
    {
        this->_pos = { 0, 0 };
        setupSlots();
    }

    BasicNode(float x, float y)
    {
        this->_pos = { x, y };
        setupSlots();
    }

    ~BasicNode() { }

    void setupSlots()
    {
        // Just one input and one output for now
        _inputs.clear();
        _outputs.clear();
        _inputs.push_back({ 0, HEIGHT / 2 });
        _outputs.push_back({ WIDTH, HEIGHT / 2 });
    }

    Vector2 getInputSlotWorldPos(int index, Vector2 offset)
    {
        return { _pos.x + _inputs[index].x + offset.x, _pos.y + _inputs[index].y + offset.y };
    }

    Vector2 getOutputSlotWorldPos(int index, Vector2 offset)
    {
        return { _pos.x + _outputs[index].x + offset.x, _pos.y + _outputs[index].y + offset.y };
    }

    int inputCount() { return _inputs.size(); }
    int outputCount() { return _outputs.size(); }

    void renderWithOffset(Vector2 offset)
    {
        Rectangle rect = { _pos.x + offset.x + 1, _pos.y + offset.y + 1, WIDTH - 1, HEIGHT - 1 };

        DrawRectangleRoundedLines(rect, 0.4f, 10, BLACK);
        DrawText(_title.c_str(), rect.x + 10, rect.y + 5, 11, DARKGRAY);

        // Draw input slots
        for (auto& input : _inputs)
        {
            Vector2 p = { _pos.x + input.x + offset.x, _pos.y + input.y + offset.y };
            DrawCircleV(p, SLOT_RADIUS, BLUE);
        }

        // Draw output slots
        for (auto& output : _outputs)
        {
            Vector2 p = { _pos.x + output.x + offset.x, _pos.y + output.y + offset.y };
            DrawCircleV(p, SLOT_RADIUS, RED);
        }
    }

    bool isMouseOver(Vector2 offset)
    {
        Vector2 mouse = GetMousePosition();
        float mx = mouse.x - offset.x;
        float my = mouse.y - offset.y;
        return (mx >= _pos.x && mx <= _pos.x + WIDTH &&
                my >= _pos.y && my <= _pos.y + HEIGHT);
    }

    void move(Vector2 delta)
    {
        _pos.x += delta.x;
        _pos.y += delta.y;
    }

protected:
    std::string _title = "BasicNode";
    Vector2 _pos;
    std::vector<Vector2> _inputs;
    std::vector<Vector2> _outputs;
};

class Node : public BasicNode
{
public:
    Node() : BasicNode()
    {
        this->_title = "Node";
    }
    Node(float x, float y) : BasicNode(x, y)
    {
        this->_title = "Node";
    }
    ~Node() { }
protected:
    std::string _title = "Node";
};

class Binding : protected Element
{
public:
    Binding() { }
    ~Binding() { }
    void render() override { }
protected:
};

class Scene : protected Element
{
public:
    Scene()
    {
        this->_offset = { 0, 0 };
        this->_draggingNode = nullptr;
        this->_draggingScene = false;

        addNode(100, 150);
        addNode(400, 300);
        addNode(100, 100);
        addNode(200, 200);

        // Connect node 0 output to node 1 input
        _connections.push_back({ 0, 0, 1, 0 });
    }
    ~Scene() { }

    void addBinding() { }

    void addNode()
    {
        this->_nodes.push_back({});
    }

    void addNode(Node& node)
    {
        this->_nodes.push_back(node);
    }

    void addNode(float x, float y)
    {
        this->_nodes.push_back({ x, y });
    }

    void update() override
    {
        Vector2 mouse = GetMousePosition();
        Vector2 mouseDelta = GetMouseDelta();
    
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
        {
            for (int i = 0; i < _nodes.size(); i++)
            {
                if (_nodes[i].isMouseOver(_offset))
                {
                    _draggingNode = &_nodes[i];
                    break;
                }
            }
        }
        if (IsMouseButtonReleased(MOUSE_RIGHT_BUTTON))
        {
            _draggingNode = nullptr;
        }
    
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            bool clickedSlot = false;
    
            for (int i = 0; i < _nodes.size(); i++)
            {
                Node& node = _nodes[i];
                for (int j = 0; j < node.outputCount(); j++)
                {
                    Vector2 p = node.getOutputSlotWorldPos(j, _offset);
                    if (CheckCollisionPointCircle(mouse, p, SLOT_RADIUS))
                    {
                        _creatingConnection = true;
                        _fromNode = i;
                        _fromSlot = j;
                        clickedSlot = true;
                        break;
                    }
                }
            }
    
            if (!clickedSlot)
            {
                _draggingScene = true;
            }
        }
    
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            if (_creatingConnection)
            {
                // Check if released over an input slot
                for (int i = 0; i < _nodes.size(); i++)
                {
                    Node& node = _nodes[i];
                    for (int j = 0; j < node.inputCount(); j++)
                    {
                        Vector2 p = node.getInputSlotWorldPos(j, _offset);
                        if (CheckCollisionPointCircle(mouse, p, SLOT_RADIUS))
                        {
                            _connections.push_back({ _fromNode, _fromSlot, i, j });
                            break;
                        }
                    }
                }
            }
    
            _creatingConnection = false;
            _draggingScene = false;
        }
    
        if (_draggingNode)
        {
            _draggingNode->move(mouseDelta);
        }
        else if (_draggingScene)
        {
            _offset.x += mouseDelta.x;
            _offset.y += mouseDelta.y;
        }
    }
    

    void render() override
    {
        // 1. Draw connections first
        for (Connection& conn : _connections)
        {
            Vector2 start = _nodes[conn.fromNode].getOutputSlotWorldPos(conn.fromSlot, _offset);
            Vector2 end = _nodes[conn.toNode].getInputSlotWorldPos(conn.toSlot, _offset);

            DrawLineBezier(start, end, 2.0f, DARKGRAY);

            // Draw small arrowhead at the end
            Vector2 dir = Vector2Normalize(Vector2Subtract(end, start));
            Vector2 left = { end.x - dir.x * 10.0f + dir.y * 5.0f, end.y - dir.y * 10.0f - dir.x * 5.0f };
            Vector2 right = { end.x - dir.x * 10.0f - dir.y * 5.0f, end.y - dir.y * 10.0f + dir.x * 5.0f };
            DrawTriangle(end, left, right, DARKGRAY);
        }

        // 2. Then draw nodes
        for (Node& node : this->_nodes)
        {
            node.renderWithOffset(_offset);
        }
        if (_creatingConnection)
{
    Vector2 start = _nodes[_fromNode].getOutputSlotWorldPos(_fromSlot, _offset);
    Vector2 end = GetMousePosition();
    DrawLineBezier(start, end, 2.0f, RED);
}

        // (bindings rendering is empty for now)
    }

protected:
    std::vector<Node> _nodes;
    std::vector<Binding> _bindings;
    std::vector<Connection> _connections;

    Vector2 _offset;
    Node* _draggingNode;
    bool _draggingScene;

    bool _creatingConnection = false;
    int _fromNode = -1;
    int _fromSlot = -1;

};

int main()
{
    InitWindow(800, 600, "Novel Editor - Nodes with Connections");
    SetTargetFPS(60);

    Scene scene;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        scene.update();
        scene.render();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
