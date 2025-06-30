#ifndef ZOAPPINTERFACEPOINTERCALLBACK_H
#define ZOAPPINTERFACEPOINTERCALLBACK_H

class IRightClickable;
class ILeftClickable;
class IHoverable;

class IUsePointer {
public:
    ~IUsePointer()=default;
protected:
    bool leftClickOn(ILeftClickable& clickable, glm::vec4 clickLocation);
    bool rightClickOn(IRightClickable& clickable, glm::vec4 clickLocation);
    bool hoverOn(IHoverable& hoverable, glm::vec4 hoverLocation);
};

class ILeftClickable {
    virtual bool onPointerLeftClick(glm::vec4 clickLocation)=0;
friend class IUsePointer;
};

class IRightClickable {
    virtual bool onPointerRightClick(glm::vec4 clickLocation)=0;
friend class IUsePointer;
};

class IHoverable {
    virtual bool onPointerHover(glm::vec4 hoverLocation)=0;
friend class IUsePointer;
};

inline bool IUsePointer::rightClickOn(IRightClickable& clickable, glm::vec4 clickLocation) { return clickable.onPointerRightClick(clickLocation); }
inline bool IUsePointer::leftClickOn(ILeftClickable& clickable, glm::vec4 clickLocation) { return clickable.onPointerLeftClick(clickLocation); }
inline bool IUsePointer::hoverOn(IHoverable& hoverable, glm::vec4 hoverLocation) { return hoverable.onPointerHover(hoverLocation); }

#endif
