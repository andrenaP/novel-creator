#ifndef TYPES_HPP
#define TYPES_HPP

#include "raylib.h"
#include "json.hpp"

#include "BasicUI.hpp"

#include <vector>
#include <string>
#include <variant>

using json = nlohmann::json;

struct TextElement
{
    std::string content;
};

struct CharacterElement 
{
    std::string name;
    std::vector<std::pair<std::string, std::string>> images;
    std::vector<Texture2D> textures;

    CharacterElement() = default;
    ~CharacterElement() 
    {
        for (auto& texture : textures) 
        {
            if (texture.id > 0) 
                UnloadTexture(texture);
        }
    }
};

struct BackgroundElement 
{
    std::string imagePath;
    Texture2D texture = {0};

    ~BackgroundElement() 
    {
        if (texture.id > 0) UnloadTexture(texture);
    }
};

enum class ElementType { TEXT, CHARACTER, BACKGROUND };

struct Element 
{
    ElementType type;
    std::string name;
    std::variant<TextElement, CharacterElement, BackgroundElement> data;

    Element() : 
        type(ElementType::TEXT), 
        name("New Element"), 
        data(TextElement{""}) 
    {}
    ~Element() = default;
};

struct SceneElement 
{
    size_t elementIndex;
    float startTime;
    float endTime;
};

struct Scene 
{
    std::string name;
    std::vector<SceneElement> elements;
};

struct NodeConnection 
{
    size_t toNodeIndex;
    std::string choiceText;
};

enum class DragType { SIMPLE, SNAPPING };

struct Node
{
    std::string name;
    int sceneIndex; // Single scene index (default -1 for none)
    std::vector<NodeConnection> connections;
    Vector2 position;
    DragType dragType;
    Color color; // Changed from NodeColor to Raylib Color

    Node():
        name("Node"), 
        sceneIndex(-1), 
        connections({}), 
        position({0, 0}), 
        dragType(DragType::SIMPLE), 
        color(LIGHTGRAY) 
    {}

    Node(
        const std::string& n = "Node", 
        int s = -1, 
        const std::vector<NodeConnection>& c = {}, 
        Vector2 p = {0, 0}, 
        DragType dt = DragType::SIMPLE, 
        Color col = LIGHTGRAY)
    : 
        name(n), 
        sceneIndex(s), 
        connections(c), 
        position(p), 
        dragType(dt), 
        color(col) 
    {}
};

#endif // TYPES_HPP
