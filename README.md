# Crust 🦀🛠️

Crust is a C++ library that adds some of the missing conveniences from Rust—because sometimes,
after using Rust, writing C++ feels like eating a sandwich without the bread.

# What

## Mutex

<table>
<tr>
  <th>crust::Mutex</th>
  <th>std::sync::Mutex</th>
</tr>
<tr>
  <td>

```cpp
crust::Mutex<int> counter{0};
constexpr int num_threads = 10;
std::vector<std::thread> threads;

for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([&]() {
        auto data = counter.lock();
        *data += 1;
    });
}

for (auto& thread : threads) {
    thread.join();
}

assert(*counter.lock() == 10);
```

  </td>
  <td>
  
```rust
let counter = Arc::new(Mutex::new(0));
let num_threads = 10;
let mut handles = vec![];

for _ in 0..num_threads {
    let counter = Arc::clone(&counter);
    handles.push(thread::spawn(move || {
        let mut data = counter.lock().unwrap();
        *data += 1;
    }));
}

for handle in handles {
    handle.join().unwrap();
}

assert!(*counter.lock().unwrap() == 10);
```

  </td>
</tr>
</table>