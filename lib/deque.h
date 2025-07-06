#pragma once
#include <algorithm>
#include <cmath>
#include <concepts>
#include <iostream>
#include <iterator>
#include <memory>

static inline const size_t deque_buffer_size = 512;

template <typename T, typename Allocator = std::allocator<T>>
class deque {
   public:
    template <typename Tp>
    class Iterator;

    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const reference;
    using pointer = std::allocator_traits<Allocator>::pointer;
    using const_pointer = std::allocator_traits<Allocator>::const_pointer;
    using iterator_category = std::random_access_iterator_tag;

    using iterator = Iterator<value_type>;
    using const_iterator = Iterator<const value_type>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    deque();
    explicit deque(const Allocator& alloc);
    explicit deque(size_type count, const Allocator& alloc = Allocator());
    deque(size_type count, const T& value, const Allocator& alloc = Allocator());

    template <typename InputIt>
    deque(InputIt first, InputIt last, const Allocator& alloc = Allocator());

    deque(const deque& other);
    deque(deque&& other);

#if __cplusplus >= 202002L
    deque(const deque& other, const std::type_identity_t<Allocator>& alloc);
    deque(deque&& other, const std::type_identity_t<Allocator>& alloc);
#endif
    deque(std::initializer_list<value_type> init, const Allocator& alloc = Allocator());

    ~deque();

    // assignment operator
    deque& operator=(const deque& other);
    deque& operator=(deque&& other) noexcept(std::allocator_traits<Allocator>::is_always_equal::value);

    deque& operator=(std::initializer_list<value_type> init);
    
    // assign
    void assign(size_type count, const T& value);

    template <class InputIt>
    void assign(InputIt first, InputIt last);

    void assign(std::initializer_list<T> ilist);

    allocator_type get_allocator() const;

    reference at(size_type pos);
    const_reference at(size_type pos) const;

    reference operator[](size_type pos);
    const_reference operator[](size_type pos) const;

    reference front();
    const_reference front() const;

    reference back();
    const_reference back() const;

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const noexcept;

    iterator end();
    const_iterator end() const;
    const_iterator cend() const noexcept;

    reverse_iterator rbegin();
    const_reverse_iterator rbegin() const;
    const_reverse_iterator crbegin() const noexcept;

    reverse_iterator rend();
    const_reverse_iterator rend() const;
    const_reverse_iterator crend() const noexcept;

    [[nodiscard]] bool empty() const;
    size_type size() const;
    size_type max_size() const;
    void shrink_to_fit();

    void clear();

    iterator insert(const_iterator pos, const T& value);
    iterator insert(const_iterator pos, T&& value);
    iterator insert(const_iterator pos, size_type count, const T& value);

