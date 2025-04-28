#define _CRT_SECURE_NO_WARNINGS
#define RAYGUI_IMPLEMENTATION

#include "init.h"

#include <raygui.h>
#include <raylib.h>

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
    Element()
    {

    }
    ~Element()
    { }

    virtual void render()
    {

    }

protected:
};


class Binding : protected Element
{
public:
    Binding()
    {

    }
    ~Binding()
    {

    }

    void render() override
    {
    }

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

    ~BasicNode()
    {

    }

    void render() override
    {

        //Rectangle rect = { this->_pos.x, this->_pos.y, this->_WIDTH, 70};

       /* DrawRectangleRounded(
            { this->_pos.x, this->_pos.y, WIDTH, HEIGHT },
            0.4, 10, RED
        );*/

        DrawRectangleRoundedLines(
            { this->_pos.x + 1, this->_pos.y + 1, WIDTH - 1, HEIGHT - 1 },
            0.4, 10, BLACK
        );

        // 3. Подпись
        DrawText(this->_title.data(), this->_pos.x + 10, this->_pos.y + 5, 11, DARKGRAY);
    }

protected:
    std::string _title = "BasicNode";
    Vector2 _pos;

};


class Node : public BasicNode
{
public:
    Node(): BasicNode()
    {
        this->_title = "Node";
    }
    Node(float x, float y): BasicNode(x, y)
    {
        this->_title = "Node";
    }
    ~Node()
    {

    }

    /*void render() override
    {
    }*/

protected:
    std::string _title = "Node";
};





class Scene : protected Element
{
public:
    Scene()
    {
        this->addNode(40, 100);
        this->addNode(200, 150);
        //this->addBinding();
    }
    ~Scene()
    {

    }

    void addBinding()
    {

    }

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
        this->_nodes.push_back({x, y});
    }

    void render() override
    {
        for (Node& node : this->_nodes)
        {
            node.render();
        }
        for (Binding& binding : this->_bindings)
        {
            binding.render();
        }

    }

protected:
    std::vector<Node> _nodes;
    std::vector<Binding> _bindings;

    
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

        

        scene.render();

        
        

        EndDrawing();
    }

    CloseWindow();
    return 0;
}