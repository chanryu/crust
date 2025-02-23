# Crust 🦀🛠️

Crust is a C++ library that adds some of the missing conveniences from Rust—because sometimes,
after using Rust, writing C++ feels like eating a sandwich without the bread.

# What

## Mutex

| crust::Mutex | std::sync::Mutex |
| - | - |
| <pre><code>
Mutex<int> counter{0};
constexpr int num_threads = 10;
std::vector<std::thread> threads;

for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([&]() {
        auto data = counter.lock();
        ++(*data);
    });
}

for (auto& thread : threads) {
    thread.join();
}

std::cout << "Final counter value: " << *counter.lock() << std::endl;
</pre></code> | x |