    template <class InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last);

    iterator insert(const_iterator pos, std::initializer_list<T> ilist);

    template <class... Args>
    iterator emplace(const_iterator pos, Args&&... args);

    iterator erase(const_iterator pos);
    iterator erase(const_iterator first, const_iterator last);

    void push_back(const value_type& value);
    void push_back(value_type&& value);

    template <class... Args>
    reference emplace_back(Args&&... args);

    void pop_back();

    void push_front(const value_type& value);
    void push_front(value_type&& value);

    template <class... Args>
    reference emplace_front(Args&&... args);

    void pop_front();
    void resize(size_type count);
    void resize(size_type count, const value_type& value);

    void swap(deque& other) noexcept(std::allocator_traits<Allocator>::is_always_equal::value);

    template <typename Tp>
    class Iterator {
       public:
        template <typename U>
        friend class Iterator;

        using value_type = Tp;
        using reference = value_type&;
        using const_reference = const reference;
        using pointer = value_type*;
        using const_pointer = const pointer;
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = std::ptrdiff_t;

       private:
        using el_pointer = pointer;
        using map_pointer = el_pointer*;

       public:
        value_type operator++(int);
        reference operator++();
        value_type operator--(int);
        reference operator--();

        reference operator*();
        const_reference operator*() const;

        pointer operator->();
        const_pointer operator->() const;

        Iterator operator+(difference_type n);
        Iterator operator-(difference_type n);

        template <typename U>
        difference_type operator-(const Iterator<U>& it) const;

        bool operator==(const Iterator& other) const;
        bool operator!=(const Iterator& other) const;

        Iterator() = default;
        Iterator(el_pointer start, el_pointer curr, el_pointer finish, map_pointer curr_node);
        Iterator(const Iterator& other);
        Iterator operator=(const Iterator& other);
        Iterator(Iterator&& other);
        Iterator operator=(Iterator&& other);

        template <typename U>
        Iterator(const Iterator<U>& other);

       private:
        el_pointer start_el_ = nullptr;
        el_pointer curr_el_ = nullptr;
        el_pointer finish_el_ = nullptr;
        map_pointer curr_node_ = nullptr;
    };

   private:
    using map_pointer = pointer*;
    using PMapAlloc = typename std::allocator_traits<Allocator>::rebind_alloc<pointer>;
    using alloc_traits = std::allocator_traits<Allocator>;
    using pmap_alloc_traits = std::allocator_traits<PMapAlloc>;

    explicit deque(const Allocator& alloc, const PMapAlloc& pmap_alloc);

    template <typename InputIt>
    deque(InputIt first, InputIt last, const Allocator& alloc, const PMapAlloc& pmap_alloc);

    static size_t deque_buffer_sz(size_t sz);
    void default_constr_with_memory_cap(size_type nodes_cnt, size_type borders_offset = 1);
    void reallocate_pointers_map(size_type offset_borders = 0);
    void shrink_to_fit_nodes();
    void move_nodes(map_pointer new_begin, size_type new_begin_ind, map_pointer old_begin, size_type old_begin_ind,
                    size_type cnt);

    void destroy_node(map_pointer node, size_type first_ind, size_type last_ind);

    template <typename Func>
    iterator insert_front(const_iterator pos, size_type cnt, Func&& get_value);

    template <typename Func>
    iterator insert_back(const_iterator pos, size_type cnt, Func&& func);

    iterator erase_front(const_iterator first, const_iterator last);
    iterator erase_back(const_iterator first, const_iterator last);

    void swap_elemets(value_type& first, value_type& second);

    void next_element(map_pointer& node, size_type& ind);
    void prev_element(map_pointer& node, size_type& ind);

    template <typename... Args>
    void resize_templ(size_type cnt, Args... args);

    void move_assign_each_element_individually(deque&& other);
    void copy_assign_each_element_individually(const deque& other);

    PMapAlloc get_pmap_allocator() const;

    // borders of capacity
    map_pointer start_node_;
    map_pointer finish_node_;

    // borders of storage data
    map_pointer curr_begin_node_;
    map_pointer curr_end_node_;

    size_type begin_ind_;
    size_type end_ind_;

    size_t buffer_size_;

    [[no_unique_address]] Allocator alloc_;
    [[no_unique_address]] PMapAlloc pmap_alloc_;  // аллокатор, управляющий памятью для мапы указателей
};

template <class T, class Alloc>
void swap(deque<T, Alloc>& lhs, deque<T, Alloc>& rhs) noexcept(noexcept(lhs.swap(rhs)));

template <class T, class Alloc>
bool operator==(const deque<T, Alloc>& lhs, const deque<T, Alloc>& rhs);

template <class T, class Alloc>
constexpr auto operator<=>(const deque<T, Alloc>& lhs, const deque<T, Alloc>& rhs);

#if __cplusplus >= 202602L  // since C++26
template <class T, class Alloc, class U = T>
constexpr typename deque<T, Alloc>::size_type erase(deque<T, Alloc>& c, const U& value);
#else
template <class T, class Alloc, class U>  // until C++26
typename deque<T, Alloc>::size_type erase(deque<T, Alloc>& c, const U& value);
#endif

template <class T, class Alloc, class Pred>
typename deque<T, Alloc>::size_type erase_if(deque<T, Alloc>& c, Pred pred);

#include "deque.inl"