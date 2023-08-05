#ifndef GSLOT_H
#define GSLOT_H

#include <functional>
#include <limits>
#include <vector>

#include "gsignal.h"
#include "public/core/templates/shared_ptr.h"

template<typename T>
class GSlot
{
public:
    typedef std::function<T> func_type;
    typedef GSignal<T> signal_type;
    typedef GSharedPtr<GSignal<T>> signal_ref;
    typedef GWeakPtr<GSignal<T>> signal_wref;

    signal_ref connect(func_type callback)
    {
        auto t = GSignal<T>::create_signal(callback);
        m_signals.emplace_back(t);
        return t;
    }

    template<typename... Args>
    void operator()(Args... args)
    {
        auto iter = m_signals.begin();

        while (iter != m_signals.end())
        {
            GSharedPtr<GSignal<T>> sharedT = iter->as_shared();
            if (!sharedT.is_valid())
            {
                iter = m_signals.erase(iter);
            }
            else
            {
                sharedT.get()->operator()(args...);
                iter++;
            }
        }
    }

private:
    func_type m_slotFunc;
    std::vector<GWeakPtr<signal_type>> m_signals;

};

#endif //GSLOT_H