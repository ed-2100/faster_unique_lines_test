// Type your code here, or load an example.
#include <array>
#include <bitset>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <immintrin.h>
#include <iostream>
#include <span>
#include <stdlib.h>
#include <string>
#include <chrono>
#include <fstream>

#include <stdint.h>
#include <highwayhash/highwayhash.h>

#ifdef _WIN32
#include "file_buffer.h"
#else
#include "common.h"
#include <sys/mman.h>
#endif

__attribute__((const)) inline std::uint32_t
fast_getline(const char *_str)
{
  unsigned long long index   = 0;
  const __m256i all_newlines = _mm256_set1_epi8('\n');
  __m256i hits               = _mm256_cmpeq_epi8(_mm256_loadu_si256((__m256i *)(_str + index)), all_newlines);
  std::uint32_t outmask      = _mm256_movemask_epi8(hits);
  while (outmask == 0) [[likely]]
  {
    index += sizeof(__m256i);
    hits    = _mm256_cmpeq_epi8(_mm256_loadu_si256((__m256i *)(_str + index)), all_newlines);
    outmask = _mm256_movemask_epi8(hits);
  }
  return __builtin_ctz(outmask) + index;
}

constexpr unsigned int uint_all = ~(unsigned int)0;

__attribute__((const)) inline bool
strcmp_(const char *__restrict__ a, const char *__restrict__ b, unsigned long long len)
{
  unsigned long long c = 0;
  while (c < len)
  {
    auto adata = _mm256_loadu_si256((__m256i const *)&a[c]);
    auto bdata = _mm256_loadu_si256((__m256i const *)&b[c]);

    auto newlines = _mm256_set1_epi8('\n');

    unsigned int are_newlines = _mm256_movemask_epi8(_mm256_cmpeq_epi8(bdata, newlines));
    unsigned int are_equal    = _mm256_movemask_epi8(_mm256_cmpeq_epi8(adata, bdata));

    c += 32;
    if (are_newlines == 0)
    {
      if (are_equal == uint_all) continue;
      else
        break;
    }

    // Index of first newline
    auto zeroes = __builtin_ctz(are_newlines);

    unsigned int should_set = uint_all >> (32 - zeroes - 1);
    return (are_equal & should_set) == should_set;
  }
  return false;
}

__attribute__((const)) inline bool
test_nonz(__m256i v)
{
  return !_mm256_testz_si256(v, v);
}

__attribute__((const)) usize
npow2(usize _num)
{
  usize lzc = _lzcnt_u64(_num);
  return ((_num << 1) & (0xffffffffffffffff << (64 - lzc)));
}

void
print256epi64(__m256i _vec)
{
  puts("wat");
  alignas(32) u64 arr[4]{};
  std::memcpy(arr, &_vec, sizeof(__m256i));

  std::cout << "vector: \n";
  for (const auto ite : arr)
  {
    std::cout << "    " << ite << "\n";
  }
}

int
main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
  using std::chrono::duration;
  using std::chrono::duration_cast;
  using std::chrono::high_resolution_clock;
  using std::chrono::milliseconds;

  usize counter = 0;

#ifdef _WIN32

  std::string ipt_buf;
  auto fsize = xenon_file::get_fsize(argv[1]);
  ipt_buf.reserve(fsize);
  xenon_file::read_file(argv[1], ipt_buf);

