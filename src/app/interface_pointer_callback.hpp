#ifndef ZOAPPINTERFACEPOINTERCALLBACK_H
#define ZOAPPINTERFACEPOINTERCALLBACK_H

class IRightClickable;
class ILeftClickable;
class IHoverable;

class IUsePointer {
public:
    ~IUsePointer()=default;
protected:
    bool leftClickOn(ILeftClickable& clickable);
    bool rightClickOn(IRightClickable& clickable);
    bool hoverOn(IHoverable& hoverable);
};

class ILeftClickable {
    virtual bool onPointerLeftClick()=0;
friend class IUsePointer;
};

class IRightClickable {
    virtual bool onPointerRightClick()=0;
friend class IUsePointer;
};

class IHoverable {
    virtual bool onPointerHover()=0;
friend class IUsePointer;
};

inline bool IUsePointer::rightClickOn(IRightClickable& clickable) { return clickable.onPointerRightClick(); }
inline bool IUsePointer::leftClickOn(ILeftClickable& clickable) { return clickable.onPointerLeftClick(); }
inline bool IUsePointer::hoverOn(IHoverable& hoverable) { return hoverable.onPointerHover(); }

#endif
