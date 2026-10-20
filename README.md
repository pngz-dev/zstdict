# zstdict

Reference implementation for `draft-lindqvist-zstdict` (Zstandard Dictionary Content-Encoding).

[![Demo](curl_proof.svg)](https://github.com/pngz-dev/zstdict/blob/main/curl_proof.cast)

## What is this?
This is a C library and WebAssembly build that implements a new HTTP Content-Encoding called `zstdict`. It uses Zstandard compression with a pre-shared dictionary to drastically reduce the size of small web payloads (like HTML and JSON).

By defining a custom `Content-Encoding` and a `Content-Dict` header, we bypass the limitations of generic compression and allow servers to shrink payloads by 50% or more for highly repetitive data.

## The IETF Standard
This repository serves as the official reference implementation for the Internet-Draft:
👉 **[draft-lindqvist-zstdict-01](https://datatracker.ietf.org/doc/draft-lindqvist-zstdict/)**

## Usage

### 1. Native C (Termux, Linux, macOS)
Compile the CLI tool and the demo:
```bash
cc -O2 cli.c -lzstd -o zstdict-cli
cc -O2 demo.c -lzstd -o demo
```

Run the local proof-of-concept (starts a Python server and decodes the response using curl):

```bash
# Terminal 1: Start the fake server
python3 fake_server.py

# Terminal 2: Fetch and decode using zstdict
curl -s -H "Accept-Encoding: zstdict" http://127.0.0.1:8080 | ./zstdict-cli
```

## 2. WebAssembly (WASM)

Compile the library for the browser using Emscripten:

```bash
emcc demo.c -O3 \
  -I/path/to/zstd/lib -I/path/to/zstd/lib/common \
  /path/to/zstd/lib/libzstd.a \
  -s WASM=1 \
  -s EXPORTED_FUNCTIONS='["_wasm_init","_wasm_deinit","_zstdict_pack","_zstdict_unpack","_zstdict_max_out","_malloc","_free"]' \
  -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","HEAPU8"]' \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s MODULARIZE=1 \
  -s EXPORT_NAME='createZstdict' \
  -s NO_EXIT_RUNTIME=1 \
  -o zstdict.js
```

## How it works

1. The server compresses a payload using Zstandard and a pre-trained 32KB dictionary.
2. The server responds with Content-Encoding: zstdict and Content-Dict: sha256-89ff...
3. The client (or WASM module) checks the hash, loads the dictionary, and decompresses the stream on the fly.

## License

- Code: 0BSD
- Dictionary: CC0 (Public Domain)

