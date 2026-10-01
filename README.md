# UCP

Vulkan program prototype

## Requirements

- Vulkan SDK
- GLFW

## Build

```bash
make          # native build
make run      # build and run
make windows  # cross-compile for windows (mingw required)
```

#### Docker CI local build:
Docker and woodpecker-cli required for CI  
Bootstrap:
```bash
docker build -t vulkan-ci-build:latest -f Dockerfile-ci .
```
Build:
```bash
make ci-build
```
