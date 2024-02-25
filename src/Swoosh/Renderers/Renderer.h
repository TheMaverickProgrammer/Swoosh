#pragma once
#include <Swoosh/Events/Events.h>
#include <SFML/Graphics.hpp>
#include <list>
#include <type_traits>

using swoosh::events::IDispatcher;
using swoosh::events::ISubscriber;

namespace swoosh {
  class IRenderer; /* forward declare */

  /**
    @class RendererEntry
    @brief Simple aggregate struct that houses a renderer and its name
  */
  struct RendererEntry {
    const char* name;
    IRenderer& renderer;
  };

  /**
    @class RenderEntries
    @brief short-hand list of RenderEntry records
  */
  using RendererEntries = std::list<RendererEntry>;

  /**
    @class RenderSource
    @brief Base event class type that has a reference to an SFML primitive
    @note Inherit from this class to design new event types
  */
  class RenderSource {
  private:
    const sf::Drawable* dptr{nullptr}; //!< pointer to SFML primitive
    const sf::RenderStates statesIn; //!< Copy of the render states to draw the primitive

  public:
    /**
      @brief constructs a RenderSource event from an SFML primitive and optional render states
      @example renderer.submit(RenderSource(sprite, states));
    */
    explicit RenderSource(const sf::Drawable* src, const sf::RenderStates& states = sf::RenderStates())
      : dptr(src), statesIn(states) {}
    virtual ~RenderSource() {}

    /**
      @brief Returns a pointer to the SFML primitive
    */
    const sf::Drawable* drawable() const { return dptr; }

    /**
      @brief Returns the render states info
    */
    const sf::RenderStates& states() const { return statesIn; }
  };

  /**
    @class Immediate
    @brief A lite event type used to distinguish drawables for a composite renderer pass
  */
  class Immediate : public RenderSource {
  public:
    /**
      @brief constructs an Immediate render event type
      @example renderer.submit(Immediate(&sprite, states));
    */
    Immediate(const sf::Drawable* src, const sf::RenderStates& states = sf::RenderStates()) : RenderSource(src, states) {}
  };

  // internal utility structs
  namespace {
    // utility to clone event data
    template<typename T, bool isRenderEvent>
    struct usefulCopier_t {};

    // alias
    template<typename T>
    using usefulCopier = usefulCopier_t<T, std::is_base_of_v<T, RenderSource>>;
    
    // clone event data from other event types and put onto the heap
    template<typename T>
    struct usefulCopier_t<T, true> {
      static RenderSource* exec(const T& from, sf::Drawable* dptr, const char** tname) { 
        *tname = typeid(T).name(); 
        return new T(from); 
      }
    };
    
    // clone event data from SFML primitives, wrapping it in a RenderSource on the heap
    template<typename T>
    struct usefulCopier_t<T, false> {
      static RenderSource* exec(const T& from, sf::Drawable** dptr, const char** tname) { 
        *tname = typeid(RenderSource).name();
        *dptr = new T(from);
        return new RenderSource(*dptr);
      }
    };
  }

  // shorthand SFNAE for Swoosh render events
  template<typename T>
  constexpr bool is_render_event_v = std::is_base_of<RenderSource, std::remove_pointer_t<T>>::value;

  // shorthand SFNAE for SFML primitives
  template<typename T>
  constexpr bool is_sfml_primitive_v = std::is_base_of<sf::Drawable, std::remove_pointer_t<T>>::value;

  /**
    @class ClonedSource
    @brief Retains heap memory for SFML drawables and swoosh events that were cloned
    Memory is deleted at the end of the `draw()` routine in the ActivityController
  */
  struct ClonedSource : RenderSource {
    class DeletePolicy {
    public:
      virtual ~DeletePolicy() {}
      virtual void free(void*) = 0;
    };

    template<typename T>
    struct Deleter : public ClonedSource::DeletePolicy {
      void free(void* mem) override {
        T* asT = (T*)mem;
        delete asT;
      }
    };

    const char* name{ 0 }; //!< typename
    void* mem{ nullptr }; //!< type-erased memory
    sf::Drawable* dptr{ nullptr }; //!< ptr to SFML drawable (may be nullptr)
    DeletePolicy* deleter{ nullptr };

    void freeMemory() {
      delete dptr;

      if (!deleter) return;
      deleter->free(mem);
    }

    ClonedSource(void* memIn, sf::Drawable* dptr, const char* nameIn, DeletePolicy* policy = nullptr) : 
      name(nameIn), mem(memIn), dptr(dptr), deleter(policy), RenderSource(dptr) {}
  };

