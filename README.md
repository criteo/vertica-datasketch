# Vertica DataSketches
This repository contains C++ bindings between Apache Datasketches library and Vertica Database. It was created by the Analytics Infrastructure teams at Criteo.

Details on the library and underlying algorithm can be found here https://datasketches.apache.org/

This extensions uses the open-source C++ implementation https://github.com/apache/incubator-datasketches-cpp/

**Currently only the theta sketch is implemented for Vertica.**

# Build
## Requirements
cmake 3.14+

## How to build
```
mkdir build
cd build
cmake ../SOURCES
make
```

Additional build options can be enabled by runing ccmake.

# Known issues
In Vertica, each query is given at runtime a pool which depends of the configuration of the database and the context (User, Roles, etc).

The Datasketch-CPP library uses C++ standard allocators to allocate/release the memory required for sketch processing.

The problem is that in its current state, the Datasketch library can only be integrated with compile time/ static allocators and the API does not offer a way to initialize those allocators with external resource at runtime (calls to allocators default constructor internally).

Ideally the datasketch library would allow users to pass in instances of custom allocator rather than only their types.

As a workaround we have built a simple custom memory allocator that constrains the algorithm up to 10GB of memory (of heap outside of the vertica pool).

**This is not ideal** and we plan to improve that by working the the datasketches-cpp maintainers.
