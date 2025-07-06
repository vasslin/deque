#pragma once
#include "deque.h"

template <typename T, typename Allocator>
deque<T, Allocator>::deque() : deque(Allocator()){};

template <typename T, typename Allocator>
deque<T, Allocator>::deque(const Allocator& alloc) : deque(alloc, PMapAlloc()) {}

template <typename T, typename Allocator>
deque<T, Allocator>::deque(const Allocator& alloc, const PMapAlloc& pmap_alloc)
    : alloc_(alloc),
      pmap_alloc_(pmap_alloc),
      start_node_(nullptr),
      finish_node_(nullptr),
      curr_begin_node_(nullptr),
      curr_end_node_(nullptr),
      begin_ind_(0),
      end_ind_(0),
      buffer_size_(deque_buffer_sz(sizeof(T)))

{
    default_constr_with_memory_cap(0);
}

template <typename T, typename Allocator>
void deque<T, Allocator>::default_constr_with_memory_cap(size_type nodes_cnt, size_type borders_offset) {
    // creates an empty deque with storage capacity
    start_node_ = pmap_alloc_traits::allocate(pmap_alloc_, nodes_cnt + borders_offset * 2);
    finish_node_ = start_node_ + nodes_cnt + borders_offset * 2;
    curr_begin_node_ = start_node_ + borders_offset;
    curr_end_node_ = curr_begin_node_ - 1;
    begin_ind_ = 0;
    end_ind_ = buffer_size_;
}

template <typename T, typename Allocator>
deque<T, Allocator>::deque(size_type count, const Allocator& alloc)
    : alloc_(alloc), buffer_size_(deque_buffer_sz(sizeof(T))), pmap_alloc_(PMapAlloc()) {
    static_assert(std::is_default_constructible_v<T>, "The stored value must have default constructor.");

    size_type sz = count / buffer_size_ + (count % buffer_size_ == 0 ? 0 : 1);
    default_constr_with_memory_cap(sz);
    size_type i;
    try {
        for (i = 0; i < count; ++i) {
            emplace_back(T());
        }
    } catch (const std::exception& e) {
        for (size_type j = 0; j < i; ++j) {
            pop_back();
        }
        pmap_alloc_traits::deallocate(pmap_alloc_, start_node_, sz + 2);
    }
}

template <typename T, typename Allocator>
deque<T, Allocator>::deque(size_type count, const T& value, const Allocator& alloc)
    : alloc_(alloc), buffer_size_(deque_buffer_sz(sizeof(T))), pmap_alloc_(PMapAlloc()) {
    static_assert(std::is_copy_constructible_v<T>, "The stored value must have copy constructor.");

    size_type sz = count / buffer_size_ + (count % buffer_size_ == 0 ? 0 : 1);
    default_constr_with_memory_cap(sz);

    size_type i;
    try {
        for (i = 0; i < count; ++i) {
            emplace_back(value);
        }
    } catch (const std::exception& e) {
        for (size_type j = 0; j < i; ++j) {
            pop_back();
        }
        std::cerr << e.what() << '\n';
    }
}

template <typename T, typename Allocator>
template <typename InputIt>
deque<T, Allocator>::deque(InputIt first, InputIt last, const Allocator& alloc)
    : deque(first, last, alloc, PMapAlloc()) {}

template <typename T, typename Allocator>
template <typename InputIt>
deque<T, Allocator>::deque(InputIt first, InputIt last, const Allocator& alloc, const PMapAlloc& pmap_alloc)
    : alloc_(alloc), buffer_size_(deque_buffer_sz(sizeof(T))), pmap_alloc_(pmap_alloc) {
    size_type el_cnt = last - first;
    size_type nodes_cnt = el_cnt / buffer_size_ + el_cnt % buffer_size_ + 2;

    default_constr_with_memory_cap(nodes_cnt);
    size_type i = 0;
    try {
        for (auto it = first; it != last; ++i, ++it) {
            emplace_back(*it);
        }
    } catch (const std::exception& e) {
        for (size_type j = 0; j < i; ++j) {
            pop_back();
        }
        std::cerr << e.what() << '\n';
    }
}

template <typename T, typename Allocator>
deque<T, Allocator>::deque(const deque& other)
    : deque(other.begin(), other.end(), alloc_traits::select_on_container_copy_construction(other.alloc_),
            pmap_alloc_traits::select_on_container_copy_construction(other.pmap_alloc_))

{}

template <typename T, typename Allocator>
deque<T, Allocator>::deque(deque&& other)
    : alloc_(std::move_if_noexcept(other.alloc_)),
      pmap_alloc_(std::move_if_noexcept(other.pmap_alloc_)),

      start_node_(std::exchange(other.start_node_, nullptr)),
      finish_node_(std::exchange(other.finish_node_, nullptr)),
      curr_begin_node_(std::exchange(other.curr_begin_node_, nullptr)),
      curr_end_node_(std::exchange(other.curr_end_node_, nullptr)),
      begin_ind_(std::exchange(other.begin_ind_, 0)),
      end_ind_(std::exchange(other.end_ind_, 0)),
      buffer_size_(other.buffer_size_)

{
    other.default_constr_with_memory_cap(0);
}

#if __cplusplus >= 202002L

template <typename T, typename Allocator>
deque<T, Allocator>::deque(const deque& other, const std::type_identity_t<Allocator>& alloc)
    : deque(other.begin(), other.end(), alloc,
            typename std::allocator_traits<std::type_identity_t<Allocator>>::rebind_alloc<PMapAlloc>()) {}

template <typename T, typename Allocator>
deque<T, Allocator>::deque(deque&& other, const std::type_identity_t<Allocator>& alloc)
    : alloc_(alloc),
      pmap_alloc_(typename std::allocator_traits<std::type_identity_t<Allocator>>::rebind_alloc<PMapAlloc>()),
      buffer_size_(other.buffer_size_) {
    if (other.get_allocator() == alloc_) {
        start_node_ = other.start_node_;
        finish_node_ = other.finish_node_;
        curr_begin_node_ = other.curr_begin_node_;
        curr_end_node_ = other.curr_end_node_;
        begin_ind_ = other.begin_ind_;
        end_ind_ = other.end_ind_;
    } else {
        size_type el_cnt = other.begin() - other.end();
        size_type nodes_cnt = el_cnt / buffer_size_ + el_cnt % buffer_size_ + 2;

        default_constr_with_memory_cap(nodes_cnt);
        size_type i = 0;
        try {
            for (auto it = other.begin(); it != other.end(); ++i, ++it) {
                emplace_back(std::move_if_noexcept(*it));
            }
        } catch (const std::exception& e) {
            for (size_type j = 0; j < i; ++j) {
                pop_back();
            }
            std::cerr << e.what() << '\n';
        }
    }

    other.start_node_ = nullptr;
    other.finish_node_ = nullptr;
    other.curr_begin_node_ = nullptr;
    other.curr_end_node_ = nullptr;
    other.begin_ind_ = 0;
    other.end_ind_ = 0;
    other.default_constr_with_memory_cap(0);
}

