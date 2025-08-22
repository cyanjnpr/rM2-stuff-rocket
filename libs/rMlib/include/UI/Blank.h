#pragma once

#include <UI/RenderObject.h>
#include <UI/Widget.h>

namespace rmlib {

class BlankRenderObject;

// widget with literally nothing, useful if container needs to be empty
class Blank : public Widget<BlankRenderObject> {
    
  public:
  Blank(bool expand = true) : expand(expand) {}

  std::unique_ptr<RenderObject> createRenderObject() const;

  private:
    bool expand;
    friend class BlankRenderObject;

};

class BlankRenderObject : public LeafRenderObject<Blank> {

public:
BlankRenderObject(const Blank& widget) : LeafRenderObject(widget) {}

void update(const Blank& newWidget) {
  widget = &newWidget;
}

protected:
  Size doLayout(const Constraints& constraints) override {
    if (getWidget().expand) {
      return { constraints.max.width, constraints.max.height };
    }
    return { constraints.min.width, constraints.min.height };
  }

  UpdateRegion doDraw(rmlib::Rect rect, rmlib::Canvas& canvas) override { 
    return UpdateRegion{ rect };
  }

};

inline std::unique_ptr<RenderObject> Blank::createRenderObject() const {
    return std::make_unique<BlankRenderObject>(*this);
  }

}
