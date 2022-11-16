#pragma once
#include <Swoosh/Renderers/Renderer.h>
#include <Swoosh/Shaders.h>
#include <functional>
#include <array>
#include <optional>

struct Draw3D : RenderSource {
  glsl::deferred::MeshData data;
  explicit Draw3D(sf::Sprite& spr, sf::Texture* normal, sf::Texture* emissive = nullptr, float metallic = 0, float specular = 0) :
    RenderSource(spr),
    data({ &spr, normal, emissive, metallic, specular })
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

template<typename T>
struct Clone : RenderSource {
  const std::shared_ptr<T> in;
  static inline const sf::RectangleShape dummy{};
  Clone(const T& in) : RenderSource(dummy), in(std::make_shared<T>(in)) {}

  const sf::Drawable& drawable() const override{
    return *in.get();
  }
};

sf::Vector3f WithZ(const sf::Vector2f xy, float z) {
  return sf::Vector3f(xy.x, xy.y, z);
}

class CustomRenderer : public Renderer<Immediate, Draw3D, Light> {
  sf::RenderTexture diffuse, normal, esm, special, out;
  swoosh::glsl::deferred::LightPass lightShader;
  swoosh::glsl::deferred::MeshPass meshShader;
  std::list<RenderSource> memForward;
  std::list<Draw3D> mem3D;
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
    esm.create(size.x, size.y);
    out.create(size.x, size.y);

    meshShader.setSurfaces(&diffuse, &normal, &esm);
    lightShader.setSurfaces(view, &diffuse, &normal, &esm);
  }

  void draw() override {
    for (Draw3D& source : mem3D) {
      meshShader.setMeshData(source.data);

      // bake the currect normals
      meshShader.apply(*this);
    }

    lightShader.clearLights();
    for (Light& source : memLight) {
      lightShader.addLight(source.radius, sf::Glsl::Vec3(source.position.x, source.position.y, source.position.z), source.color, source.specular);
    }

    // compose final shaded scene
    lightShader.apply(*this);

    // draw forward rendered content
    for (RenderSource& source : memForward) {
      // TODO: fix or remove
      if (const Clone<sf::Sprite>* ptr = dynamic_cast<const Clone<sf::Sprite>*>(&source); ptr) {
        out.draw(ptr->drawable());
        continue;
      }
      out.draw(source.drawable());
    }

    // reset the light index
    nextLightIdx = 0;

    // clear the buffer data
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
    esm.clear(sf::Color::Transparent);
    out.clear(color);
  }

  void setView(const sf::View& view) override {
    diffuse.setView(view);
    normal.setView(view);
    esm.setView(view);
    out.setView(view);
  }

  sf::Texture getTexture() override {
    return out.getTexture();
  }

  void onEvent(const RenderSource& event) override {
    memForward.push_back(event);
  }

  void onEvent(const Immediate& event) override {
    out.draw(event.drawable(), event.states());
  }

  void onEvent(const Draw3D& event) override {
    mem3D.emplace_back(event);
  }

  void onEvent(const Light& event) override {
    memLight.emplace_back(event);
  }
};