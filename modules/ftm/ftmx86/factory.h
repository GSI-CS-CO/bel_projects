#ifndef FACTORY_H
#define FACTORY_H

#include <map>
#include <sring>

// a generic factory, string -> derivedType

template <typename T>
class Factory
{
public:
    template <typename TDerived>
    void registerType(std::string name)
    {
        static_assert(std::is_base_of<T, TDerived>::value, "Factory::registerType doesn't accept this type because doesn't derive from base class");
        _createFuncs[name] = &createFunc<TDerived>;
    }

    T* create(std::string name) {
        typename std::map<std::string,PCreateFunc>::const_iterator it = _createFuncs.find(name);
        if (it != _createFuncs.end()) {
            return it.value()();
        }
        return nullptr;
    }

private:
    template <typename TDerived>
    static T* createFunc()
    {
        return new TDerived();
    }

    typedef T* (*PCreateFunc)();
    std::map<std::string,PCreateFunc> _createFuncs;
};

#endif
