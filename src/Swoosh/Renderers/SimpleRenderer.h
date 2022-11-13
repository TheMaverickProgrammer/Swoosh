#pragma once
#include <Swoosh/Renderers/Renderer.h>

namespace swoosh {
  class SimpleRenderer : public Renderer<> {
    sf::RenderTexture surface;

  public:
    SimpleRenderer(const sf::View view) {
      surface.create(view.getSize().x, view.getSize().y);
    }

    void draw() override {
      /* 
      Intentionally empty because the simple renderer 
      draws submitted resources directly to the 
      target surface and doesn't need a special draw step 
      */
    }

    void display() override {
      surface.display();
    }

    void clear(sf::Color color) override {
      surface.clear(color);
    }

    void setView(const sf::View& view) override {
      surface.setView(view);
    }

    sf::Texture getTexture() override {
      return surface.getTexture();
    }

    void onEvent(const RenderSource& event) override {
      surface.draw(event.drawable(), event.states());
    }
  };
}