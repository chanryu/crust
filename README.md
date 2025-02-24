# Crust 🦀🔩

Crust is a C++ library that brings some of Rust’s best conveniences to C++.

## Why

Because after using Rust, writing C++ sometimes feels like making a sandwich without the bread.

## How

While I proudly admit to being a Rust fanboy, I’m not trying to turn C++ into Rust. That would be both impossible (try adding a borrow checker to C++! 🤯) and unnecessary.

Instead, Crust carefully ports Rust features that make practical sense in C++, enhancing the language without fighting it.

## What

Crust is still in its early days, and there's plenty more to come!

Right now, it includes:

### Mutex

`crust::Mutex` is designed to mimic Rust’s `std::sync::Mutex` by binding the mutex with the data it protects. This prevents accidental misuse, such as accessing shared data without locking. Unlike `std::mutex` in C++, where the mutex and data are separate, `crust::Mutex<T>` ensures that locking grants scoped, safe access to `T`, just like Rust’s `Mutex<T>`. It simplifies synchronization by encapsulating both the locking mechanism and the protected resource in a single entity.

Below is a side-by-side code comparison of `crust::Mutex` and `std::sync::Mutex`. The example is intentionally written to be as similar as possible to highlight their conceptual equivalence.

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
