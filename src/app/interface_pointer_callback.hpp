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
    bool leftReleaseOn(ILeftClickable& clickable, glm::vec4 clickLocation);
    bool pointerEnter(IHoverable& hoverable, glm::vec4 hoverLocation);
    bool pointerLeave(IHoverable& hoverable);

    // bool rightClickOn(IRightClickable& clickable, glm::vec4 clickLocation);
    // bool rightReleaseOn(IRightClickable& clickable, glm::vec4 clickLocation);
};

class ILeftClickable {
    virtual bool onPointerLeftClick(glm::vec4 clickLocation)=0;
    virtual bool onPointerLeftRelease(glm::vec4 clickLocation)=0;
friend class IUsePointer;
};

// class IRightClickable {
//     virtual bool onPointerRightClick(glm::vec4 clickLocation)=0;
//     virtual bool onPointerRightRelease(glm::vec4 clickLocation)=0;
// friend class IUsePointer;
// };

class IHoverable {
    virtual bool onPointerEnter(glm::vec4 hoverLocation)=0;
    virtual bool onPointerLeave()=0;
friend class IUsePointer;
};

// inline bool IUsePointer::rightClickOn(IRightClickable& clickable, glm::vec4 clickLocation) { return clickable.onPointerRightClick(clickLocation); }
// inline bool IUsePointer::rightReleaseOn(IRightClickable& clickable, glm::vec4 clickLocation) { return clickable.onPointerRightRelease(clickLocation); }

inline bool IUsePointer::leftClickOn(ILeftClickable& clickable, glm::vec4 clickLocation) { return clickable.onPointerLeftClick(clickLocation); }
inline bool IUsePointer::leftReleaseOn(ILeftClickable& clickable, glm::vec4 clickLocation) { return clickable.onPointerLeftRelease(clickLocation); }

inline bool IUsePointer::pointerEnter(IHoverable& hoverable, glm::vec4 hoverLocation) { return hoverable.onPointerEnter(hoverLocation); }
inline bool IUsePointer::pointerLeave(IHoverable& hoverable) { return hoverable.onPointerLeave(); }

#endif
