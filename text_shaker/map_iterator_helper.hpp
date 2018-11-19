// Iterator helpers, so we can create objects just from values or just from keys
template<typename MapIterator_t>
class MapValueIterator : public MapIterator_t {
public:
    using value_type = typename MapIterator_t::value_type::second_type;
    using pointer = std::add_pointer_t<std::add_const_t<value_type>>;
    using reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;

    template<typename T>
    MapValueIterator(T&& in) noexcept :
        MapIterator_t(std::forward<T>(in))
    {}

    reference operator*() const noexcept {
        return MapIterator_t::operator*().second;
    }

    pointer operator->() const noexcept {
        return &MapIterator_t::operator->()->second;
    }
};

template<typename MapIterator_t>
MapValueIterator(MapIterator_t&& in) -> MapValueIterator<MapIterator_t>;

template<typename MapIterator_t>
class MapKeyIterator : public MapIterator_t {
public:
    using value_type = typename MapIterator_t::value_type::first_type;
    using pointer = std::add_pointer_t<std::add_const_t<value_type>>;
    using reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;

    template<typename T>
    MapKeyIterator(T&& in) noexcept :
        MapIterator_t(std::forward<T>(in))
    {}

    reference operator*() const noexcept {
        return MapIterator_t::operator*().first;
    }

    pointer operator->() const noexcept {
        return &MapIterator_t::operator->()->first;
    }
};

template<typename MapIterator_t>
MapKeyIterator(MapIterator_t&& in) -> MapKeyIterator<MapIterator_t>;
