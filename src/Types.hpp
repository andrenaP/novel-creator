#ifndef TYPES_HPP
#define TYPES_HPP

#include "raylib.h"
#include <vector>
#include <string>
#include <variant>
#include "json.hpp"

using json = nlohmann::json;

struct TextElement {
    std::string content;
};

struct CharacterElement {
    std::string name;
    std::vector<std::pair<std::string, std::string>> images;
    std::vector<Texture2D> textures;

    CharacterElement() = default;
    ~CharacterElement() {
        for (auto& texture : textures) {
            if (texture.id > 0) UnloadTexture(texture);
        }
    }
};

struct BackgroundElement {
    std::string imagePath;
    Texture2D texture = {0};

    ~BackgroundElement() {
        if (texture.id > 0) UnloadTexture(texture);
    }
};

enum class ElementType { TEXT, CHARACTER, BACKGROUND };

struct Element {
    ElementType type;
    std::string name;
    std::variant<TextElement, CharacterElement, BackgroundElement> data;

    Element() : type(ElementType::TEXT), name("New Element"), data(TextElement{""}) {}
    ~Element() = default;
};

struct SceneElement {
    size_t elementIndex;
    float startTime;
    float endTime;
};

struct Scene {
    std::string name;
    std::vector<SceneElement> elements;
};

#endif // TYPES_HPP
