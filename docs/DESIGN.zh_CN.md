# 设计说明

此程序旨在帮助**目标程序**在 Windows 平台完成“文件格式关联相关信息的注册”与“文件格式关联的管理”操作。

## 词汇定义

- 目标程序：此程序所帮助管理文件格式关联信息的程序。尽管类型不限但通常是便携程序。
- 配置文件：用于描述目标程序的文件格式关联的相关信息的文件，文件扩展名为 `.pacfg`，文件格式类型为 `ini`。
- 文件格式：指目标程序所支持的文件类型，概念上近似文件扩展名，但一个文件格式可能对应多个近似的扩展名（例如 `.jpg`、`.jpeg` 和 `.jfif` 都是 JPEG 文件格式的文件）。
- `ProgID`：程序标识符（Programmatic Identifier），但此应用中基本等同于描述一组文件格式。选用此名称是因为和注册表对应的字段名称一致。

## 程序行为

程序启动时，会读取配置文件中的信息，然后根据配置文件中的信息对用户呈现目标程序所支持的文件格式列表（`ProgId`）。用户可在对应列表中勾选自己希望目标程序所支持的文件格式。

当用户点击“注册关联”按钮时，程序会先向系统注册目标程序的基本信息（显示名称等），然后根据用户所勾选的文件格式列表，注册或取消注册目标程序所支持的文件格式。如果点击“注册关联”按钮时没有勾选任何文件格式，则会向系统取消注册目标程序以及所有文件格式关联信息。

## 相关注册表位置

假定目标程序 `MyApp.exe`，期望支持的文件格式是 JPEG，包含扩展名 `jpg`，本程序会向下列注册表位置写入内容：

```ini
; ----------------------------------------------------------
; 1. 声明 MyApp 的 Capabilities（核心）
; ----------------------------------------------------------
[HKEY_CURRENT_USER\Software\Classes\Applications\MyApp.exe\Capabilities]
"ApplicationName"="MyApp"
"ApplicationDescription"="MyApp Image Viewer"

[HKEY_CURRENT_USER\Software\Classes\Applications\MyApp.exe\Capabilities\FileAssociations]
".jpg"="MyApp.jpg"

; ----------------------------------------------------------
; 2. 将 MyApp 注册进 RegisteredApplications（必须）
;    这样 ms-settings:defaultapps?registeredAppUser=MyApp 才能找到
; ----------------------------------------------------------
[HKEY_CURRENT_USER\Software\RegisteredApplications]
"MyApp"="Software\\Classes\\Applications\\MyApp.exe\\Capabilities"

; ----------------------------------------------------------
; 3. 定义 ProgID: MyApp.jpg
; ----------------------------------------------------------
[HKEY_CURRENT_USER\Software\Classes\MyApp.jpg]
@="MyApp JPEG File"

[HKEY_CURRENT_USER\Software\Classes\MyApp.jpg\DefaultIcon]
@="\"C:\\Users\\Gary\\Sources\\pineapple-assoc-manager\\build\\MyApp.exe\",0"

[HKEY_CURRENT_USER\Software\Classes\MyApp.jpg\shell\open\command]
@="\"C:\\Users\\Gary\\Sources\\pineapple-assoc-manager\\build\\MyApp.exe\" \"%1\""

; ----------------------------------------------------------
; 4. 让系统知道 MyApp 支持这些扩展名（打开方式菜单）
; ----------------------------------------------------------

[HKEY_CURRENT_USER\Software\Classes\.jpg\OpenWithProgids]
"MyApp.jpg"=hex(0):
```
