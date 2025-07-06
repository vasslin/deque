# Deque
## STL-compatible container


STL-compatible implementation of deque (double-ended-queue).
The implementation provides all the basic operations of a deque with support for random access, efficient insertion/deletion at both ends, and a custom allocator.


### Main characteristics

- full compliance with the std::deque interface from the C++ standard library
- move-semantics support
- custom allocators support
- random access to elements (O(1))
- strong exception guarantee

### Implementation

The implementation uses the _"map of pointers"_ approach, where:
The elements are stored in blocks of a fixed size (`deque_buffer_size`)

Pointers to blocks are stored in a separate array.

The block size is automatically calculated based on the size of the saved type using the formula:
```
 buffer_size_ = max(deque_buffer_size / sizeof(deque::value_type), 1)

```

The deque class members:

- `start_node_` - a poiner to the beggining of the array of pointers
- `finish_node_` - a pointer to the end of the array of pointers (including)
- `curr_begin_node_` - a pointer to the first element in array of pointers (node) that contains constructed data
- `begin_ind_` - index of first constructed element in `curr_begin_node_`
- `curr_end_node_` - a pointer to the last element in array of pointers (node) that contains constructed data
- `end_ind_` - index of the element following the last constructed element in `curr_end_node_`
- `buffer_size_` - size of buffer. (each node (aka element of pointers map) between `curr_begin_node_` and `curr_end_node_` 
 has a pointer to the allocated memory for `buffer_size_` elements of deque::value_type)
- `alloc_` - allocator for memory management for deque::value_type elements
- `pmap_alloc_` - allocator for memory management for pointer map elements (nodes)


### Key member functions

1. Constructors :

```
deque();                                                                // default constructor
explicit deque(const Allocator& alloc);                                 // constructor with allocator
explicit deque(size_type count, const Allocator& alloc = Allocator());  // deque with count default-inserted objects

// deque with count copies of elements with value value
deque(size_type count, const T& value, const Allocator& alloc = Allocator());

// constructs a deque with the contents of the range [first, last)
template <typename InputIt>
deque(InputIt first, InputIt last, const Allocator& alloc = Allocator());

deque(const deque& other);  // copy-constructor
deque(deque&& other);       // move-constructor

// a copy/move constructor in which alloc is used as an allocator.
#if __cplusplus >= 202002L  // if the C++ language version is С++20 or higher
    deque(const deque& other, const std::type_identity_t<Allocator>& alloc);
    deque(deque&& other, const std::type_identity_t<Allocator>& alloc);
#endif

//  constructor with initializer list
deque(std::initializer_list<value_type> init, const Allocator& alloc = Allocator());

```


2. Element access :

- __at__ - access specified element with bounds checking
- __operator[]__ - access specified element
- __front__ - access the first element
- __back__ - access the last element

3. Iterators :

- __begin, cbegin__ - returns an iterator to the beginning
- __end, cend__ - returns an iterator to the end
- __rbegin, crbegin__ - returns a reverse iterator to the beginning
- __rend, crend__ - returns a reverse iterator to the end

4. Capacity :

- __empty__ - checks whether the container is empty
- __size__ returns the number of elements
- __max_size__ - returns the maximum possible number of elements
- __shrink_to_fit__ - reduces memory usage by freeing unused memory


5. Modifiers
- __clear__ - clears the contents
- __insert__ - inserts elements
- __emplace__ - constructs element in-place
- __erase__ - erases elements
- __push_back__ - adds an element to the end
- __emplace_back__ - constructs an element in-place at the end
- __pop_back__ - removes the last element
- __push_front__ - inserts an element to the beginning
- __emplace_front__ - constructs an element in-place at the beginning
- __pop_front__ - removes the first element
- __resize__ - changes the number of elements stored
- __swap__ - swaps the contents


6. Other member functions:
- __assign__ - assigns values to the container
- __get_allocator__ - returns the associated allocator
- __~deque__ - destructs the deque
- __operator=__ - assigns values to the container





<ins>__Non-member functions__:</ins>

- __operator<=>__ - lexicographically compares the values of two deques
- __swap(deque1, deque2)__ - specializes the swap algorithm
- __erase(deque), erase_if(deque)__ - erases all elements satisfying specific criteria
  

### Complexity


All operations at the ends (push/pop front/back) are performed in O(1) amortized time. Index access is O(1), insertion/deletion in the middle O(N), respectively.


### Implementation features

- _Exception safety_: The implementation provides a strong exception safety guarantee for all operations.
- _Movement optimization_: std::move_if_noexcept is used to safely move elements.
- _Rule of five_: the implementation has all five special member functions:
user-defined destructor, user-defined copy and move constructors, user-defined copy and move assignment operators.
- __Allocator support__: Full support for custom allocators based on propagate_on_container_* properties.


### Compiler Requirements

Support for the C++20 standard


### Usage examples:

See in [bin/main.cpp](bin/main.cpp)


### Building and launching :

1. Clone the repository
2. Build the project 

```
cd deque
mkdir build && cd build
cmake ..
cmake --build .
```

3. Run the example:

```
./deque
```