  /**
    @brief Clones render events and SFML primitives. Useful when re-using the same drawable variables in your scene.
    @warning T's copy constructor is invoked and the data is allocated onto the heap to be deleted for you.
    @param t The resource to make a copy of
    @return ClonedSource has a copy of `t` stored on the heap. Any underlying event will be unwrapped and re-submitted.
  */
  template<typename T>
  ClonedSource Clone(const T& t) {
    static ClonedSource::Deleter<T> deleter_type_policy;

    const char* tname {0};
    sf::Drawable* dptr {nullptr};
    RenderSource* ptr = usefulCopier<T>::exec(t, &dptr, &tname);
    return ClonedSource((void*)ptr, dptr, tname, &deleter_type_policy);
  }

  /**
    @class IRenderer
    @brief RenderSource event dispatcher used internally by the ActivityController to replace the old draw pipeline
  */
  class IRenderer : public IDispatcher<RenderSource> {
    friend class ActivityController;

    /**
      @brief Free any allocated memory used by the specialization of this class
    */
    virtual void flushMemory() = 0;

  public:
    virtual ~IRenderer() { }

    /**
      @brief Submits a custom render event
      @param event A custom event object to be handled by the renderer
    */
    template<typename Event, typename use = std::enable_if_t<(is_render_event_v<Event> || !is_sfml_primitive_v<Event>)>>
    void submit(const Event& event) {
      IDispatcher::submit(event);
    }

    /**
      @brief This shortcut for SFML users submits any drawable as a basic render event
      @param drawable a pointer to any class that inherits from SFML drawable
      @param states RenderState info for this graphic
    */
    void submit(const sf::Drawable* drawable, const sf::RenderStates& states = sf::RenderStates()) {
      IDispatcher::submit(RenderSource(drawable, states));
    }


    /**
      @brief Implementation defined. The ActivityController draw step invokes this callback.
      @note This function must be used to compose the final texture drawn to the screen.
    */
    virtual void draw() = 0;

    /**
      @brief Implementation defined. The ActivityController draw step invokes this callback.
      @note This function must be used to clear all render surfaces before drawing to them.
    */
    virtual void clear(sf::Color color = sf::Color::Transparent) = 0;

    /**
      @brief Implementation defined. The ActivityController update step invokes this callback.
      @note This function must be used to update all render surfaces with the new screen resolution.
    */
    virtual void setView(const sf::View& view) = 0;

    /**
      @brief Return the render texture target that represents the screen's fully composed output
      @return sf::RenderTexture& a reference to the render texture target
    */
    virtual sf::RenderTexture& getRenderTextureTarget() = 0;

    /**
      @brief Prepares the render texture target for displaying on the screen by invoking `display()`
    */
    void display() { getRenderTextureTarget().display(); }

    /**
     * @brief Creates a texture copy of the current renderer's output for the scene. Useful for displaying or doing post-processing effects.
     * @return sf::Texture
    */
    sf::Texture getTexture() { return getRenderTextureTarget().getTexture(); }
  };

  /**
    @class Renderer<...Ts>
    @brief All renderers must implement this class
    Given a list of render event types [Ts...], require an `onEvent(e)` function to be implemented for each
    @example class MyRenderer : Renderer<UI, Layers, Particles>{ ... }
  */
  template<typename... Ts>
  class Renderer : public IRenderer, public ISubscriber<RenderSource, Immediate, ClonedSource, Ts...> {
  private:
    std::vector<ClonedSource> clonedMem; //!< Track ClonedSource objects

    /**
      @brief forwards the broadcasted render event to through the ISubscriber<> implementation
    */
    void broadcast(const char* name, void* src, bool is_base) override {
      this->redirect(name, src, is_base);
    }

    /**
      @brief Built-in event handler for cloned render source events that unpacks and re-submits contained data
    */
    void onEvent(const ClonedSource& event) override {
      ClonedSource& ref = clonedMem.emplace_back(std::move(event));
      this->redirect(ref.name, ref.mem, true);
    }

    /**
    @brief Built-in event handler for immediate render events that draw directly to the assigned render target at the time of call
    */
    void onEvent(const Immediate& event) override {
      getRenderTextureTarget().draw(*event.drawable(), event.states());
    }

    /**
      @brief Free's mem and dptr from cloned sources
    */
    void flushMemory() override {
      for(ClonedSource& c : clonedMem) {
        c.freeMemory();
      }
      clonedMem.clear();
    }

  public:
    /**
      @brief deconstructor gaurantees `flushMemory` function is called
    */
    virtual ~Renderer() { flushMemory(); }
  };
}