# Pineapple Association Manager

Pineapple Association Manager is a lightweight helper utility designed to manage file associations for portable Windows applications. It allows users to easily register and unregister file extensions for a specific target application based on a configuration file, without requiring an installer.

## Features

- **Portable Design**: Works perfectly with portable applications.
- **Configuration Driven**: All association logic is defined in a simple `.pacfg` (INI-style) configuration file.
- **User-Friendly UI**: Provides a clear interface for users to select which file types they want to associate.
- **Localization Support**: Supports localized display names for the application and file types.
- **Advanced Customization**:
  - Custom open commands per file type (ProgId).
  - Flexible icon configuration with fallback support.
  - Automatic registration in Windows "Default Apps" settings.

## Usage

### For Developers

You can distribute this tool alongside your own application.

1. Create a `default-assoc.pacfg` file describing your application's file associations.
2. Ship `passoc.exe` (or whatever the executable is named) and the `default-assoc.pacfg` file with your app.

If your app uses Qt 6, you can (and should) use this tool with the same Qt version to share DLLs and reduce distribution size.

### For End Users

End user will get a `passoc.exe` file and a `default-assoc.pacfg` file alongside your application.

1. Run the tool.
2. Select the file types you wish to associate.
3. Click **Apply Now**.

### For Advanced Users

This tool can be used as a standalone utility as well. If no external `default-assoc.pacfg` was provided, it will by default use a internal configuration that contain association information of `.pacfg` for itself. Once user choose to associate `.pacfg` to itself, user can simply double-click any `.pacfg` file to register the manage their own customized association information for their own favorite applications.

**Command Line Arguments:**

- `-c <path>` / `--config <path>`: Load a specific configuration file.
- `-d <path>` / `--demo <path>`: Run in demo mode (for testing).
- `-m` / `--manual-fallback`: Allow users to manually select target application when the target application is not found.

## Configuration (.pacfg)

Please read `docs/PACFG_SPEC.md`.

## Building

This project is built using **CMake** and **Qt 6**.

### Prerequisites

- Qt 6 SDK
- CMake
- A C++ Compiler

### Build Steps

```shell
cmake -Bbuild .
cmake --build build
```

## License

This project is licensed under the MIT License.

## Paid Support and Sponsorship

Feature requests can be made through GitHub issues or discussions but we don't garantee that we will implement them. If you want to speedup a certain feature's development, please consider sponsor us!

While this project is free and open-source, we offer paid support for Pineapple Association Manager as well. If MIT license is not an option for your usage or if you need assistance or have a question, please feel free to reach out to us at `business<at>blumia<dot>net` in private or by contacting us on GitHub.
