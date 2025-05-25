#ifndef NODE_MANAGER_HPP
#define NODE_MANAGER_HPP

#include "Types.hpp"
#include "raylib.h"
#include "raygui.h"
#include <vector>
#include <memory>
#include <string>
#include <sstream>

class Draggable
{
public:
    virtual bool isMouseOver(Vector2 pos, Vector2 size, Vector2 offset) const = 0;
    virtual void move(Vector2& target, Vector2 delta) = 0;
    virtual ~Draggable() = default;
};

class SimpleDrag : public Draggable
{
public:
    bool isMouseOver(Vector2 pos, Vector2 size, Vector2 offset) const override
    {
        Vector2 mouse = GetMousePosition();
        float mx = mouse.x - offset.x;
        float my = mouse.y - offset.y;
        return (mx >= pos.x && mx <= pos.x + size.x && my >= pos.y && my <= pos.y + size.y);
    }

    void move(Vector2& target, Vector2 delta) override
    {
        target.x += delta.x;
        target.y += delta.y;
    }
};

class SnappingDrag : public Draggable
{
public:
    bool isMouseOver(Vector2 pos, Vector2 size, Vector2 offset) const override
    {
        Vector2 mouse = GetMousePosition();
        float mx = mouse.x - offset.x;
        float my = mouse.y - offset.y;
        return (mx >= pos.x && mx <= pos.x + size.x && my >= pos.y && my <= pos.y + size.y);
    }

    void move(Vector2& target, Vector2 delta) override
    {
        target.x += delta.x;
        target.y += delta.y;
        target.x = std::round(target.x / 50.0f) * 50.0f;
        target.y = std::round(target.y / 50.0f) * 50.0f;
    }
};

class NodeManager : BasicUI
{
public:
    NodeManager(std::vector<Scene>& scenes);

    void update();

    void draw();

    void addNode(float x, float y);

    void deleteNode(size_t index);

    std::vector<Node>& getNodes() { return nodes; }

    enum class ConnectionRenderMode {
            SINGLE_POINT,
            MULTI_POINT
        };

private:
    bool isMouseOverNode(size_t index);
    bool isMouseOverNodeInput(size_t index);
    bool isMouseOverNodeOutput(size_t index);
    bool isMouseOverAnyNode();
    Vector2 getNodeInputPos(size_t index);
    Vector2 getNodeInputPos(size_t index, size_t connectionIndex);
    Vector2 getNodeOutputPos(size_t index, size_t connectionIndex);

    void drawEditUI();

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
    bool isEditingChoiceText;
    ConnectionRenderMode connectionRenderMode;

    size_t getIncomingConnectionCount(size_t nodeIndex);
    size_t getIncomingConnectionIndex(size_t targetNodeIndex, size_t sourceNodeIndex, size_t sourceConnIndex);
    float getNodeHeight(size_t index); // Add this line
};

#endif // NODE_MANAGER_HPP
