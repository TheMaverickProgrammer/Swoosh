#pragma once
#include <map>
#include <typeinfo>
#include <type_traits>

//
// Event dispatcher interfaces introduced in Swoosh v2.0.0
//

namespace swoosh {
  namespace events {
    /**
      @class IDispatcher<E>
      @brief am abstract class that broadcasts submitted events with base type `E`
    */
    template<typename E>
    struct IDispatcher {
      virtual ~IDispatcher() {}

      /**
        @brief Implementation defined
      */
      virtual void broadcast(const char* name, void* src) = 0;

      /**
        @brief Sends the event to the correct handling function
        @param event Event type const reference
        @note `Event` type must inherit from base type `E`
      */
      template<typename Event>
      void submit(const Event& event) {
        constexpr bool ofEventBase = std::is_base_of<E, Event>::value;
        static_assert(ofEventBase, "Cannot submit `event` because it does not have a base class `RenderSource`");
        broadcast(typeid(Event).name(), (void*)&event);
      }
    };

    /**
      @class IReceiver<T>
      @brief an abstract class that recieves type-erased data for the given type `T`
    */
    template<typename T>
    struct IReceiver {
      virtual ~IReceiver() {}

      /**
        @brief Implementation defined
      */
      virtual void onEvent(const T& src) = 0;

      /**
        @brief recasts the void* src type into type `T`
      */
      void trampoline(void* src) {
        onEvent(*((const T*)src));
      }
    };

    /**
      @class ISubscriber<E, ...Ts>
      @brief an abstract class that redrects typename data to the correct reciever 
    */
    template<typename E, typename... Ts>
    struct ISubscriber : public IReceiver<Ts>... {
      typedef void(*funptr)(ISubscriber* ptr, void* src);
      using typestable = std::map<const char*, funptr>;
      typestable types;

      ISubscriber() {
        (types.emplace(typeid(Ts).name(), +[](ISubscriber* ptr, void* src) {
          ptr->IReceiver<Ts>::trampoline(src);
          }), ...);
      }
      virtual ~ISubscriber() {}

      /**
        @brief Given a type name, find the registered reciever type and invoke its `onEvent()` function
      */
      void redirect(const char* name, void* src) {
        if (types.count(name)) {
          types[name](this, src);
          return;
        }
        
        // assertion gaurantees `src` will have a type with base of `E`
        onEvent(*((const E*)src));
      }

    /**
      @brief Base event handler. Implementation defined.
    */
      virtual void onEvent(const E& other) = 0;
    };
  }
}