#endif

template <typename T, typename Allocator>
deque<T, Allocator>::deque(std::initializer_list<value_type> init, const Allocator& alloc)
    : deque(init.begin(), init.end(), alloc) {}

template <typename T, typename Allocator>
template <typename Tp>
deque<T, Allocator>::Iterator<Tp>::Iterator(el_pointer start, el_pointer curr, el_pointer finish, map_pointer curr_node)
    : start_el_(start),
      finish_el_(finish),
      curr_el_(curr),
      curr_node_(curr_node)

{}

template <typename T, typename Allocator>
template <typename Tp>
deque<T, Allocator>::Iterator<Tp>::Iterator(const Iterator& other)
    : start_el_(other.start_el_),
      finish_el_(other.finish_el_),
      curr_el_(other.curr_el_),
      curr_node_(other.curr_node_) {}

template <typename T, typename Allocator>
template <typename Tp>
deque<T, Allocator>::Iterator<Tp> deque<T, Allocator>::Iterator<Tp>::operator=(const Iterator& other) {
    start_el_ = other.start_el_;
    finish_el_ = other.finish_el_;
    curr_el_ = other.curr_el_;
    curr_node_ = other.curr_node_;
    return *this;
}

template <typename T, typename Allocator>
template <typename Tp>
deque<T, Allocator>::Iterator<Tp>::Iterator(Iterator&& other)
    : start_el_(std::exchange(other.start_el_, nullptr)),
      finish_el_(std::exchange(other.finish_el_, nullptr)),
      curr_el_(std::exchange(other.curr_el_, nullptr)),
      curr_node_(std::exchange(other.curr_node_, nullptr)) {}

template <typename T, typename Allocator>
template <typename Tp>
template <typename U>
deque<T, Allocator>::Iterator<Tp>::Iterator(const Iterator<U>& other)
    : start_el_(const_cast<el_pointer>(other.start_el_)),
      curr_el_(const_cast<el_pointer>(other.curr_el_)),
      finish_el_(const_cast<el_pointer>(other.finish_el_)),
      curr_node_(const_cast<map_pointer>(other.curr_node_)) {}

template <typename T, typename Allocator>
template <typename Tp>
deque<T, Allocator>::Iterator<Tp> deque<T, Allocator>::Iterator<Tp>::operator=(Iterator&& other) {
    start_el_ = std::exchange(other.start_el_, nullptr);
    finish_el_ = std::exchange(other.finish_el_, nullptr);
    curr_el_ = std::exchange(other.curr_el_, nullptr);
    curr_node_ = std::exchange(other.curr_node_, nullptr);
    return *this;
}

template <typename T, typename Allocator>
deque<T, Allocator>::~deque() {
    size_type front_cap = (curr_begin_node_ - start_node_) * buffer_size_ + begin_ind_;
    size_type back_cap = (finish_node_ - curr_end_node_) * buffer_size_ + (buffer_size_ - end_ind_);
    size_type mem_sz = size() + front_cap + back_cap;
    clear();
    pmap_alloc_traits::deallocate(pmap_alloc_, start_node_, mem_sz);
}

template <typename T, typename Allocator>
template <typename Tp>
deque<T, Allocator>::Iterator<Tp>::reference deque<T, Allocator>::Iterator<Tp>::Iterator::operator++() {
    ++curr_el_;
    if (curr_el_ == finish_el_) {
        auto sz = finish_el_ - start_el_;
        ++curr_node_;
        start_el_ = curr_el_ = *curr_node_;
        finish_el_ = start_el_ + sz;
    }
    return *curr_el_;
}

template <typename T, typename Allocator>
template <typename Tp>
deque<T, Allocator>::Iterator<Tp>::value_type deque<T, Allocator>::Iterator<Tp>::Iterator::operator++(int) {
    auto tmp_copy = *curr_el_;
    ++curr_el_;
    if (curr_el_ == finish_el_) {
        auto sz = finish_el_ - start_el_;
        ++curr_node_;
        start_el_ = curr_el_ = *curr_node_;
        finish_el_ = start_el_ + sz;
    }
    return tmp_copy;
}

template <typename T, typename Allocator>
template <typename Tp>
deque<T, Allocator>::Iterator<Tp>::reference deque<T, Allocator>::Iterator<Tp>::Iterator::operator--() {
    if (curr_el_ == start_el_) {
        auto sz = finish_el_ - start_el_;
        --curr_node_;
        start_el_ = *curr_node_;
        finish_el_ = *curr_node_ + sz;
        curr_el_ = *curr_node_ + sz - 1;
    } else {
        --curr_el_;
    }
    return *curr_el_;
}

template <typename T, typename Allocator>
template <typename Tp>
deque<T, Allocator>::Iterator<Tp>::value_type deque<T, Allocator>::Iterator<Tp>::Iterator::operator--(int) {
    auto tmp_copy = *curr_el_;
    if (curr_el_ == start_el_) {
        auto sz = finish_el_ - start_el_;
        --curr_node_;
        start_el_ = *curr_node_;
        finish_el_ = *curr_node_ + sz;
        curr_el_ = *curr_node_ + sz - 1;
    } else {
        --curr_el_;
    }
    return tmp_copy;
}

template <typename T, typename Allocator>
template <typename Tp>
deque<T, Allocator>::Iterator<Tp>::reference deque<T, Allocator>::Iterator<Tp>::Iterator::operator*() {
    return *curr_el_;
}

template <typename T, typename Allocator>
template <typename Tp>
deque<T, Allocator>::Iterator<Tp>::const_reference deque<T, Allocator>::Iterator<Tp>::Iterator::operator*() const {
    return *curr_el_;
}

template <typename T, typename Allocator>
template <typename Tp>
deque<T, Allocator>::Iterator<Tp>::pointer deque<T, Allocator>::Iterator<Tp>::Iterator::operator->() {
    return curr_el_;
}

template <typename T, typename Allocator>
template <typename Tp>
deque<T, Allocator>::Iterator<Tp>::const_pointer deque<T, Allocator>::Iterator<Tp>::Iterator::operator->() const {
    return curr_el_;
}

