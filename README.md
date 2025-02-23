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
        ++(*data);
    });
}

for (auto& thread : threads) {
    thread.join();
}

std::cout << "Final counter value: " << *counter.lock() << std::endl;
```

  </td>
  <td>
  
```rust
let counter = Arc::new(Mutex::new(0));
let num_threads = 10;
let mut handles = vec![];

for _ in 0..num_threads {
    let counter = Arc::clone(&counter);
    let handle = thread::spawn(move || {
        let mut value = counter.lock().unwrap();
        *value += 1;
    });
    handles.push(handle);
}

for handle in handles {
    handle.join().unwrap();
}

println!("Final counter value: {}", *counter.lock().unwrap());
```

  </td>
</tr>
</table>