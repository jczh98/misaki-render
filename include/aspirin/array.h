#pragma once

#include <Eigen/Dense>
#include <memory>

namespace aspirin {

template <typename Value, int Dimension> class Array {
public:
    using Index = Eigen::Matrix<int, Dimension, 1>;
    using Self  = Array<Value, Dimension>;

    Array() : m_data(nullptr), m_shape(0), m_data_size(0) {}
    Array(const Index &index)
        : m_data(nullptr), m_shape(index), m_data_size(hprod(index)) {
        m_data = std::unique_ptr<Value[]>(new Value[m_data_size]);
    }
    Array(const Self &copy_from) : Array() {
        if (copy_from.m_data != nullptr) {
            std::copy(copy_from.m_data.get(),
                      copy_from.m_data.get() + copy_from.m_data_size,
                      m_data.get());
            m_shape     = copy_from.m_shape;
            m_data_size = copy_from.m_data_size;
        }
    }
    Array(Self &&move_from) noexcept : Array() {
        m_data      = std::move(move_from.m_data);
        m_shape     = move_from.m_shape;
        m_data_size = move_from.m_data_size;
    }

    static Self from_linear_indexed(const Index &shape,
                                    const std::function<Value(int)> &func) {
        Array<Value, Dimension> result(shape);
        auto *data = new Value[result.data_size()];
        for (int i = 0; i < result.data_size(); ++i) {
            new (&data[i]) Value(func(i));
        }
        result.m_data.reset(data);
        return result;
    }

    static Self from_array(const Index &shape, const Value *data) {
        return from_linear_indexed(shape, [&](int i) { return data[i]; });
    }

    Self &operator=(const Self &copy_from) {
        if (m_shape != copy_from.m_shape) {
            std::copy(copy_from.m_data.get(),
                      copy_from.m_data.get() + copy_from.m_data_size,
                      m_data.get());
            m_shape     = copy_from.m_shape;
            m_data_size = copy_from.m_data_size;
            return *this;
        }
        for (int i = 0; i < m_data_size; ++i)
            m_data[i] = copy_from.m_data[i];
        return *this;
    }
    Self &operator=(Self &&move_from) noexcept {
        m_data      = std::move(move_from.m_data);
        m_shape     = move_from.m_shape;
        m_data_size = move_from.m_data_size;
        return *this;
    }
    ~Array() {
        m_shape     = Index(0);
        m_data_size = 0;
    }

    Value &at(const Index &index) noexcept {
        assert(m_data != nullptr);
        if constexpr (Dimension == 2) {
            const int idx = index[0] * m_shape[1] + index[1];
            assert(idx < m_data_size);
            return m_data[idx];
        } else {
            int idx = 0, base = 1;
            for (int i = Dimension - 1; i >= 0; --i) {
                idx += index[i] * base;
                base *= m_shape[i];
            }
            assert(idx < m_data_size);
            return m_data[idx];
        }
    }

    const Value &at(const Index &index) const noexcept {
        assert(m_data != nullptr);
        if constexpr (Dimension == 2) {
            const int idx = index[0] * m_shape[1] + index[1];
            assert(idx < m_data_size);
            return m_data[idx];
        } else {
            int idx = 0, base = 1;
            for (int i = Dimension - 1; i >= 0; --i) {
                idx += index[i] * base;
                base *= m_shape[i];
            }
            assert(idx < m_data_size);
            return m_data[idx];
        }
    }

    const Index &shape() const noexcept { return m_shape; }
    int data_size() const noexcept { return m_data_size; }
    Value *raw_data() noexcept { return m_data.get(); }
    const Value *raw_data() const noexcept { return m_data.get(); }

private:
    std::unique_ptr<Value[]> m_data;
    Index m_shape;
    int m_data_size;
};

} // namespace aspirin::math