template <typename T, typename Allocator>
template <typename Tp>
deque<T, Allocator>::Iterator<Tp> deque<T, Allocator>::Iterator<Tp>::Iterator::operator+(difference_type n) {
    difference_type fc_diff = finish_el_ - curr_el_;
    Iterator ret_it = *this;
    if (fc_diff > n) {
        ret_it.curr_el_ += n;
        return ret_it;
    }
    size_type sz = ret_it.finish_el_ - ret_it.start_el_;
    if (sz == 0) {
        throw std::runtime_error("Invalid iterator finish-start borders");
    }
    n -= fc_diff;
    ++(ret_it.curr_node_);

    ret_it.curr_node_ += n / sz;

    ret_it.start_el_ = *(ret_it.curr_node_);
    ret_it.curr_el_ = ret_it.start_el_ + n % sz;
    ret_it.finish_el_ = ret_it.start_el_ + sz;

    return *this;
}

template <typename T, typename Allocator>
template <typename Tp>
deque<T, Allocator>::Iterator<Tp> deque<T, Allocator>::Iterator<Tp>::Iterator::operator-(difference_type n) {
    difference_type cs_diff = curr_el_ - start_el_;
    Iterator ret_it = *this;
    if (cs_diff >= n) {
        ret_it.curr_el_ -= n;
        return ret_it;
    }
    size_type sz = ret_it.finish_el_ - ret_it.start_el_;
    if (sz == 0) {
        throw std::runtime_error("Invalid iterator finish-start borders");
    }
    n -= cs_diff + 1;
    --(ret_it.curr_node_);
    ret_it.curr_node_ -= n / sz;
    ret_it.start_el_ = *(ret_it.curr_node_);
    ret_it.finish_el_ = ret_it.start_el_ + sz;
    ret_it.curr_el_ = ret_it.finish_el_ - 1 - n % sz;
    return ret_it;
}

template <typename T, typename Allocator>
template <typename Tp>
template <typename U>
deque<T, Allocator>::Iterator<Tp>::difference_type deque<T, Allocator>::Iterator<Tp>::Iterator::operator-(
    const Iterator<U>& it) const {
    if (curr_node_ == it.curr_node_) {
        return curr_el_ - it.curr_el_;
    }
    difference_type full_nodes = curr_node_ - const_cast<map_pointer>(it.curr_node_) - 1;
    size_type sz = finish_el_ - start_el_;
    return full_nodes * sz + (curr_el_ - start_el_) + (it.finish_el_ - it.curr_el_);
}

template <typename T, typename Allocator>
template <typename Tp>
bool deque<T, Allocator>::Iterator<Tp>::Iterator::operator==(const Iterator& other) const {
    return start_el_ == other.start_el_ && curr_el_ == other.curr_el_ && finish_el_ == other.finish_el_ &&
           curr_node_ == other.curr_node_;
}

template <typename T, typename Allocator>
template <typename Tp>
bool deque<T, Allocator>::Iterator<Tp>::Iterator::operator!=(const Iterator& other) const {
    return !(*this == other);
}

template <typename T, typename Allocator>
deque<T, Allocator>::reference deque<T, Allocator>::front() {
    return *(*curr_begin_node_ + begin_ind_);
}

template <typename T, typename Allocator>
deque<T, Allocator>::const_reference deque<T, Allocator>::front() const {
    return *(*curr_begin_node_ + begin_ind_);
}

template <typename T, typename Allocator>
deque<T, Allocator>::reference deque<T, Allocator>::back() {
    return *(*curr_end_node_ + end_ind_ - 1);
}

template <typename T, typename Allocator>
deque<T, Allocator>::const_reference deque<T, Allocator>::back() const {
    return *(*curr_end_node_ + end_ind_ - 1);
}

template <typename T, typename Allocator>
deque<T, Allocator>::iterator deque<T, Allocator>::begin() {
    return iterator(*curr_begin_node_, *curr_begin_node_ + begin_ind_, *curr_begin_node_ + buffer_size_,
                    curr_begin_node_);
}

template <typename T, typename Allocator>
deque<T, Allocator>::const_iterator deque<T, Allocator>::begin() const {
    return const_iterator(const_cast<const T*>(*curr_begin_node_), const_cast<const T*>(*curr_begin_node_ + begin_ind_),
                          const_cast<const T*>(*curr_begin_node_ + buffer_size_),
                          const_cast<const T**>(curr_begin_node_));
}

template <typename T, typename Allocator>
deque<T, Allocator>::const_iterator deque<T, Allocator>::cbegin() const noexcept {
    return const_iterator(const_cast<const T*>(*curr_begin_node_), const_cast<const T*>(*curr_begin_node_ + begin_ind_),
                          const_cast<const T*>(*curr_begin_node_ + buffer_size_),
                          const_cast<const T**>(curr_begin_node_));
}

template <typename T, typename Allocator>
deque<T, Allocator>::iterator deque<T, Allocator>::end() {
    return iterator(*curr_end_node_, *curr_end_node_ + end_ind_, *curr_end_node_ + buffer_size_, curr_end_node_);
}

template <typename T, typename Allocator>
deque<T, Allocator>::const_iterator deque<T, Allocator>::end() const {
    return const_iterator(const_cast<const T*>(*curr_end_node_), const_cast<const T*>(*curr_end_node_ + end_ind_),
                          const_cast<const T*>(*curr_end_node_ + buffer_size_), const_cast<const T**>(curr_end_node_));
}

template <typename T, typename Allocator>
deque<T, Allocator>::const_iterator deque<T, Allocator>::cend() const noexcept {
    return const_iterator(const_cast<const T*>(*curr_end_node_), const_cast<const T*>(*curr_end_node_ + end_ind_),
                          const_cast<const T*>(*curr_end_node_ + buffer_size_), const_cast<const T**>(curr_end_node_));
}

template <typename T, typename Allocator>
deque<T, Allocator>::reverse_iterator deque<T, Allocator>::rbegin() {
    return end();
}

template <typename T, typename Allocator>
deque<T, Allocator>::const_reverse_iterator deque<T, Allocator>::rbegin() const {
    return end();
}

template <typename T, typename Allocator>
deque<T, Allocator>::const_reverse_iterator deque<T, Allocator>::crbegin() const noexcept {
    return cend();
}

template <typename T, typename Allocator>
deque<T, Allocator>::reverse_iterator deque<T, Allocator>::rend() {
    return begin();
}

