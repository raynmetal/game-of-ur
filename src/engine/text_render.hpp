#ifndef ZOTEXTRENDER_H
#define ZOTEXTRENDER_H

#include <SDL2/SDL_ttf.h>

#include "core/resource_database.hpp"
#include "texture.hpp"

class TextFont;

class TextFont: public Resource<TextFont> {
public:
    virtual ~TextFont();
    explicit TextFont(const std::string& fontPath, uint16_t pointSize);
    inline static std::string getResourceTypeName() { return "TextFont"; }

    inline std::string getFontPath() { return mFontPath; }
    inline uint16_t getPointSize() { return mPointSize; }

    std::shared_ptr<Texture> renderText(
        const std::string& text,
        glm::u8vec3 textColor,
        glm::u8vec3 backgroundColor
    ) const;


private:
    static TTF_Font* LoadFont(const std::string& fontPath, uint16_t pointSize);

    TTF_Font* mFont;
    std::string mFontPath;
    uint16_t mPointSize;
};

class TextFontFromFile: public ResourceConstructor<TextFont, TextFontFromFile> {
public:
    explicit TextFontFromFile(): ResourceConstructor<TextFont, TextFontFromFile>{0} {}
    inline static std::string getResourceConstructorName() { return "fromFile"; }
private:
    std::shared_ptr<IResource> createResource(const nlohmann::json& methodParameters) override;
};

#endif
