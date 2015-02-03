#pragma once


template <typename StateType> class DynamicObjectBase;

template <typename StateType>
class DynamicObjectBaseDeleter {
public:
    void operator() (DynamicObjectBase<StateType> *o) {
        delete o;
    }
};

template<typename StateType>
using DynamicObjectSmartPtr = std::unique_ptr< DynamicObjectBase<StateType>, DynamicObjectBaseDeleter<StateType> >;

template <typename StateType>
class ActiveContainer : public vector<DynamicObjectSmartPtr<StateType>> {
};
