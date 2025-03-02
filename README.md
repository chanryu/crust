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


### Cow

`crust::Cow` (Copy-on-Write) is inspired by Rust's `std::borrow::Cow`. It provides efficient handling of data that's usually read but occasionally modified. Unlike Rust's implementation, which can borrow or own data, the C++ version always owns its data but optimizes copying by sharing the underlying data until a mutation occurs.

`crust::Cow<T>` is particularly useful when you need to pass read-only data around that may occasionally need modification. Instead of always making deep copies, it only clones the data when it's actually modified, improving performance for read-heavy workloads.

<table>
<tr>
  <th>crust::Cow</th>
  <th>std::borrow::Cow</th>
</tr>
<tr>
  <td>

```cpp
// Create a Cow with an initial string
crust::Cow text("hello");

// Reading doesn't clone the data
std::cout << *text << std::endl;

// Multiple Cows can share the same data
auto text2 = text;
std::cout << (text.is_unique() ? "unique" : "shared")
          << std::endl; // "shared"

// Mutating triggers copy-on-write
text.mutate([](std::string& s) {
    s += " world";
});

// After mutation, data is no longer shared
std::cout << (text.is_unique() ? "unique" : "shared") 
          << std::endl; // "unique"
std::cout << *text << std::endl;  // "hello world"
std::cout << *text2 << std::endl; // "hello"
```

  </td>
  <td>
  
```rust
// Create a Cow that borrows a string slice
let text: Cow = Cow::Borrowed("hello");

// Reading doesn't clone the data
println!("{}", text);

// Making a clone for comparison 
let text2 = text.clone();
println!("{}", 
    if Cow::is_borrowed(&text) { "borrowed" } 
    else { "owned" }); // "borrowed"

// Mutating converts to owned
let text = text.into_owned() + " world";
let text = Cow::Owned(text);

// After mutation, data is now owned
println!("{}", 
    if Cow::is_borrowed(&text) { "borrowed" } 
    else { "owned" }); // "owned"
println!("{}", text);  // "hello world"
println!("{}", text2); // "hello"
```

  </td>
</tr>
</table>

The C++ implementation provides a clean, safe interface:

- Safe read access via const references and operators
- Explicit mutation via `mutate()` method with lambda
- Automatic copy-on-write semantics
- Works with any copyable type