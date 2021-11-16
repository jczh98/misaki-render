#pragma once

#include "class.h"
#include <atomic>
#include <stdexcept>
#include <vector>

namespace misaki {

class APR_EXPORT Object {
public:
    Object() {}
    Object(const Object &) {}
    int ref_count() const { return m_ref_count; };
    void inc_ref() const { ++m_ref_count; }
    void dec_ref(bool dealloc = true) const noexcept;
    virtual std::vector<ref<Object>> expand() const;
    virtual const Class *clazz() const;
    virtual std::string id() const;
    virtual std::string to_string() const;

protected:
    virtual ~Object();

private:
    mutable std::atomic<int> m_ref_count{ 0 };

    static Class *m_class;
};

template <typename T> class ref {
public:
    // Create a <tt>nullptr</tt>-valued reference
    ref() {}

    // Construct a reference from a pointer
    template <typename T2 = T> ref(T *ptr) : m_ptr(ptr) {
        static_assert(std::is_base_of_v<Object, T2>,
                      "Cannot create reference to object not inheriting from "
                      "Object class.");
        if (m_ptr)
            ((Object *) m_ptr)->inc_ref();
    }

    // Construct a reference from another convertible reference
    template <typename T2> ref(const ref<T2> &r) : m_ptr((T2 *) r.get()) {
        static_assert(std::is_convertible_v<T2 *, T *>,
                      "Cannot create reference to object from another "
                      "unconvertible reference.");
        if (m_ptr)
            ((Object *) m_ptr)->inc_ref();
    }

    // Copy constructor
    ref(const ref &r) : m_ptr(r.m_ptr) {
        if (m_ptr)
            ((Object *) m_ptr)->inc_ref();
    }

    // Move constructor
    ref(ref &&r) noexcept : m_ptr(r.m_ptr) { r.m_ptr = nullptr; }

    // Destroy this reference
    ~ref() {
        if (m_ptr)
            ((Object *) m_ptr)->dec_ref();
    }

    // Move another reference into the current one
    ref &operator=(ref &&r) noexcept {
        if (&r != this) {
            if (m_ptr)
                ((Object *) m_ptr)->dec_ref();
            m_ptr   = r.m_ptr;
            r.m_ptr = nullptr;
        }
        return *this;
    }

    // Overwrite this reference with another reference
    ref &operator=(const ref &r) noexcept {
        if (m_ptr != r.m_ptr) {
            if (r.m_ptr)
                ((Object *) r.m_ptr)->inc_ref();
            if (m_ptr)
                ((Object *) m_ptr)->dec_ref();
            m_ptr = r.m_ptr;
        }
        return *this;
    }

    // Overwrite this reference with a pointer to another object
    template <typename T2 = T> ref &operator=(T *ptr) noexcept {
        static_assert(std::is_base_of_v<Object, T2>,
                      "Cannot create reference to an instance that does not"
                      " inherit from the Object class..");
        if (m_ptr != ptr) {
            if (ptr)
                ((Object *) ptr)->inc_ref();
            if (m_ptr)
                ((Object *) m_ptr)->dec_ref();
            m_ptr = ptr;
        }
        return *this;
    }

    // Compare this reference to another reference
    bool operator==(const ref &r) const { return m_ptr == r.m_ptr; }

    // Compare this reference to another reference
    bool operator!=(const ref &r) const { return m_ptr != r.m_ptr; }

    // Compare this reference to a pointer
    bool operator==(const T *ptr) const { return m_ptr == ptr; }

    // Compare this reference to a pointer
    bool operator!=(const T *ptr) const { return m_ptr != ptr; }

    // Access the object referenced by this reference
    T *operator->() { return m_ptr; }

    // Access the object referenced by this reference
    const T *operator->() const { return m_ptr; }

    // Return a C++ reference to the referenced object
    T &operator*() { return *m_ptr; }

    // Return a const C++ reference to the referenced object
    const T &operator*() const { return *m_ptr; }

    // Return a pointer to the referenced object
    operator T *() { return m_ptr; }

    // Return a pointer to the referenced object
    operator const T *() const { return m_ptr; }

    // Return a const pointer to the referenced object
    T *get() { return m_ptr; }

    // Return a pointer to the referenced object
    const T *get() const { return m_ptr; }

    // Check if the object is defined
    operator bool() const { return m_ptr != nullptr; }

private:
    T *m_ptr = nullptr;
};

// Prints the canonical string representation of an object instance
APR_EXPORT std::ostream &operator<<(std::ostream &os, const Object *object);

// Prints the canonical string representation of an object instance
template <typename T>
std::ostream &operator<<(std::ostream &os, const ref<T> &object) {
    return operator<<(os, object.get());
}

// This checks that it is safe to reinterpret a \c ref object into the
// underlying pointer type.
static_assert(sizeof(ref<Object>) == sizeof(Object *),
              "ref<T> must be reinterpretable as a T*.");

} // namespace misaki