#else
  auto file = fopen(argv[1], "r");

  fseek(file, 0L, SEEK_END);
  u64 fsize = ftell(file);
  fseek(file, 0L, SEEK_SET);

  auto fd = fileno(file);
  // Who even needs error checking
  // And an entire page to prevent read overruns
  auto data    = mmap(NULL, fsize + 0x1000, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
  auto ipt_buf = std::span<char>((char *)data, fsize);

#endif

  // use fsize for inner loop speed max
  // this is optimized for full application run speed
  const auto mapsize = (usize)(npow2(fsize / 20));

  HH_ALIGNAS(32) const highwayhash::HHKey key = {1, 1, 1, 1};
  highwayhash::HHStateT<HH_TARGET_AVX2> state(key);

  std::vector<uptr> hashmap(mapsize, 0);
  u64 current_index = 0, next_index = 0;

#ifdef _WIN32
  SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
#endif

  alignas(32) std::array<u64, 4> hash_indeces;
  std::array<u64, 4> sizes;
  std::array<uptr, 4> ptr_buf;
  u64 result_ = 0;
  alignas(32) std::array<u64, 4> hash_indeces2;
  std::array<u64, 4> sizes2;
  std::array<uptr, 4> ptr_buf2;
  u64 collisioncounter = 0;

  auto t1 = high_resolution_clock::now();

  for (usize j = 0; j != 4 && (current_index != fsize); ++j)
  {
    next_index = current_index + fast_getline((const char *)((uptr)(ipt_buf.data() + current_index)));
    highwayhash::HighwayHashT(&state, ipt_buf.data() + current_index, next_index - current_index, &result_);
    state.Reset(key);

    // potentially avx too?
    ptr_buf[j] = (uptr)(ipt_buf.data() + current_index);
    sizes[j]   = next_index - current_index;
    // make avx
    hash_indeces[j] = result_ % mapsize;
    current_index   = next_index + 1;
  }

  u64 old_current_idx = 0;
  while (old_current_idx != fsize) [[likely]]
  {
    old_current_idx = current_index;
    for (usize j = 0; j != 4 && (current_index != fsize); ++j)
    {
      next_index = current_index + fast_getline((const char *)((uptr)(ipt_buf.data() + current_index)));
      highwayhash::HighwayHashT(&state, ipt_buf.data() + current_index, next_index - current_index, &result_);
      state.Reset(key);

      // potentially avx too?
      ptr_buf2[j] = (uptr)(ipt_buf.data() + current_index);
      sizes2[j]   = next_index - current_index;
      // make avx
      hash_indeces2[j] = result_ % mapsize;
      current_index    = next_index + 1;
    }

    for (int i = 0; i < 4; ++i)
    {
      _mm_prefetch(hashmap.data() + hash_indeces2[i], _MM_HINT_NTA);
    }

    __m256i hash_index_vector = _mm256_load_si256((__m256i *)hash_indeces.data());
    __m256i map_entries       = _mm256_i64gather_epi64((long long *)hashmap.data(), hash_index_vector, 8);

    while (test_nonz(map_entries)) [[unlikely]] // hashmap[hashvalue] != 0 equivalent
    {
      __m256i are_nonzero = _mm256_xor_si256(_mm256_cmpeq_epi64(map_entries, _mm256_setzero_si256()), _mm256_set1_epi64x(~u64(0)));

      auto prefetcher = _mm256_blendv_epi8(hash_index_vector, _mm256_add_epi64(_mm256_set1_epi64x(1), hash_index_vector), are_nonzero);
      prefetcher      = _mm256_and_si256(prefetcher, _mm256_set1_epi64x(mapsize - 1));
      alignas(32) std::array<u64, 4> prefetch;
      _mm256_storeu_si256((__m256i *)prefetch.data(), prefetcher);
      for (const auto e : prefetch)
      {
        _mm_prefetch(e, _MM_HINT_NTA);
      }

      auto mask = _mm256_movemask_epi8(are_nonzero);
      for (int i = 0; i < 4; ++i)
      {
        if ((mask & (0b1 << (i * 8))) != 0)
        {
          if (strcmp_((char *)ptr_buf[i], (char *)map_entries[i], sizes[i]))
          {
            are_nonzero[i]       = 0;
            // hash_index_vector[i] = 0;
            --counter;
          }
          else
          {
            // ++collisioncounter;
          }
        }
      }

      /* + 1 % mapsize */
      hash_index_vector = _mm256_blendv_epi8(hash_index_vector, _mm256_add_epi64(_mm256_set1_epi64x(1), hash_index_vector), are_nonzero);
      hash_index_vector = _mm256_and_si256(hash_index_vector, _mm256_set1_epi64x(mapsize - 1));

      // if it wasn't 0 then load again, otherwise just 0
      map_entries = _mm256_mask_i64gather_epi64(_mm256_setzero_si256(), (long long *)hashmap.data(), hash_index_vector, are_nonzero, 8);
    }
    // Directly write to hash_indeces when we found something
    _mm256_storeu_si256((__m256i *)hash_indeces.data(), hash_index_vector);

    for (usize k = 0; k != 4; ++k)
    {
      // if (hash_indeces[k] != 0)
      // {
        ++counter;
        hashmap[hash_indeces[k]] = ptr_buf[k];
      // }
    }

    hash_indeces = hash_indeces2;
    ptr_buf      = ptr_buf2;
    sizes        = sizes2;
  }

  auto t2 = high_resolution_clock::now();
  /* Getting number of milliseconds as a double. */
  duration<double, std::milli> ms_double = t2 - t1;

  std::cout << "unique lines: " << counter << "  " << (ms_double.count() / 1000.f) << "s\n";
  std::cout << "collisions " << collisioncounter << "\n";
}
