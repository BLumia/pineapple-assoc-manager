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

The configuration file is a standard INI file.

### Top-Level Settings

| Key | Description |
| :--- | :--- |
| `targetApp` | The executable name of the target application (e.g., `myapp.exe`). |
| `friendlyAppName` | The display name of the application in Windows settings. |
| `openCommand` | The default command to open files. `{targetAppFullPath}` is replaced by the absolute path to `targetApp`. |
| `genericFileIcon` | (Optional) A fallback icon path to use if a specific ProgId icon is missing. |

### ProgId Sections

Define file types using `[ProgId/<ID>]` sections.

| Key | Description |
| :--- | :--- |
| `name` | The description of the file type (e.g., "JPEG Image"). |
| `extensions` | Comma-separated list of extensions (e.g., `jpg,jpeg`). If omitted, the ProgId ID is used as the extension. |
| `icon` | (Optional) Path to the icon file. Defaults to `icons/<ID>.ico` if omitted. |
| `openCommand` | (Optional) Specific command to open this file type. Overrides the global `openCommand`. |

**Example:**

```ini
targetApp=myapp.exe
friendlyAppName=My Application
openCommand="\"{targetAppFullPath}\" \"%1\""
genericFileIcon=icons/generic.ico

[ProgId/image]
name=Image File
extensions=png,jpg,jpeg
icon=icons/image.ico

[ProgId/text]
name=Text Document
extensions=txt
; Uses global openCommand
```

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
