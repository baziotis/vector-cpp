#pragma once

#include <memory>

template<typename T> class vector {
public:

	typedef T* iterator;
	typedef const T* const_iterator;

	/// Constructors
	vector();
	explicit vector(size_t n, const T& v = T());
	// Copy constructor
	vector(const vector<T>& v);
	// move constructor (rvalue reference)
	vector(vector<T>&& rhs) noexcept;


	/// Destructor
	~vector();

	void push_back(const T& val);
	void clear(void);  // Same as calling destructor, but explicitly
	iterator erase(iterator pos);

	/// Operator overloading
	T& operator[](size_t i);
	const T& operator[](size_t i) const;

	vector<T>& operator=(const vector<T>& rhs);
	// move assignment
	vector<T>& operator=(vector<T>&& rhs) noexcept;

	inline iterator begin(void) { return this->data; }
	inline const_iterator begin(void) const { return this->data; }

	inline iterator end(void) { return this->one_past_last; }
	inline const_iterator end(void) const { return this->one_past_last; }

	inline size_t size(void) const { return this->len; }
	inline bool empty(void) const { return this->len == 0; }

private:
	/// Object to handle memory
	std::allocator<T> alloc;

	/// Private properties
	size_t cap;
	size_t len;
	iterator one_past_last;  // one past the last constructed element
	iterator data;

	/// Private methods
	void _construct(void);
	void _construct(const_iterator s, const_iterator e);
	void _construct(size_t n, const T& v);
	void _destroy(void);
	void _grow(size_t new_len);
	void _swap(vector<T>& other) noexcept;
};

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>

// C++17 craziness (fold expressions) to print a simple fatal message
#include <cstdlib>
template <typename Arg, typename... Args>
void fatal(Arg&& arg, Args&&... args) {
	std::cout << "FATAL: ";
	std::cout << std::forward<Arg>(arg);
	((std::cout << ' ' << std::forward<Args>(args)), ...);
	std::cout << std::endl;
	exit(1);
}

template<typename T>
vector<T>::vector() {
	this->_construct();
}

template<typename T>
vector<T>::vector(const vector& v) {
	_construct(v.begin(), v.end());
}

template<typename T>
vector<T>::vector(size_t n, const T& v) {
	_construct(n, v);
}

// move constructor
template<typename T>
vector<T>::vector(vector<T>&& rhs) noexcept
	: len(0),
	  cap(0),
	  data(nullptr),
	  one_past_last(nullptr) {

	rhs._swap(*this);
}

template<typename T>
vector<T>::~vector() {
	this->_destroy();
}

template<typename T>
void vector<T>::_construct(void) {
	this->cap = this->len = 0;
	this->data = this->one_past_last = NULL;
}

template<typename T>
void vector<T>::_construct(size_t n, const T& v) {
	this->len = this->cap = n;
	try {
		this->data = this->alloc.allocate(n);
	}
	catch (std::bad_alloc&) {
		fatal("memory allocation failed");
	}
	this->one_past_last = this->data + n;
	std::uninitialized_fill(this->data, this->one_past_last, v);
}

template<typename T>
void vector<T>::_construct(const_iterator s, const_iterator e) {
	size_t new_cap = e - s;
	this->data = this->alloc.allocate(new_cap);
	this->one_past_last = std::uninitialized_copy(s, e, this->data);
	this->len = this->cap = new_cap;
}

template<typename T>
void vector<T>::_grow(size_t new_len) {
	// overflow check
	assert(this->cap < (SIZE_MAX - 1) / 2);
	// minimum of 8 elements
	size_t new_cap = std::max(2 * this->cap + 1, std::max(new_len, (size_t)8));
	// allocate new data and copy
	T* new_data = NULL;
	try {
		new_data = this->alloc.allocate(new_cap);
	}
	catch (std::bad_alloc&) {
		fatal("memory allocation failed");
	}
	T* new_one_past_last = std::uninitialized_copy(this->data, this->one_past_last, new_data);
	// destroy the old data
	_destroy();
	// set the new iterators
	this->data = new_data;
	this->one_past_last = new_one_past_last;
}

template<typename T>
void vector<T>::_swap(vector<T>& other) noexcept {
	using std::swap;
	swap(this->data, other.data);
	swap(this->one_past_last, other.one_past_last);
	swap(this->len, other.len);
	swap(this->cap, other.cap);
}

template<typename T>
void vector<T>::push_back(const T& v) {
	size_t new_len = this->len + 1;
	if (new_len > this->cap) _grow(new_len);
	// construct the last appended element
	// placement new - call the constructor explicitly
	new (this->one_past_last) T(v);
	this->len = new_len;
}

template<typename T>
void vector<T>::clear(void) {
	this->_destroy();
}

// erase an element from a vector
template<typename T>
typename vector<T>::iterator vector<T>::erase(iterator pos) {
	iterator i;
	// remove the element - aka do compaction
	for (i = pos; i != this->one_past_last - 1; ++i) {
		// call the destructor explicitly
		reinterpret_cast<T*>(i)->~T();
		// placement new - calling the constructor explicitly
		new (this->one_past_last) T(*(i + 1));
	}
	reinterpret_cast<T*>(i)->~T();
	this->one_past_last--;
	this->len--;
	return pos;
}

template<typename T>
void vector<T>::_destroy(void) {
	if (this->data) {
		// destroy (in reverse order) the elements that were constructed
		iterator it = this->one_past_last;
		while (it != this->data) {
			// call the destructor explicitly
			reinterpret_cast<T*>(--it)->~T();
		}
		// return all the space that was allocated
		this->alloc.deallocate(this->data, this->cap);
	}
	// reset pointers
	this->cap = this->len = 0;
	this->data = this->one_past_last = NULL;
}

template<typename T>
T& vector<T>::operator[](size_t i) {
	return this->data[i];
}

template<typename T>
const T& vector<T>::operator[](size_t i) const {
	return this->data[i];
}

// NOTE(stefanos): Should we destroy and check for self-assignment?
template<typename T>
vector<T>& vector<T>::operator=(const vector& rhs) {
	// check self-assignment
	if (this != &rhs) {
		// free the left side (the 'this' side)
		_destroy();

		// copy construct from rhs
		_construct(rhs.begin(), rhs.end());
	}
	return *this;
}

// move assignment
template<typename T>
vector<T>& vector<T>::operator=(vector<T>&& rhs) noexcept {
	rhs.swap(*this);
	return *this;
}