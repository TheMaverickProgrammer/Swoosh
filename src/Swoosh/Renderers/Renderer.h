#pragma once
#include <Swoosh/Events/Events.h>
#include <SFML/Graphics.hpp>

using swoosh::events::IDispatcher;
using swoosh::events::ISubscriber;

namespace swoosh {
    class RenderSource {
    private:
        sf::Drawable& ref;
        sf::RenderStates statesIn;

    public:
        RenderSource(sf::Drawable& src, const sf::RenderStates& states = sf::RenderStates()) 
            : ref(src), statesIn(states) {}
        virtual ~RenderSource() {}

        sf::Drawable& drawable() { return ref; }
        sf::RenderStates& states() { return statesIn; }
    };

    class IRenderer : public IDispatcher<RenderSource> {
    public:
        virtual ~IRenderer(){}

        // shortcut
        void submit(sf::Drawable& drawable, const sf::RenderStates& states = sf::RenderStates()) {
          IDispatcher<RenderSource>::submit(RenderSource(drawable, states));
        }

        virtual void draw() = 0;
        virtual void display() = 0;
        virtual void clear(sf::Color color = sf::Color::Transparent) = 0;
        virtual void setView(const sf::View& view) = 0;
        virtual sf::Texture getTexture() = 0;
    };

    template<typename... Ts>
    class Renderer : public IRenderer, public ISubscriber<RenderSource, Ts...> {
    private:
        void broadcast(const char* name, void* src, bool ofBaseEvent) override {
            this->redirect(name, src, ofBaseEvent);
        }

    public:
        virtual ~Renderer(){}
    };
}