#ifndef UTILS_INCLUDED
#define UTILS_INCLUDED

#if defined(__BAMBU__) && !defined(STATIC)
#define STATIC
#endif

#ifdef STATIC
#include <etl/deque.h>
#include <etl/priority_queue.h>
#include <etl/vector.h>
#else
#include <deque>
#include <queue>
#include <vector>
#endif

namespace utils {

typedef unsigned int size_t;

#ifdef STATIC
template <typename T, const utils::size_t MAX_SIZE>
using vector_t = etl::vector_storage<T, MAX_SIZE>;

template <typename T, const utils::size_t MAX_SIZE,
          typename Compare =
              etl::less<typename etl::vector_storage<T, MAX_SIZE>::value_type>>
using priority_queue_t =
    etl::priority_queue<T, MAX_SIZE, etl::vector_storage<T, MAX_SIZE>, Compare>;

template <typename T, const utils::size_t MAX_SIZE>
using deque_t = etl::deque<T, MAX_SIZE>;

#else
template <typename T, const utils::size_t MAX_SIZE>
using vector_t = std::vector<T>;

template <typename T, const utils::size_t MAX_SIZE,
          typename Compare = std::less<typename std::vector<T>::value_type>>
using priority_queue_t = std::priority_queue<T, std::vector<T>, Compare>;

template <typename T, const utils::size_t MAX_SIZE>
using deque_t = std::deque<T>;
#endif

} // namespace utils

#endif // UTILS_INCLUDED