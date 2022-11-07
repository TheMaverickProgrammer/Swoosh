#pragma once
#include <map>

//
// Event dispatcher interfaces introduced in Swoosh v2.0.0
//

namespace swoosh {
    namespace events {
        template<typename EventBase>
        struct IDispatcher {
            virtual ~IDispatcher(){}
            
            virtual void broadcast(const char* name, void* src, bool isIRenderSource) = 0;

            template<typename Event>
            void submit(const Event& event) {
                constexpr bool ofEventBase = std::is_base_of<typename EventBase, typename Event>::value;
                static_assert(ofEventBase, "Cannot submit `event` because it does not have a base class of EventBase type");
                broadcast(typeid(Event).name(), (void*)&event, ofEventBase);
            }
        };

        template<typename T>
        struct IReceiver {
            virtual ~IReceiver() {}
            virtual void onEvent(const T& src) = 0;

            void trampoline(void* src) {
                onEvent(*((const T*)src));
            }
        };

        template<typename EventBase, typename... Ts>
        struct ISubscriber : public IReceiver<Ts>... {
            typedef void(*funptr)(ISubscriber* ptr, void* src);
            using typestable = std::map<const char*, funptr>;
            typestable types;

            ISubscriber() {
                (types.emplace(typeid(Ts).name(),+[](ISubscriber* ptr, void* src) {
                    ptr->IReceiver<Ts>::trampoline(src);
                }), ...);
            }
            virtual ~ISubscriber(){}

            void redirect(const char* name, void* src, bool ofBaseEvent)  {
                if(types.count(name))
                    types[name](this, src);
                else if(ofBaseEvent) 
                    onEvent(*((const EventBase*)src));
            }

            virtual void onEvent(const EventBase& other) = 0;
        };
    }
}