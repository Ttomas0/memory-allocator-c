# memory-allocator-c

Custom memory allocator in C — free list, block splitting, heap shrinking, thread-safety.

![C99](https://img.shields.io/badge/C-99-blue) ![POSIX](https://img.shields.io/badge/platform-POSIX%20%2F%20Linux-orange) ![status](https://img.shields.io/badge/status-learning%20project-green) ![license](https://img.shields.io/badge/license-MIT-purple)

---

## Overview

Implements `an_malloc` and `an_free` from scratch without touching libc's allocator.
Memory is managed as a doubly linked list of blocks, each with an `area` header
carrying size, in-use status, and a magic marker for corruption detection.
A global `my_stats` header at `heap_start` tracks block count, page count, and a coarse lock.
When no free block is large enough, the heap grows via `sbrk(4096)`;
when large trailing regions go unused, `reduce_heap_size_if_possible()` trims them
back with `sbrk(-PAGE_SIZE)`.

---

## Features

- **Best-fit free list** — scans all free blocks, picks the smallest that fits; reduces fragmentation vs first-fit
- **sbrk heap growth** — requests raw pages from the OS in 4 KB increments; no libc malloc underneath
- **Magic byte guards** — `0x55` init sentinel + `0xDD` per-block marker; detects wild writes and double-free early
- **Block coalescing** — on free, merges adjacent free blocks (next + prev) to prevent fragmentation buildup
- **Simple spin lock** — `my_simple_lock` in the stats header prevents concurrent alloc/free races
- **Heap shrinking** — releases trailing unused pages back to the OS after each free

---

### Block header (`area`)

| field | type | description |
|---|---|---|
| `marker` | `int` | must equal `0xDD`; checked on every alloc/free |
| `InUse` | `bool` | whether this block is currently allocated |
| `length` | `u32` | usable bytes (excludes `sizeof(area)`) |
| `prev` / `next` | `area *` | doubly linked list pointers |

---

## Build & run

```bash
# sbrk requires _DEFAULT_SOURCE (not exposed under strict C99)
gcc -Wall -Wextra -g -D_DEFAULT_SOURCE allocator.c -o allocator

# run all fork-isolated tests
./allocator

# with address sanitizer
gcc -fsanitize=address -g -D_DEFAULT_SOURCE allocator.c -o allocator && ./allocator
```

> **Note:** `-std=c99` alone hides `sbrk()` in glibc — the compiler assumes it returns
> `int`, truncates the 64-bit pointer, and segfaults at runtime. Always add `-D_DEFAULT_SOURCE`.

---

## API reference

| symbol | description |
|---|---|
| `an_malloc(size)` | allocates `size` bytes; initializes heap on first call via the `0x55` magic byte check |
| `an_free(ptr)` | zeroes the block, coalesces neighbors, trims heap; returns `false` if marker is corrupt |
| `add_used_block(size)` | best-fit search + block splitting; grows heap with `sbrk` if no block fits |
| `reduce_heap_size_if_possible()` | walks to last used block, releases trailing pages, injects leftover block |
| `my_stats` | global header at `heap_start`: block count, page count, lock, magic byte |
| `call_test(fn, msg)` | forks a child, runs the test, reports pass/crash by signal |

---

## Tests

Each test runs in an isolated child process via `fork()` — a crash in one test
doesn't abort the suite.
Basic Malloc                  passed
Request more memory Malloc    passed
Basic Free                    passed
Complex                       passed
DONE

---

## Based on

- [Malloc is not magic](https://levelup.gitconnected.com/malloc-is-not-magic-implementing-my-own-memory-allocator-e0354e914402) — original article this implementation follows
  
---

## Related

See also `arena.c / arena.h` in this repo — a companion arena allocator using the
reserve/commit model (`mmap` + `mprotect` on Linux, `VirtualAlloc` on Windows).
The arena is faster for lifetime-scoped allocations (bump pointer, O(1) alloc);
this free list handles general-purpose dynamic allocation where lifetimes are unpredictable.
