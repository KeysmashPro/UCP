# UCP

Vulkan program prototype

## Requirements

- Vulkan SDK
- GLFW
- Or just podman

## Build

```bash
make          # native build
make run      # build and run
make windows  # cross-compile for windows (mingw64 required)
make debug    # build and run with ASan/UBSan -O0 -g3 -DDEBUG
```

#### Rootless local build with podman:
Build:
```bash
make podman-release  #(for linux)
```
