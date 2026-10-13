add_executable(cerastes-fe main.cpp)
target_link_libraries(cerastes-fe PRIVATE cerastes_backend edg_fe)
target_compile_features(cerastes-fe PRIVATE cxx_std_20)
target_compile_options(cerastes-fe PRIVATE -Wall -Wextra -Werror)
