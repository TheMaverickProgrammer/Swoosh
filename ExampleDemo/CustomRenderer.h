#pragma once
#include <Swoosh/Renderers/Renderer.h>
#include <Swoosh/Shaders.h>
#include <functional>
#include <array>
#include <optional>

struct Fake3D : RenderSource {
  sf::Texture& normal;
  sf::Sprite& sprite;
  std::optional<sf::Texture*> emissive;
  float z{};
  explicit Fake3D(sf::Sprite& src, sf::Texture& normal, std::optional<sf::Texture*> emissive = nullptr, float z = 0) :
    RenderSource(src),
    normal(normal),
    sprite(src),
    emissive(emissive),
    z(z)
  {}
};

struct Light : RenderSource {
  float radius{};
  sf::Vector3f position{};
  sf::Color color{ sf::Color::White };
  sf::CircleShape circle{};
  float specular{};
  explicit Light(float radius, sf::Vector3f position, sf::Color color, float specular = 0.0f) :
    RenderSource(circle),
    radius(radius),
    position(position),
    color(color),
    specular(specular)
  {
    circle.setPointCount(360);
    circle.setRadius(radius);
    circle.setFillColor(color);
    circle.setPosition({ position.x, position.y });

    sf::FloatRect bounds = circle.getLocalBounds();
    circle.setOrigin(bounds.width/2, bounds.height/2);
  };
};

sf::Vector3f WithZ(const sf::Vector2f xy, float z) {
  return sf::Vector3f(xy.x, xy.y, z);
}

class CustomRenderer : public Renderer<Immediate, Fake3D, Light> {
  sf::RenderTexture diffuse, normal, emissive, out;
  swoosh::glsl::deferred::LightPass lightShader;
  swoosh::glsl::deferred::MeshPass meshShader;
  std::list<RenderSource*> sources;
  std::list<RenderSource> memForward;
  std::list<Fake3D> mem3D;
  std::list<Light> memLight;

  std::size_t nextLightIdx{ 0 };
  std::array<sf::Glsl::Vec3, 30> lightPos{};
  std::array<sf::Glsl::Vec4, 30> lightColor{};
  std::array<float, 30> lightRadius{};

public:
  CustomRenderer(const sf::View view) {
    const sf::Vector2u size = sf::Vector2u(view.getSize().x, view.getSize().y);
    diffuse.create(size.x, size.y);
    normal.create(size.x, size.y);
    emissive.create(size.x, size.y);
    out.create(size.x, size.y);

    lightShader.setSurfaces(view, &diffuse, &normal, &emissive);
    meshShader.setSurfaces(&diffuse, &normal, &emissive);
  }

  void draw() override {
    sf::RenderStates lightPass;
    lightPass.blendMode = sf::BlendAdd;

    for (RenderSource* source : sources) {
      if (Fake3D* ptr3D = dynamic_cast<Fake3D*>(source); ptr3D) {
        sf::Texture* texEmissive = ptr3D->emissive.value();;
        meshShader.setSprite(&ptr3D->sprite, &ptr3D->normal, texEmissive);

        // bake the currect normals
        meshShader.apply(*this);
        continue;
      }

      out.draw(source->drawable(), source->states());
    }

    lightShader.clearLights();
    for (Light& source : memLight) {
      lightShader.addLight(source.radius, sf::Glsl::Vec3(source.position.x, source.position.y, source.position.z), source.color, source.specular);
    }

    // compose final scene
    lightShader.apply(*this);

    // reset the light index
    nextLightIdx = 0;

    // clear the buffer data
    sources.clear();
    memForward.clear();
    mem3D.clear();
    memLight.clear();
  }

  void display() override {
    out.display();
  }

  void clear(sf::Color color) override {
    diffuse.clear(sf::Color::Transparent);
    normal.clear(sf::Color::Transparent);
    emissive.clear(sf::Color::Transparent);
    out.clear(color);
  }

  void setView(const sf::View& view) override {
    diffuse.setView(view);
    normal.setView(view);
    emissive.setView(view);
    out.setView(view);
  }

  sf::Texture getTexture() override {
    return out.getTexture();
  }

  void onEvent(const RenderSource& event) override {
    memForward.emplace_back(event);
    sources.push_back(&memForward.back());
  }

  void onEvent(const Immediate& event) override {
    out.draw(event.drawable(), event.states());
  }

  void onEvent(const Fake3D& event) override {
    mem3D.emplace_back(event);
    sources.push_back(&mem3D.back());
  }

  void onEvent(const Light& event) override {
    memLight.emplace_back(event);
  }
};