#ifndef ZOAPPINTERFACEPOINTERCALLBACK_H
#define ZOAPPINTERFACEPOINTERCALLBACK_H

class IClickable;
class IHoverable;

class IUsePointer {
public:
    ~IUsePointer()=default;
protected:
    bool clickOn(IClickable& clickable);
    bool hoverOn(IHoverable& hoverable);
};

class IClickable {
    virtual bool onPointerClick()=0;
friend class IUsePointer;
};

class IHoverable {
    virtual bool onPointerHover()=0;
friend class IUsePointer;
};

inline bool IUsePointer::clickOn(IClickable& clickable) { return clickable.onPointerClick(); }
inline bool IUsePointer::hoverOn(IHoverable& hoverable) { return hoverable.onPointerHover(); }


#endif