template <typename T, typename Allocator>
deque<T, Allocator>::const_reverse_iterator deque<T, Allocator>::rend() const {
    return begin();
}

template <typename T, typename Allocator>
deque<T, Allocator>::const_reverse_iterator deque<T, Allocator>::crend() const noexcept {
    return cbegin();
}

template <typename T, typename Allocator>
size_t deque<T, Allocator>::deque_buffer_sz(size_t sz) {
    return (sz > deque_buffer_size) ? size_t(1) : (deque_buffer_size / sz);
}

template <typename T, typename Allocator>
void deque<T, Allocator>::reallocate_pointers_map(size_type offset_borders) {
    size_type sz = size();
    if (offset_borders == 0) {
        offset_borders = sz / 2 + sz % 2;
    }
    auto new_start = pmap_alloc_traits::allocate(pmap_alloc_, sz + 2 * offset_borders);
    auto new_begin = new_start + offset_borders;
    auto node = new_begin;
    auto new_node = new_begin;
    try {
        for (; node != curr_end_node_; ++node, ++new_node) {
            pmap_alloc_traits::construct(pmap_alloc_, new_node, *node);
        }
    } catch (const std::exception& e) {
        for (; node != new_begin - 1; --node, --new_node) {
            pmap_alloc_traits::destroy(pmap_alloc_, new_node);
        }
    }

    pmap_alloc_traits::deallocate(pmap_alloc_, start_node_, finish_node_ - start_node_);

    curr_begin_node_ = new_begin;
    curr_end_node_ = curr_begin_node_ + sz;
    finish_node_ = new_start + sz + 2 * offset_borders;
    start_node_ = new_start;
}

template <typename T, typename Allocator>
void deque<T, Allocator>::push_back(const deque<T, Allocator>::value_type& value) {
    emplace_back(value);
}

template <typename T, typename Allocator>
void deque<T, Allocator>::push_back(deque<T, Allocator>::value_type&& value) {
    emplace_back(std::forward<value_type>(value));
}

template <typename T, typename Allocator>
template <class... Args>
deque<T, Allocator>::reference deque<T, Allocator>::emplace_back(Args&&... args) {
    if (curr_end_node_ == finish_node_ && end_ind_ == buffer_size_) {  // reallocate pointers map
        reallocate_pointers_map();
    }
    if (end_ind_ == buffer_size_) {
        auto val = alloc_traits::allocate(alloc_, buffer_size_);
        try {
            pmap_alloc_traits::construct(pmap_alloc_, curr_end_node_ + 1, val);
        } catch (const std::exception& e) {
            alloc_traits::deallocate(alloc_, val, buffer_size_);
            throw e;
        }
        end_ind_ = 0;
        ++curr_end_node_;
    }
    alloc_traits::construct(alloc_, *curr_end_node_ + end_ind_, std::forward<Args>(args)...);
    ++end_ind_;
    return *(*curr_end_node_ + end_ind_ - 1);
}

template <typename T, typename Allocator>
void deque<T, Allocator>::push_front(const deque<T, Allocator>::value_type& value) {
    emplace_front(value);
}

template <typename T, typename Allocator>
void deque<T, Allocator>::push_front(deque<T, Allocator>::value_type&& value) {
    emplace_front(std::forward<value_type>(value));
}

template <typename T, typename Allocator>
template <class... Args>
deque<T, Allocator>::reference deque<T, Allocator>::emplace_front(Args&&... args) {
    if (curr_begin_node_ == start_node_ && begin_ind_ == 0) {  // reallocate pointers map
        reallocate_pointers_map();
    }
    if (begin_ind_ == 0) {
        auto val = alloc_traits::allocate(alloc_, buffer_size_);
        try {
            pmap_alloc_traits::construct(pmap_alloc_, curr_begin_node_ - 1, val);
        } catch (const std::exception& e) {
            alloc_traits::deallocate(alloc_, val, buffer_size_);
            throw e;
        }

        begin_ind_ = buffer_size_;
        --curr_begin_node_;
    }
    --begin_ind_;
    alloc_traits::construct(alloc_, *curr_begin_node_ + begin_ind_, std::forward<Args>(args)...);
    return *(*curr_begin_node_ + begin_ind_);
}

template <typename T, typename Allocator>
bool deque<T, Allocator>::empty() const {
    if (curr_begin_node_ == curr_end_node_ && begin_ind_ == end_ind_) {
        return true;
    }
    if ((curr_end_node_ == curr_begin_node_ - 1) && (begin_ind_ == 0) && (end_ind_ == buffer_size_)) {
        return true;
    }
    return false;
}

template <typename T, typename Allocator>
deque<T, Allocator>::size_type deque<T, Allocator>::size() const {
    return end() - begin();
}

template <typename T, typename Allocator>
void deque<T, Allocator>::shrink_to_fit() {
    difference_type front_cap = (curr_begin_node_ - start_node_) * buffer_size_ + begin_ind_;
    difference_type back_cap = (finish_node_ - curr_end_node_) * buffer_size_ + (buffer_size_ - end_ind_);
    if ((front_cap + back_cap) < buffer_size_) {
        return;
    }
    shrink_to_fit_nodes();
}

template <typename T, typename Allocator>
void deque<T, Allocator>::shrink_to_fit_nodes() {
    size_type new_nodes_cnt = size() / buffer_size_ + (size() % buffer_size_ == 0 ? 0 : 1);
    map_pointer new_start = pmap_alloc_traits::allocate(pmap_alloc_, new_nodes_cnt);
    size_type i;
    try {
        // выделение памяти для каждой ноды
        for (i = 0; i < new_nodes_cnt; ++i) {
            auto chunk = alloc_traits::allocate(alloc_, buffer_size_);
            pmap_alloc_traits::construct(pmap_alloc_, new_start + i, chunk);
        }

        move_nodes(new_start, 0, curr_begin_node_, begin_ind_, new_nodes_cnt);

    } catch (const std::exception& e) {
        for (size_type j = 0; j < i; ++j) {
            alloc_traits::deallocate(alloc_, *(new_start + i), buffer_size_);
            pmap_alloc_traits::destroy(pmap_alloc_, new_start + i);
        }
        std::cerr << e.what() << '\n';
    }
}

