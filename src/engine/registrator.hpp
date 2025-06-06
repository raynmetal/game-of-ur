#ifndef FOOLSENGINE_REGISTRATOR_H
#define FOOLSENGINE_REGISTRATOR_H

namespace ToyMakersEngine {

    /**  
     *  Forces implementation of static function `registerSelf()` using CRTP, called here.
     * 
     * ## Usage:
        ```c++
        class YourClass {
            // ... the rest of your class definition
            // ...
            YourClass() {s_registrator.emptyFunc()} // Explicit constructor definition
        
            // This function will be called by constructor Registrator<YourClass>()
            static YourReturnType registerSelf() {
                // Ensure correct order of registration
                Registrator<ClassYouDependOn>::getRegistrator();
                Registrator<AnotherClassYouDependOn>::getRegistrator();

                // ... whatever the class needs to do to register itself wherever
                // it needs to be registered
            }
    
            inline static Registrator<YourClass> s_registrator { Registrator<YourClass>::getRegistrator() };
        }
        ```
    */
    template<typename TRegisterable>
    class Registrator {
    public:
        inline static Registrator& getRegistrator() {
            static Registrator reg {};
            return reg;
        };
        void emptyFunc() {}
    protected:
        Registrator();
    };

    template <typename TRegisterable>
    Registrator<TRegisterable>::Registrator() {
        TRegisterable::registerSelf();
    }

}

#endif
