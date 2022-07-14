#ifndef BAMBOO_BASE_SINGLETON_H
#define BAMBOO_BASE_SINGLETON_H

namespace bamboo {

template<class T>
class Singleton {
    public:
        Singleton() = delete;
        ~Singleton() = delete;
        
        // template<class ...Args>
        // static T* getInstance(Args... args) {
        //     static T obj(args...);
        //     return &obj;
        // }

        static T* getInstance() {
            static T obj;
            return &obj;
        }
};

}

#endif