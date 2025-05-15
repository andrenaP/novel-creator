#ifndef JSON_UTILS_HPP
#define JSON_UTILS_HPP

#include "Types.hpp"
#include <fstream>
#include <stdexcept>

namespace JsonUtils
{
    // Helper to convert Color to JSON
    json colorToJson(const Color& color)
    {
        json j;
        j["r"] = color.r;
        j["g"] = color.g;
        j["b"] = color.b;
        j["a"] = color.a;
        return j;
    }

    // Helper to convert JSON to Color
    Color jsonToColor(const json& j)
    {
        return CLITERAL(Color){
            static_cast<unsigned char>(j.value("r", 200)),
            static_cast<unsigned char>(j.value("g", 200)),
            static_cast<unsigned char>(j.value("b", 200)),
            static_cast<unsigned char>(j.value("a", 255))
        };
    }

    // Export Element to JSON
    json elementToJson(const Element& element)
    {
        json j;
        j["type"] = static_cast<int>(element.type);
        j["name"] = element.name;

        switch (element.type)
        {
            case ElementType::TEXT:
            {
                auto& text = std::get<TextElement>(element.data);
                j["data"]["content"] = text.content;
                break;
            }
            case ElementType::CHARACTER:
            {
                auto& character = std::get<CharacterElement>(element.data);
                j["data"]["name"] = character.name;
                j["data"]["images"] = json::array();
                for (const auto& [pose, path] : character.images)
                {
                    json img;
                    img["pose"] = pose;
                    img["path"] = path;
                    j["data"]["images"].push_back(img);
                }
                break;
            }
            case ElementType::BACKGROUND:
            {
                auto& background = std::get<BackgroundElement>(element.data);
                j["data"]["imagePath"] = background.imagePath;
                break;
            }
        }
        return j;
    }

    // Import Element from JSON
    Element jsonToElement(const json& j)
    {
        Element element;
        element.type = static_cast<ElementType>(j.value("type", 0));
        element.name = j.value("name", "New Element");

        if (j.contains("data"))
        {
            switch (element.type)
            {
                case ElementType::TEXT:
                {
                    TextElement text;
                    text.content = j["data"].value("content", "");
                    element.data = text;
                    break;
                }
                case ElementType::CHARACTER:
                {
                    CharacterElement character;
                    character.name = j["data"].value("name", "");
                    if (j["data"].contains("images"))
                    {
                        for (const auto& img : j["data"]["images"])
                        {
                            character.images.emplace_back(
                                img.value("pose", ""),
                                img.value("path", "")
                            );
                        }
                    }
                    element.data = character;
                    break;
                }
                case ElementType::BACKGROUND:
                {
                    BackgroundElement background;
                    background.imagePath = j["data"].value("imagePath", "");
                    element.data = background;
                    break;
                }
            }
        }
        else
        {
            element.data = TextElement{""};
        }
        return element;
    }

    // Export Scene to JSON
    json sceneToJson(const Scene& scene)
    {
        json j;
        j["name"] = scene.name;
        j["elements"] = json::array();
        for (const auto& sceneElement : scene.elements)
        {
            json se;
            se["elementIndex"] = sceneElement.elementIndex;
            se["startTime"] = sceneElement.startTime;
            se["endTime"] = sceneElement.endTime;
            se["renderlevel"] = sceneElement.renderlevel;
            se["selectedPose"] = sceneElement.selectedPose;
            j["elements"].push_back(se);
        }
        return j;
    }

    // Import Scene from JSON
    Scene jsonToScene(const json& j)
    {
        Scene scene;
        scene.name = j.value("name", "");
        if (j.contains("elements"))
        {
            for (const auto& se : j["elements"])
            {
                SceneElement sceneElement;
                sceneElement.elementIndex = se.value("elementIndex", 0);
                sceneElement.startTime = se.value("startTime", 0.0f);
                sceneElement.endTime = se.value("endTime", 1.0f);
                sceneElement.renderlevel = se.value("renderlevel", 0);
                sceneElement.selectedPose = se.value("selectedPose", "");
                scene.elements.push_back(sceneElement);
            }
        }
        return scene;
    }

