#pragma once
#include <Swoosh/Renderers/Renderer.h>

namespace swoosh {
    class SimpleRenderer : public Renderer<Immediate> {
        sf::RenderTexture surface;
        std::list<RenderSource> sources;

        public:
        SimpleRenderer(const sf::Vector2u size) {
            surface.create(size.x, size.y);
        }

        void draw() override {
            for(RenderSource& source : sources) {
                surface.draw(source.drawable(), source.states());
            }

            sources.clear();
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
            sources.push_back(event);
        }

        void onEvent(const Immediate& event) override {
            surface.draw(event.drawable(), event.states());
        }
    };
}