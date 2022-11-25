#pragma once
#include <Swoosh/Renderers/Renderer.h>

namespace swoosh {
  /**
  @class SimpleRenderer
  @brief A composite renderer that comes with Swooshlib
  */
  class SimpleRenderer : public Renderer<> {
    sf::RenderTexture surface; // !< One surface to draw to

  public:
    SimpleRenderer(const sf::View view) {
      const unsigned int ux = (unsigned int)view.getSize().x;
      const unsigned int uy = (unsigned int)view.getSize().y;
      surface.create(ux, uy);
    }

    void draw() override {
      /* 
      Intentionally empty because the simple renderer 
      draws submitted resources directly to the 
      target surface and doesn't need a special draw step 
      */
    }

    sf::RenderTexture& getRenderTextureTarget() {
      return surface;
    }

    void clear(sf::Color color) override {
      surface.clear(color);
    }

    void setView(const sf::View& view) override {
      surface.setView(view);
    }

    /**
     * @brief The only event handler. All drawables will be promptly drawn into the render surface
    */
    void onEvent(const RenderSource& event) override {
      surface.draw(*event.drawable(), event.states());
    }
  };
}