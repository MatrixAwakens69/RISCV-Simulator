cmake_minimum_required(VERSION 3.5)

project(RISCV-Simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-O2 -Wall")

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(
    Simulator 
    src/MainCPU.cpp 
    src/MemoryManager.cpp 
    src/Simulator.cpp 
    src/BranchPredictor.cpp 
    src/Cache.cpp
    src/predictors/BimodalPredictor.cpp
    src/predictors/ContextualLLBPredictor.cpp
    src/predictors/DynamicAllocator.cpp
    src/predictors/PerceptronPredictor.cpp
    src/predictors/TAGEPredictor.cpp
)

add_executable(
    CacheSim 
    src/MainCache.cpp 
    src/MemoryManager.cpp 
    src/Cache.cpp
)

add_executable(
    CacheOptimized
    src/MainCacheOptimization.cpp
    src/MemoryManager.cpp
    src/Cache.cpp
)

add_executable(ToDirenoTrace src/ToDirenoTrace.cpp)