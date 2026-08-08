# PACFG Spec

`.pacfg` is an `ini` format configuration file, used to describe the properties of file extensions managed by the "Pineapple Assoc Manager".

## Config file example

```ini
; Top-level settings
targetApp=passoc.exe
friendlyAppName=Pineapple Assoc Manager
friendlyAppName[zh_CN]=菠萝格式关联管理器 ; i18n support
openCommand="\"{targetAppFullPath}\" \"%1\"" ; default open command (in case ProgId section does not specify one)
genericFileIcon=icons/generic.ico

[ProgId/pacfg]
name=Pineapple Association Configuration File ; display name of the file type
name[zh_CN]=菠萝格式关联配置文件 ; i18n support
extensions=pacfg
openCommand="\"{targetAppFullPath}\" -c \"%1\"" ; open command (overrides global openCommand)
icon=icons/pacfg.ico ; icon (optional)

[ContextMenu/editWithMyApp]
name=Edit with MyApp ; context menu display text
name[zh_CN]=使用 MyApp 编辑 ; i18n support
target=* ; applies to all files
command="\"{targetAppFullPath}\" --edit \"%1\"" ; command to execute
```

## Config file content description and notes

### Top-Level Settings

| Key | Description |
| :--- | :--- |
| `targetApp` | The executable name of the target application (e.g., `myapp.exe`), path relative to the directory where the `.pacfg` file is located. |
| `friendlyAppName` | The display name of the application in Windows settings，will display in Windows settings |
| `openCommand` | default open command. `{targetAppFullPath}` will be replaced to the `targetApp`'s full path |
| `genericFileIcon` | Fallback generic file icon (optional), defaults to `icons/generic.ico` |

### ProgId Sections

Define file types using `[ProgId/<ID>]` sections.

| Key | Description |
| :--- | :--- |
| `name` | The description of the file type (e.g. "JPEG Image"). |
| `extensions` | Comma-separated list of extensions (e.g. `jpg,jpeg`). If omitted, the ProgId ID is used as the extension. |
| `icon` | (Optional) Path to the icon file. Defaults to `icons/<ID>.ico` if omitted. |
| `openCommand` | (Optional) Specific command to open this file type. Overrides the global `openCommand`. |

### ContextMenu Sections

Define right-click context menu entries using `[ContextMenu/<ID>]` sections. These add custom actions (e.g. "Edit with MyApp") to the Windows Explorer right-click menu. This section is entirely optional — omit it if the target application does not need context menu entries.

| Key | Description |
| :--- | :--- |
| `name` | The display text shown in the context menu (e.g. "Edit with MyApp"). |
| `target` | (Optional) What the menu item applies to. `*` for all files (default), or comma-separated extensions (e.g. `txt,md`). |
| `command` | The command to execute. `{targetAppFullPath}` is replaced with the target app's full path. `%1` is the right-clicked file. Falls back to the global `openCommand` if omitted. |
| `icon` | (Optional) Icon shown next to the menu item. Defaults to the target application's icon. |

### Notes

0. `.pacfg` config file must be placed in the same directory as the target application, other usage is not supported.
1. `targetApp`'s value must match the actual executable name of the target application. Assumes `ppic.exe`, the value must be `ppic.exe`.
2. `friendlyAppName` cannot be omitted, it will be used to register the file type list information.
3. `openCommand` defines the command to open files. The top-level `openCommand` will be used as the default open command, can be overridden by `ProgId` section.
   - `{targetAppFullPath}` will be replaced to `targetApp`'s full path.
   - Since the configuration file is an ini file, please note that characters that need to be escaped in it, such as double quotes `"` need to be escaped as `\"`.
4. `ProgId` can have multiple groups, `[ProgId/{ProgIdName}]` name can be customized, but it is recommended to keep it consistent with one of the actual extensions in the group.
5. `extensions` can have multiple extensions, separated by commas, and no additional spaces after the comma.
6. `friendlyAppName` and `name` under `ProgId` support localization. The actual name written to the registry is the localized name matched.
7. `icon` is optional field, can specify an icon file. The path is relative to the directory where the target application is located. When this field is omitted, the default value is `icons/{ProgIdName}.ico`. If the icon file cannot be found, the `genericFileIcon` specified icon will be used. If it still cannot be found, the icon of the target application will not be registered (the system will use the icon of the target application).
8. `genericFileIcon` is optional top-level field, used to specify a generic fallback icon, defaults to `icons/generic.ico`.