    // Export Node to JSON
    json nodeToJson(const Node& node)
    {
        json j;
        j["name"] = node.name;
        j["sceneIndex"] = node.sceneIndex;
        j["position"]["x"] = node.position.x;
        j["position"]["y"] = node.position.y;
        j["dragType"] = static_cast<int>(node.dragType);
        j["color"] = colorToJson(node.color);
        j["isStartNode"] = node.isStartNode;
        j["connections"] = json::array();
        for (const auto& conn : node.connections)
        {
            json c;
            c["toNodeIndex"] = conn.toNodeIndex;
            c["choiceText"] = conn.choiceText;
            j["connections"].push_back(c);
        }
        return j;
    }

    // Import Node from JSON
    Node jsonToNode(const json& j)
    {
        Node node;
        node.name = j.value("name", "Node");
        node.sceneIndex = j.value("sceneIndex", -1);
        node.position.x = j["position"].value("x", 0.0f);
        node.position.y = j["position"].value("y", 0.0f);
        node.dragType = static_cast<DragType>(j.value("dragType", 0));
        node.color = j.contains("color") ? jsonToColor(j["color"]) : CLITERAL(Color){200, 200, 200, 255};
        node.isStartNode = j.value("isStartNode", false);
        if (j.contains("connections"))
        {
            for (const auto& c : j["connections"])
            {
                NodeConnection conn;
                conn.toNodeIndex = c.value("toNodeIndex", 0);
                conn.choiceText = c.value("choiceText", "");
                node.connections.push_back(conn);
            }
        }
        return node;
    }

    // Export all data to file
    void exportToFile(const std::vector<Element>& elements, const std::vector<Scene>& scenes, const std::vector<Node>& nodes, const std::string& filename)
    {
        json j;
        j["elements"] = json::array();
        for (const auto& element : elements)
        {
            j["elements"].push_back(elementToJson(element));
        }
        j["scenes"] = json::array();
        for (const auto& scene : scenes)
        {
            j["scenes"].push_back(sceneToJson(scene));
        }
        j["nodes"] = json::array();
        for (const auto& node : nodes)
        {
            j["nodes"].push_back(nodeToJson(node));
        }

        std::ofstream file(filename);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file for writing: " + filename);
        }
        file << j.dump(4);
        file.close();
    }

    // Import all data from file and load textures
    void importFromFile(std::vector<Element>& elements, std::vector<Scene>& scenes, std::vector<Node>& nodes, const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file for reading: " + filename);
        }

        json j;
        file >> j;
        file.close();

        elements.clear();
        if (j.contains("elements"))
        {
            for (const auto& je : j["elements"])
            {
                elements.push_back(jsonToElement(je));
            }
        }

        // Load textures for CharacterElement and BackgroundElement
        for (auto& element : elements)
        {
            if (element.type == ElementType::CHARACTER)
            {
                auto& character = std::get<CharacterElement>(element.data);
                character.textures.clear(); // Clear any existing textures
                for (const auto& img : character.images)
                {
                    if (!img.second.empty())
                    {
                        Image image = LoadImage(img.second.c_str());
                        if (image.data != nullptr)
                        {
                            Texture2D texture = LoadTextureFromImage(image);
                            character.textures.push_back(texture);
                            UnloadImage(image);
                        }
                        else
                        {
                            TraceLog(LOG_WARNING, "Failed to load image: %s", img.second.c_str());
                            character.textures.push_back({0}); // Push empty texture
                        }
                    }
                    else
                    {
                        character.textures.push_back({0}); // Push empty texture for empty path
                    }
                }
            }
            else if (element.type == ElementType::BACKGROUND)
            {
                auto& background = std::get<BackgroundElement>(element.data);
                if (!background.imagePath.empty())
                {
                    Image image = LoadImage(background.imagePath.c_str());
                    if (image.data != nullptr)
                    {
                        background.texture = LoadTextureFromImage(image);
                        UnloadImage(image);
                    }
                    else
                    {
                        TraceLog(LOG_WARNING, "Failed to load image: %s", background.imagePath.c_str());
                        background.texture = {0}; // Set empty texture
                    }
                }
                else
                {
                    background.texture = {0}; // Set empty texture for empty path
                }
            }
        }

        scenes.clear();
        if (j.contains("scenes"))
        {
            for (const auto& js : j["scenes"])
            {
                scenes.push_back(jsonToScene(js));
            }
        }

        nodes.clear();
        if (j.contains("nodes"))
        {
            for (const auto& jn : j["nodes"])
            {
                nodes.push_back(jsonToNode(jn));
            }
        }
    }
}

#endif // JSON_UTILS_HPP
