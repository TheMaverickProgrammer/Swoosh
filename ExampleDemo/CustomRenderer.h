#pragma once
#include <Swoosh/Renderers/Renderer.h>
#include <Swoosh/Shaders.h>
#include <functional>

struct Fake3D : RenderSource {
  sf::Texture& normal;
  sf::Sprite& sprite;
  explicit Fake3D(sf::Sprite& src, sf::Texture& normal) : 
    RenderSource(src),
    normal(normal),
    sprite(src)
  {}
};

class CustomRenderer : public Renderer<Immediate, Fake3D> {
  sf::RenderTexture surface;
  swoosh::glsl::Deferred shader;
  std::list<RenderSource*> sources;
  std::list<RenderSource> memRs;
  std::list<Fake3D> mem3D;

public:
  CustomRenderer(const sf::Vector2u size) {
    shader.setCamera(sf::Vector3f(size.x * 0.5f, size.y * 0.5f, -10.f));
    surface.create(size.x, size.y);
  }

  void setLightPos(const sf::Vector3f pos) {
    shader.setLight(pos);
  }

  void draw() override {
    for (RenderSource* source : sources) {
      if (Fake3D* ptr3D = dynamic_cast<Fake3D*>(source); ptr3D) {
        shader.setSprite(&ptr3D->sprite);
        shader.setNormal(&ptr3D->normal);
        sf::RenderStates states;
        states.shader = &shader.getShader();
        surface.draw(ptr3D->sprite, states);
        continue;
      }

      surface.draw(source->drawable(), source->states());
    }

    sources.clear();
    memRs.clear();
    mem3D.clear();
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
    memRs.emplace_back(event);
    sources.push_back(&memRs.back());
  }

  void onEvent(const Immediate& event) override {
    surface.draw(event.drawable(), event.states());
  }

  void onEvent(const Fake3D& event) override {
    mem3D.emplace_back(event);
    sources.push_back(&mem3D.back());
  }
};