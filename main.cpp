#define _CRT_SECURE_NO_WARNINGS
#define RAYGUI_IMPLEMENTATION

#include "init.h"
#include <vector>
#include <string>

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

class BasicNode : protected Element
{
public:
    BasicNode()
    {
        this->_pos = { 0, 0 };
    }

    BasicNode(float x, float y)
    {
        this->_pos = { x, y };
    }

    ~BasicNode() { }

    // Default render (without offset, won't be used anymore)
    void render() override { }

    // Render node with scene offset applied
    void renderWithOffset(Vector2 offset)
    {
        Rectangle rect = { _pos.x + offset.x + 1, _pos.y + offset.y + 1, WIDTH - 1, HEIGHT - 1 };

        DrawRectangleRoundedLines(
            rect,
            0.4f, 10, BLACK
        );

        DrawText(_title.c_str(), rect.x + 10, rect.y + 5, 11, DARKGRAY);
    }

    // Check if mouse is over this node (taking scene offset into account)
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

        addNode(40, 100);
        addNode(200, 150);
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
        Vector2 mouseDelta = GetMouseDelta();

        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
        {
            for (Node& node : _nodes)
            {
                if (node.isMouseOver(_offset))
                {
                    _draggingNode = &node;
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
            _draggingScene = true;
        }
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
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
        for (Node& node : this->_nodes)
        {
            node.renderWithOffset(_offset);
        }
        for (Binding& binding : this->_bindings)
        {
            binding.render();
        }
    }

protected:
    std::vector<Node> _nodes;
    std::vector<Binding> _bindings;

    Vector2 _offset;
    Node* _draggingNode;
    bool _draggingScene;
};

int main()
{
    InitWindow(800, 600, "Novel Editor");
    SetTargetFPS(60);

    Scene scene;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        scene.update(); // Update input (dragging)
        scene.render(); // Render everything

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