template <typename T, typename Allocator>
void deque<T, Allocator>::move_nodes(map_pointer new_begin, size_type new_begin_ind, map_pointer old_begin,
                                     size_type old_begin_ind, size_type cnt) {
    // функция мувает cnt элементов с ноды old_begin с индекса old_begin_ind в ноду new_begin с индекса new_begin_ind
    // память в *new_begin и далее (cnt ячекк) должна быть только саллоцированной (сырой)
    map_pointer new_node = new_begin;
    map_pointer old_node = old_begin;

    size_type new_ind = old_begin_ind % buffer_size_;
    size_type old_ind = old_begin_ind % buffer_size_;

    size_type i;

    try {
        for (i = 0; i < cnt; ++i) {
            alloc_traits::construct(alloc_, *new_node + new_ind, std::move(*(*old_node) + old_ind));

            if (++new_ind == buffer_size_) {
                new_ind = 0;
                ++new_node;
            }
            if (++old_ind == buffer_size_) {
                old_ind = 0;
                ++old_node;
            }
        }

    } catch (const std::exception& e) {
        for (size_type j = 0; j < i; ++j) {
            if constexpr (std::is_nothrow_move_constructible_v<value_type>) {  // значение мувалось без исключений,
                                                                               // исключение вызвано методом construct
                if (std::is_nothrow_move_assignable_v<value_type>) {
                    *(*old_node + old_ind) = std::move(*(*new_node + new_ind));
                }
            }
            alloc_traits::destroy(alloc_, *new_node + new_ind);

            if (new_ind == 0) {
                new_ind = buffer_size_;
                --new_node;
            }
            if (old_ind == 0) {
                old_ind = buffer_size_;
                --old_node;
            }
            --new_ind;
            --old_ind;
        }

        throw e;
    }
}

template <typename T, typename Allocator>
void deque<T, Allocator>::destroy_node(map_pointer node, size_type first_ind, size_type last_ind) {
    if (first_ind == last_ind) return;
    for (size_type i = first_ind; i != last_ind; ++i) {
        alloc_traits::destroy(alloc_, *node + i);
    }
    alloc_traits::deallocate(alloc_, *node, last_ind - first_ind);
    pmap_alloc_traits::destroy(pmap_alloc_, node);
}

template <typename T, typename Allocator>
void deque<T, Allocator>::clear() {
    if (empty()) return;
    if (curr_begin_node_ == curr_end_node_) {
        destroy_node(curr_begin_node_, begin_ind_, end_ind_);
        return;
    }
    destroy_node(curr_begin_node_, begin_ind_, buffer_size_);
    destroy_node(curr_end_node_, 0, end_ind_);

    for (map_pointer p = curr_begin_node_ + 1; p != curr_end_node_; ++p) {
        destroy_node(p, 0, buffer_size_);
    }
    curr_begin_node_ = start_node_;
    curr_end_node_ = start_node_;
    begin_ind_ = end_ind_ = 0;
}

template <typename T, typename Allocator>
deque<T, Allocator>::iterator deque<T, Allocator>::insert(const_iterator pos, const T& value) {
    if (pos == begin()) {
        emplace_front(value);
        return begin();
    }
    size_type front_diff = pos - begin();
    size_type back_diff = end() - pos;
    auto func = [&value]() { return value; };
    if (front_diff > back_diff) {
        return insert_front(pos, 1, std::ref(func));
    }
    return insert_back(pos, 1, std::ref(func));
}

template <typename T, typename Allocator>
deque<T, Allocator>::iterator deque<T, Allocator>::insert(const_iterator pos, T&& value) {
    if (pos == begin()) {
        emplace_front(std::move(value));
        return begin();
    }
    size_type front_diff = pos - begin();
    size_type back_diff = end() - pos;

    auto func = [&value]() mutable -> value_type&& { return std::move(value); };
    if (front_diff > back_diff) {
        return insert_front(pos, 1, std::ref(func));
    }
    return insert_back(pos, 1, std::ref(func));
}

template <typename T, typename Allocator>
template <class... Args>
deque<T, Allocator>::iterator deque<T, Allocator>::emplace(const_iterator pos, Args&&... args) {
    return insert(pos, (T(std::forward<Args>(args)...)));
}

template <typename T, typename Allocator>
deque<T, Allocator>::iterator deque<T, Allocator>::insert(const_iterator pos, size_type count, const T& value) {
    size_type front_diff = pos - begin();
    size_type back_diff = end() - pos;
    auto func = [&value]() { return value; };
    if (front_diff < back_diff) {
        return insert_front(pos, count, std::ref(func));
    }
    return insert_back(pos, count, std::ref(func));
}

template <typename T, typename Allocator>
template <class InputIt>
deque<T, Allocator>::iterator deque<T, Allocator>::insert(const_iterator pos, InputIt first, InputIt last) {
    size_type front_diff = pos - begin();
    size_type back_diff = end() - pos;

    deque<T, Allocator>::iterator ret_val;
    auto func = [first]() { return *first; };
    if (front_diff < back_diff) {
        for (; first != last; ++first) {
            ret_val = insert_front(pos, 1, std::ref(func));
        }
    } else {
        for (; first != last; ++first) {
            ret_val = insert_back(pos, 1, std::ref(func));
        }
    }
    return ret_val;
}

template <typename T, typename Allocator>
deque<T, Allocator>::iterator deque<T, Allocator>::insert(const_iterator pos, std::initializer_list<T> ilist) {
    size_type cnt = ilist.size();
    typename std::initializer_list<T>::iterator ibegin = ilist.begin();
    auto ibegin_ptr = &ibegin;

    auto func = [ibegin_ptr]() {
        auto ret_val = *(*ibegin_ptr);
        ++(*ibegin_ptr);
        return ret_val;
    };

    size_type front_diff = pos - begin();
    size_type back_diff = end() - pos;

    if (front_diff < back_diff) {
        return insert_front(pos, cnt, std::ref(func));
    }
    return insert_back(pos, cnt, std::ref(func));
}

template <typename T>
struct move_assign_if_noexcept_cond {
    static constexpr bool value = (std::is_nothrow_move_assignable_v<T> || !(std::is_copy_assignable_v<T>));
};

template <typename T>
std::conditional_t<move_assign_if_noexcept_cond<T>::value, T&&, const T&> move_assign_if_noexcept(T& val) {
    if constexpr (move_assign_if_noexcept_cond<T>::value) {
        return std::move(val);
    } else if constexpr (!(move_assign_if_noexcept_cond<T>::value)) {
        return val;
    }
}

