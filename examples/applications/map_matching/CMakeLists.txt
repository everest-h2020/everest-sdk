
file(GLOB srcs mma/*.cpp)

include_directories(mma etl/include libbambu/ac_types/include libbambu/ac_math/include)
add_definitions(-DDEBUG_PRINT)

add_executable(mma_xample ${srcs})

add_executable(mma_static ${srcs})
target_compile_definitions(mma_static PRIVATE -DSTATIC)