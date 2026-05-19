# memory-allocator-c
# Memory Allocator in C

A custom memory allocator implemented in C, based on a free list (doubly linked list).

## Features
- Custom `my_malloc` and `my_free` implementation
- Uses `sbrk` to request memory from the OS
- Block corruption detection via magic bytes
- Thread-safe with a simple lock

## How it works
The allocator manages a heap divided into blocks tracked by a doubly linked list.
Each block has a header (`area`) with metadata like size, in-use status, and markers.
A global `my_stats` header tracks overall allocator state.

## Based on
[Malloc is not magic](https://levelup.gitconnected.com/malloc-is-not-magic-implementing-my-own-memory-allocator-e0354e914402)