template <typename T, typename Allocator>
template <typename Func>
deque<T, Allocator>::iterator deque<T, Allocator>::insert_front(const_iterator pos, size_type cnt, Func&& get_value) {
    // allocate memory
    difference_type front_cap = (curr_begin_node_ - start_node_) * buffer_size_ + begin_ind_;
    size_type pos_ind = pos - begin();

    if (front_cap < cnt) {
        reallocate_pointers_map(cnt / buffer_size_ + 1);
    }
    map_pointer tmp_begin_node = curr_begin_node_;
    size_type tmp_begin_ind = begin_ind_;

    // установка новой начальной ноды (со сдвигом в cnt)
    try {
        for (size_type i = 0; i < cnt / buffer_size_; ++i, --curr_begin_node_) {
            auto val = alloc_traits::allocate(alloc_, buffer_size_);
            pmap_alloc_traits::construct(pmap_alloc_, curr_begin_node_, val);
        }

        if (begin_ind_ < cnt % buffer_size_) {
            --curr_begin_node_;
            auto val = alloc_traits::allocate(alloc_, buffer_size_);
            pmap_alloc_traits::construct(pmap_alloc_, curr_begin_node_, val);

            begin_ind_ = begin_ind_ + buffer_size_ - cnt % buffer_size_;

        } else {
            begin_ind_ -= cnt % buffer_size_;
        }
    } catch (const std::exception& e) {
        for (; curr_begin_node_ != tmp_begin_node; ++curr_begin_node_) {
            alloc_traits::deallocate(alloc_, *curr_begin_node_, buffer_size_);
            pmap_alloc_traits::destroy(pmap_alloc_, curr_begin_node_);
        }
    }

    size_type i, j, k;
    map_pointer node = curr_begin_node_;
    size_type ind = begin_ind_;
    try {
        // 1 step : construct elements on the allocated memory
        for (i = 0; i < cnt && i < pos_ind; ++i) {
            alloc_traits::construct(alloc_, *node + ind, std::move_if_noexcept(this->operator[](i + cnt)));
            next_element(node, ind);
        }

        try {
            // 2 step : construct new elements on the allocated memory (cnt > pos_ind) or move old elements (cnt
            // <=pos_ind)

            if (i == cnt) {  // move elements
                for (j = cnt; j < pos_ind; ++j) {
                    this->operator[](j) = move_assign_if_noexcept(this->operator[](j + cnt));
                }
            } else {  // construct
                for (j = pos_ind; j < cnt; ++j) {
                    alloc_traits::construct(alloc_, *node + ind, std::forward<value_type>(get_value()));
                    next_element(node, ind);
                }
            }

            // 3 step : move new elements
            for (k = j; k < (pos_ind + cnt); ++k) {
                this->operator[](k) = std::forward<value_type>(get_value());
            }

        } catch (const std::exception& e) {
            if (i == cnt) {
                for (; j > 0; --j) {
                    this->operator[](j + cnt - 1) = move_assign_if_noexcept(this->operator[](j - 1));
                }
            } else {
                for (j = pos_ind; j < cnt; ++j) {
                    alloc_traits::destroy(alloc_, *node + ind);
                    prev_element(node, ind);
                }
            }
            throw e;
        }
    } catch (const std::exception& e) {
        for (; i > 0; --i) {
            this->operator[](i + cnt - 1) = move_assign_if_noexcept(*(*node + ind));
            alloc_traits::destroy(alloc_, *node + ind);
            prev_element(node, ind);
        }
        curr_begin_node_ = tmp_begin_node;
        begin_ind_ = tmp_begin_ind;
    }
    return begin() + pos_ind;
}

template <typename T, typename Allocator>
template <typename Func>
deque<T, Allocator>::iterator deque<T, Allocator>::insert_back(const_iterator pos, size_type cnt, Func&& get_value) {
    // allocate memory
    difference_type back_cap = (finish_node_ - curr_end_node_) * buffer_size_ + (buffer_size_ - end_ind_);
    size_type pos_ind = end() - pos;

    if (back_cap < cnt) {
        reallocate_pointers_map(cnt / buffer_size_ + 1);
    }

    map_pointer tmp_end_node = curr_end_node_;
    size_type tmp_end_ind = end_ind_;

    try {
        // установка новой конечной ноды (со сдвигом в cnt)

        for (size_type i = 0; i < (cnt / buffer_size_); ++i, ++curr_end_node_) {
            auto val = alloc_traits::allocate(alloc_, buffer_size_);
            pmap_alloc_traits::construct(pmap_alloc_, curr_end_node_ + 1, val);
        }

        if ((end_ind_ + cnt % buffer_size_) > buffer_size_) {
            ++curr_end_node_;
            auto val = alloc_traits::allocate(alloc_, buffer_size_);
            pmap_alloc_traits::construct(pmap_alloc_, curr_end_node_, val);
        }
        end_ind_ = (end_ind_ + cnt % buffer_size_) % buffer_size_;

    } catch (const std::exception& e) {
        for (; curr_end_node_ != tmp_end_node; --curr_end_node_) {
            alloc_traits::deallocate(alloc_, *curr_end_node_, buffer_size_);
            pmap_alloc_traits::destroy(pmap_alloc_, curr_end_node_);
        }
    }

    size_type i, j, k;
    map_pointer node = curr_end_node_;
    size_type ind = end_ind_;
    size_type sz = size();
    try {
        // 1 step : construct elements on the allocated memory
        for (i = 0; i < cnt && i < pos_ind; ++i) {
            alloc_traits::construct(alloc_, *node + ind, std::move_if_noexcept(this->operator[](sz - cnt - i - 1)));
            prev_element(node, ind);
        }

        try {
            // 2 step : construct new elements on the allocated memory (cnt > pos_ind) or move old elements
            // (cnt <=pos_ind)
            if (i == cnt) {  // move elements
                for (j = cnt; j < pos_ind; ++j) {
                    this->operator[](sz - j - 1) = move_assign_if_noexcept(this->operator[](sz - j - cnt - 1));
                }
                for (k = pos_ind + cnt; k > pos_ind; --k) {
                    std::cout << (sz - k) << std::endl;
                    this->operator[](sz - k) = std::forward<value_type>(get_value());
                }

            } else {  // construct
                for (k = pos_ind + cnt; k > cnt + 1; --k) {
                    this->operator[](sz - k) = std::forward<value_type>(get_value());
                }

                for (auto i = 0; i < (cnt - pos_ind); ++i) {
                    prev_element(node, ind);
                }

                for (j = pos_ind; j < cnt; ++j) {
                    alloc_traits::construct(alloc_, *node + ind, std::forward<value_type>(get_value()));
                    next_element(node, ind);
                }
            }

        } catch (const std::exception& e) {
            if (i == cnt) {
                for (; j > 0; --j) {
                    this->operator[](sz - (j + cnt - 1)) = move_assign_if_noexcept(this->operator[](sz - (j - 1)));
                }
            } else {
                for (j = pos_ind; j < cnt; ++j) {
                    alloc_traits::destroy(alloc_, *node + ind);
                    next_element(node, ind);
                }
            }
            throw e;
        }
    } catch (const std::exception& e) {
        for (; i > 0; --i) {
            this->operator[](sz - (i + cnt - 1)) = move_assign_if_noexcept(*(*node + ind));
            alloc_traits::destroy(alloc_, *node + ind);
            next_element(node, ind);
        }
        curr_end_node_ = tmp_end_node;
        end_ind_ = tmp_end_ind;
    }
    return end() - pos_ind;
}

