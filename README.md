# osmium

Web browser written from scratch™ in C++

## Building
```
meson setup build
ninja -C build -j $(nproc)
./build/osmium
```

## Dependencies
* Qt6
* [osmium-html](https://github.com/antpiasecki/osmium-html)
* cpp-httplib