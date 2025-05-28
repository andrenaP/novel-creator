#ifndef TYPES_HPP
#define TYPES_HPP

#include "raylib.h"
#include "json.hpp"

#include "ui/BasicUI.hpp"

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
    int positionIndex; // positionIndex

    CharacterElement() : positionIndex(0) {} // Initialize positionIndex to 0
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
    int renderlevel;
    int positionIndex; // Added for CharacterElement position2
    std::string selectedPose;

    SceneElement() : elementIndex(0), startTime(0.0f), endTime(1.0f), renderlevel(0), positionIndex(0), selectedPose("") {}
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
    int sceneIndex;
    std::vector<NodeConnection> connections;
    Vector2 position;
    DragType dragType;
    Color color;
    bool isStartNode;

    Node(
        const std::string& n = "Node",
        int s = -1,
        const std::vector<NodeConnection>& c = {},
        Vector2 p = {0, 0},
        DragType dt = DragType::SIMPLE,
        Color col = LIGHTGRAY,
        bool start = false)
    :
        name(n),
        sceneIndex(s),
        connections(c),
        position(p),
        dragType(dt),
        color(col),
        isStartNode(start)
    {}
};

#endif // TYPES_HPP