template <typename T, typename Allocator>
void deque<T, Allocator>::next_element(map_pointer& node, size_type& ind) {
    if (ind == buffer_size_ - 1) {
        ++node;
        ind = 0;
    } else {
        ++ind;
    }
}

template <typename T, typename Allocator>
void deque<T, Allocator>::prev_element(map_pointer& node, size_type& ind) {
    if (ind == 0) {
        --node;
        ind = buffer_size_ - 1;
    } else {
        --ind;
    }
}

template <typename T, typename Allocator>
deque<T, Allocator>::allocator_type deque<T, Allocator>::get_allocator() const {
    return alloc_;
}

template <typename T, typename Allocator>
deque<T, Allocator>::PMapAlloc deque<T, Allocator>::get_pmap_allocator() const {
    return pmap_alloc_;
}

template <typename T, typename Allocator>
deque<T, Allocator>::reference deque<T, Allocator>::operator[](size_type pos) {
    map_pointer ret_node = curr_begin_node_;
    size_type ret_ind = begin_ind_;

    ret_node += pos / buffer_size_;
    ret_ind += pos % buffer_size_;
    if (ret_ind >= buffer_size_) {
        ++ret_node;
        ret_ind %= buffer_size_;
    }

    return *(*(ret_node) + ret_ind);
}

template <typename T, typename Allocator>
deque<T, Allocator>::reference deque<T, Allocator>::at(size_type pos) {
    if (pos >= size()) {
        throw std::out_of_range("The size of container is smaller than the numbers in the function argument");
    }
    return this->operator[](pos);
}

template <typename T, typename Allocator>
deque<T, Allocator>::const_reference deque<T, Allocator>::at(size_type pos) const {
    if (pos >= size()) {
        throw std::out_of_range("The size of container is smaller than the numbers in the function argument");
    }
    return this->operator[](pos);
}

template <typename T, typename Allocator>
void deque<T, Allocator>::pop_back() {
    alloc_traits::destroy(alloc_, *curr_end_node_ + end_ind_ - 1);
    if (end_ind_ == 1) {
        alloc_traits::deallocate(alloc_, *curr_end_node_, buffer_size_);
    }
    prev_element(curr_end_node_, end_ind_);
}

template <typename T, typename Allocator>
void deque<T, Allocator>::pop_front() {
    alloc_traits::destroy(alloc_, *curr_begin_node_ + begin_ind_);
    if (begin_ind_ == buffer_size_ - 1) {
        alloc_traits::deallocate(alloc_, *curr_begin_node_, buffer_size_);
    }
    next_element(curr_begin_node_, begin_ind_);
}

template <typename T, typename Allocator>
void deque<T, Allocator>::resize(size_type count) {
    resize_templ(count);
}

template <typename T, typename Allocator>
void deque<T, Allocator>::resize(size_type count, const value_type& value) {
    resize_templ(count, value);
}

template <typename T, typename Allocator>
template <typename... Args>
void deque<T, Allocator>::resize_templ(size_type cnt, Args... args) {
    size_type sz = size();
    if (cnt > sz) {
        for (size_type i = 0; i < cnt - sz; ++i) {
            emplace_back(std::forward<Args>(args)...);
        }
    } else {
        for (size_type i = 0; i < sz - cnt; ++i) {
            pop_back();
        }
    }
}

template <typename T, typename Allocator>
void deque<T, Allocator>::swap(deque& other) noexcept(std::allocator_traits<Allocator>::is_always_equal::value) {
    if (alloc_ == other.get_allocator()) {
        if (alloc_traits::propagate_on_container_swap::value) {
            std::swap(alloc_, other.alloc_);
        }
        std::swap(start_node_, other.start_node_);
        std::swap(finish_node_, other.finish_node_);
        std::swap(curr_begin_node_, other.curr_begin_node_);
        std::swap(curr_end_node_, other.curr_end_node_);
        std::swap(begin_ind_, other.begin_ind_);
        std::swap(end_ind_, other.end_ind_);
        std::swap(buffer_size_, other.buffer_size_);
    }
}

template <typename T, typename Allocator>
void deque<T, Allocator>::assign(size_type count, const T& value) {
    size_type sz = size();

    deque<T, Allocator> tmp_deque(get_allocator());
    size_type i;
    try {
        for (i = 0; i < count; ++i) {
            if (i < sz) {
                tmp_deque.emplace_back(value);
                this->operator[](i) = value;
            } else {
                emplace_back(value);
            }
        }
        for (; i < sz; ++i) {
            pop_back();
        }
    }

    catch (const std::exception& e) {
        for (; i < sz; ++i) {
            tmp_deque.emplace_back(std::move_if_noexcept(this->operator[](i)));
        }
        *this = tmp_deque;
    }
}

template <typename T, typename Allocator>
template <class InputIt>
void deque<T, Allocator>::assign(InputIt first, InputIt last) {
    size_type sz = size();

    deque<T, Allocator> tmp_deque(get_allocator());
    size_type i;
    try {
        for (i = 0; first != last; ++first, ++i) {
            if (i < sz) {
                tmp_deque.emplace_back(std::move_if_noexcept(this->operator[](i)));
                this->operator[](i) = (*first);
            } else {
                emplace_back(*first);
            }
        }
        for (; i < sz; ++i) {
            pop_back();
        }
    }

    catch (const std::exception& e) {
        for (; i < sz; ++i) {
            tmp_deque.emplace_back(std::move_if_noexcept(this->operator[](i)));
        }
        *this = tmp_deque;
    }
}

template <typename T, typename Allocator>
void deque<T, Allocator>::assign(std::initializer_list<T> ilist) {
    size_type capacity = size() + (finish_node_ - curr_end_node_ - 1) * buffer_size_ + (buffer_size_ - end_ind_);
    if (capacity < ilist.size()) {
        size_type offset_sz = (ilist.size() - capacity) / buffer_size_ + 1;
        reallocate_pointers_map(offset_sz);
    }
    assign(ilist.begin(), ilist.end());
}

