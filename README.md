# Crust 🦀🛠️

Crust is a C++ library that adds some of the missing conveniences from Rust—because sometimes,
after using Rust, writing C++ feels like eating a sandwich without the bread.

# What

## Mutex

`crust::Mutex` is designed to mimic Rust’s `std::sync::Mutex` by binding the mutex with the data it protects. This prevents accidental misuse, such as accessing shared data without locking. Unlike `std::mutex` in C++, where the mutex and data are separate, `crust::Mutex<T>` ensures that locking grants scoped, safe access to `T`, just like Rust’s `Mutex<T>`. It simplifies synchronization by encapsulating both the locking mechanism and the protected resource in a single entity.

Here's 

<table>
<tr>
  <th>std::sync::Mutex</th>
  <th>crust::Mutex</th>
</tr>
<tr>
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
  <td>

```cpp
crust::Mutex<int> counter{0};
constexpr int num_threads = 10;
std::vector<std::thread> threads;

for (int i = 0; i < num_threads; ++i) {

    threads.emplace_back([&counter]() {
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
</tr>
</table>