#ifndef FOOLSENGINE_TEXTRENDER_H
#define FOOLSENGINE_TEXTRENDER_H

#include <SDL2/SDL_ttf.h>

#include "core/resource_database.hpp"
#include "texture.hpp"

namespace ToyMakersEngine{
    class TextFont;

    class TextFont: public Resource<TextFont> {
    public:
        virtual ~TextFont();
        explicit TextFont(const std::string& fontPath, uint16_t pointSize);
        inline static std::string getResourceTypeName() { return "TextFont"; }

        inline std::string getFontPath() const { return mFontPath; }
        inline uint16_t getPointSize() const { return mPointSize; }

        std::shared_ptr<Texture> renderText(
            const std::string& text,
            glm::u8vec3 textColor,
            glm::u8vec3 backgroundColor
        ) const;

        std::shared_ptr<Texture> renderTextArea(
            const std::string& text,
            glm::u8vec4 textColor,
            uint32_t wrapLength=0
        ) const;

        std::shared_ptr<Texture> renderText(
            const std::string& text,
            glm::u8vec4 textColor
        ) const;


    private:
        static TTF_Font* LoadFont(const std::string& fontPath, uint16_t pointSize);
        static std::shared_ptr<Texture> MakeTexture(SDL_Surface* surface);

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
}

#endif