template <typename T, typename Allocator>
deque<T, Allocator>& deque<T, Allocator>::operator=(std::initializer_list<value_type> init) {
    assign(init);
    return *this;
}

template <typename T, typename Allocator>
deque<T, Allocator>& deque<T, Allocator>::operator=(deque&& other) noexcept(
    std::allocator_traits<Allocator>::is_always_equal::value) {
    if (alloc_ != other.get_allocator()) {  // move-assign each element individually
        move_assign_each_element_individually(std::forward<deque>(other));
        return *this;
    }
    if (alloc_traits::propagate_on_container_move_assignment::value) {
        alloc_ = other.get_allocator();
    }
    if (pmap_alloc_traits::propagate_on_container_move_assignment::value) {
        pmap_alloc_ = other.get_pmap_allocator();
    }

    start_node_ = other.start_node_;
    finish_node_ = other.finish_node_;
    curr_begin_node_ = other.curr_begin_node_;
    curr_end_node_ = other.curr_end_node_;
    begin_ind_ = other.begin_ind_;
    end_ind_ = other.end_ind_;
    other.default_constr_with_memory_cap(0);

    return *this;
}

template <typename T, typename Allocator>
void deque<T, Allocator>::move_assign_each_element_individually(deque&& other) {
    assign(std::move_iterator<iterator>(other.begin()), std::move_iterator<iterator>(other.end()));
    other.clear();
}

template <typename T, typename Allocator>
void deque<T, Allocator>::copy_assign_each_element_individually(const deque& other) {
    assign(other.begin(), other.end());
}

template <typename T, typename Allocator>
deque<T, Allocator>& deque<T, Allocator>::operator=(const deque& other) {
    if (alloc_traits::propagate_on_container_copy_assignment::value) {
        alloc_ = other.get_allocator();
    }
    if (pmap_alloc_traits::propagate_on_container_copy_assignment::value) {
        pmap_alloc_ = other.get_pmap_allocator();
    }
    copy_assign_each_element_individually(other);
    return *this;
}

template <typename T, typename Allocator>
deque<T, Allocator>::iterator deque<T, Allocator>::erase(const_iterator pos) {
    return erase(pos, pos + 1);
}

template <typename T, typename Allocator>
deque<T, Allocator>::iterator deque<T, Allocator>::erase(const_iterator first, const_iterator last) {
    difference_type front_diff = first - begin();
    difference_type back_diff = end() - last;

    if (front_diff < back_diff) {
        return erase_front(first, last);
    } else {
        return erase_back(first, last);
    }
}

template <typename T, typename Allocator>
deque<T, Allocator>::iterator deque<T, Allocator>::erase_back(const_iterator first, const_iterator last) {
    if (first == last) return last;
    difference_type back_diff = end() - last;
    difference_type diff = last - first;
    iterator it1 = first;
    iterator it2 = last;
    try {
        for (; it2 != end(); ++it2, ++it1) {
            swap_elemets(*it1, *it2);
        }
        for (auto i = 0; i < diff; ++i) {
            pop_back();
        }
    } catch (const std::exception& e) {
        for (; it2 != last; --it2, --it1) {
            swap_elemets(*it1, *it2);
        }
    }
    return end() - back_diff;
}

template <typename T, typename Allocator>
deque<T, Allocator>::iterator deque<T, Allocator>::erase_front(const_iterator first, const_iterator last) {
    if (first == last) return last;
    difference_type front_diff = first - begin();
    difference_type diff = last - first;
    iterator it1 = first - 1;
    iterator it2 = last - 1;
    size_type i;
    try {
        for (i = 0; i < front_diff; ++i, --it2, --it1) {
            swap_elemets(*it1, *it2);
        }
        for (auto j = 0; j < diff; ++j) {
            pop_front();
        }
    } catch (const std::exception& e) {
        for (; i > 0; --it2, --it1) {
            swap_elemets(*it1, *it2);
        }
    }
    return begin() + front_diff;
}

template <typename T, typename Allocator>
void deque<T, Allocator>::swap_elemets(value_type& first, value_type& second) {
    if constexpr (std::is_nothrow_swappable_v<value_type>) {
        std::swap(first, second);
    } else {
        value_type tmp = move_assign_if_noexcept(first);
        first = move_assign_if_noexcept(second);
        second = move_assign_if_noexcept(tmp);
    }
}

template <class T, class Alloc>
void swap(deque<T, Alloc>& lhs, deque<T, Alloc>& rhs) noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
}

template <class T, class Alloc>
bool operator==(const deque<T, Alloc>& lhs, const deque<T, Alloc>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (typename deque<T, Alloc>::size_type i = 0; i < lhs.size(); ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

#if __cplusplus >= 202602L
template <class T, class Alloc, class U = T>
constexpr typename deque<T, Alloc>::size_type erase(deque<T, Alloc>& c, const U& value) {
    auto it = std::remove(c.begin(), c.end(), value);
    typename deque<T, Alloc>::size_type r = c.end() - it;
    c.erase(it, c.end());
    return r;
}
#else
template <class T, class Alloc, class U>
typename deque<T, Alloc>::size_type erase(deque<T, Alloc>& c, const U& value) {
    auto it = std::remove(c.begin(), c.end(), value);
    typename deque<T, Alloc>::size_type r = c.end() - it;
    c.erase(it, c.end());
    return r;
}
#endif

template <class T, class Alloc, class Pred>
typename deque<T, Alloc>::size_type erase_if(deque<T, Alloc>& c, Pred pred) {
    auto it = std::remove_if(c.begin(), c.end(), pred);
    typename deque<T, Alloc>::size_type r = c.end() - it;
    c.erase(it, c.end());
    return r;
}

template <typename U>
concept boolean_testable = std::convertible_to<U, bool> && requires(U&& t) {
    { !static_cast<U &&>(t) } -> std::convertible_to<bool>;
};

constexpr auto synth_three_way = []<class T, class U>(const T& t, const U& u)
    requires requires {
        { t < u } -> boolean_testable;
        { u < t } -> boolean_testable;
    }
{
    if constexpr (std::three_way_comparable_with<T, U>)
        return t <=> u;
    else {
        if (t < u) return std::weak_ordering::less;
        if (u < t) return std::weak_ordering::greater;
        return std::weak_ordering::equivalent;
    }
};

template <class T, class Alloc>
constexpr auto operator<=>(const deque<T, Alloc>& lhs, const deque<T, Alloc>& rhs) {
    return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), synth_three_way);
}
