# 🦊 Flox VM (v0.1.0)

![License: GPL v3](https://gnu.org)

**Flox VM** is an ultra-fast, minimalist, register-based bytecode virtual machine and interpreter written in modern C++. It is engineered from the ground up to respect the CPU architecture, completely discarding the massive runtime abstractions, pointer-chasing, and object-bloat found in modern scripting languages. 

Flox achieves native-like performance by utilizing a strict opcode dispatch system, cache-friendly flat memory arenas, and manual memory management. It achieves **nanosecond-range startup times** ("warm-up") and fits entirely within the CPU's L1/L2 Instruction Cache.

---

## 🚀 The Philosophy: Death to Bloat

Flox VM is the antidote. It operates directly on the silicon. 
- **Zero Heap Bloat:** Arrays are allocated as flat, sequential memory blocks.
- **Cache-Optimized:** Direct linear access prevents CPU pipeline stalls.
- **No JIT Overhead:** While JIT runtimes waste megabytes of RAM profiling code, Flox reaches peak execution speed instantly.

---

## 📊 Benchmarks (Measured on Intel Core i7-14650HX @ 5.2 GHz)

### 1. The I/O Loop Test (10,000 `print` Operations)
A raw throughput test flushing character streams to the console.
- **CPython:** 188,071 μs (188 ms) — *An absolute eternity of object wrapping and GIL locking.*
- **Flox VM:** **456 μs** (0.45 ms) — *Over **400x faster**, pushing hardware and OS buffer limits.*

### 2. Cryptographic & Vector Core Operations
Processing heavy sequential array math and conditional logic.
- **Flox VM:** **367 μs** to complete **500,000 complex cryptographic loop operations**.

---


## 📜 License

Distributed under the **GPLv3 License**. See `LICENSE` for more information. Corporate extraction or inclusion into proprietary closed-source systems without contributing back is strictly prohibited by copyleft law.
