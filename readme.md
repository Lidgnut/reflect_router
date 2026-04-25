# reflect_router

An isolated development laboratory for exploring **C++26 Static Reflection (P2996)**.

## Overview
This project is dedicated to testing experimental language features proposed in [P2996](https://wg21.link/p2996). It investigates compile-time metadata extraction, constant-expression string parsing, and automated type synthesis.

> [!WARNING]
> This project is in a **proof-of-concept** state and relies on unstable compiler snapshots.

## Purpose: The Reflect-Router
The primary goal is to build a zero-overhead REST-Router where route parameters are parsed and typed at compile-time.

### The Vision
The ultimate goal is to allow natural member access for parsed parameters:

```cpp
router.addRoute<"GET /api/user/:id:=uint64_t"_route>([](auto&& path_params){
    if(path_params.id == 0)
        // ...
});
```

### Current Status & Roadblocks
During development, we encountered a significant limitation in current compiler implementations (GCC 16 Snapshot):
* **GCC Bug:** `std::meta::define_aggregate` fails to detect its required `consteval` context when used within modern injection patterns, or triggers an **Internal Compiler Error (ICE)** in the GIMPLE backend when combined with `template for`.
* **The Pivot:** To maintain progress, the project currently utilizes a **Named-Tuple-Fallback**. Reflection is used to analyze the route and synthesize a `std::tuple`, while `FixedString` mapping provides a clean interface.

**Currenty possible Syntax:**

```cpp
router.addRoute<"GET /api/user/:id:=uint64_t"_route>([](auto&& path_params){
    if(path_params.get<"id"> == 0)
        // ...
});
```

**Current State:**
Currently this behavior is proven to work. With more to come.
```cpp
constexpr my_route = "/api/user/:id:=uint64_t"_route;
reflect_router::Params<my_route> req_params{"/api/user/12"};
ASSERT_EQ(req_params.get<"id">() == 12);
```


## Tech Stack
* **Compiler:** GCC 16 Snapshot (Experimental P2996 branch)
* **Standard:** C++26 (`-std=c++26` / `-std=c++2c`)
* **Features:** `-freflection`
* **Build System:** CMake 3.25+ & Ninja

## Setup
The project is optimized for **VS Code DevContainers**, to provide a pre-configured environment with the necessary GCC trunk binaries.

1. Open the project folder in VS Code.
2. Select **"Reopen in Container"** when prompted.
3. Configure the project using the `gcc-trunk` CMake preset.
4. Build via `Ctrl+Shift+B`.

## Execution
Binaries are generated in the `build/` directory.

[CODE_BLOCK: Execution command]