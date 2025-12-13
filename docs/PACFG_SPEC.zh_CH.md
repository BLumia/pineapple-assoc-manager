# PACFG 格式说明

`.pacfg` 是一个 `ini` 格式的文件，用以描述“菠萝文件格式管理器”所需帮你管理的目标程序的文件扩展名相关的属性信息。

## 配置文件示例

```ini
; 顶层配置
targetApp=passoc.exe
friendlyAppName=Pineapple Assoc Manager
friendlyAppName[zh_CN]=菠萝格式关联管理器 ; 可以添加多语言的支持
openCommand="\"{targetAppFullPath}\" \"%1\"" ; 打开文件的命令（若 ProgId 内没有指定则使用此命令）
genericFileIcon=icons/generic.ico

[ProgId/pacfg]
name=Pineapple Association Configuration File ; 此扩展名对用户的显示名称
name[zh_CN]=菠萝格式关联配置文件 ; 同样支持多语言本地化支持
extensions=pacfg
openCommand="\"{targetAppFullPath}\" -c \"%1\"" ; 打开文件的命令，可选，不提供则使用顶层 openCommand 的内容
icon=icons/pacfg.ico ; 图标（可选）
```

## 配置文件内容说明和注意事项

### 顶层设置

| 键 | 描述 |
| :--- | :--- |
| `targetApp` | 目标程序的可执行文件名，位置相对于 `.pacfg` 所在目录 (例如 `myapp.exe`) |
| `friendlyAppName` | 目标程序的友好名称，会显示在 Windows 设置中的“默认应用程序”等位置 |
| `openCommand` | 打开文件的默认命令（若 ProgId 内没有指定则使用此命令）。`{targetAppFullPath}` 将被替换为 `targetApp` 的完整绝对路径 |
| `genericFileIcon` | 当 ProgId 的图标缺失时的 fallback 图标（可选），默认为 `icons/generic.ico` |

### ProgId 部分

使用 `[ProgId/<ID>]` 区块定义文件格式支持。

| 键 | 描述 |
| :--- | :--- |
| `name` | 此扩展名对用户的显示名称 (例如 "JPEG Image") |
| `extensions` | 扩展名列表，多个扩展名时用半角逗号分隔 (例如 `jpg,jpeg`)。省略此字段时，将使用 ProgId 的名称作为扩展名 |
| `icon` | 图标文件路径（可选），默认为 `icons/<ID>.ico` |
| `openCommand` | 打开文件的默认命令（可选），不提供时使用顶层设置的内容 |

### 注意事项

0. `.pacfg` 配置文件必须放置于与目标程序同级的目录下，其他用法不受支持。
1. `targetApp` 的值必须和目标程序的可执行文件名称完全一致，例如希望为 `ppic.exe` 关联文件扩展名，则 `targetApp` 的值必须为 `ppic.exe`。
2. `friendlyAppName` 不可省略，将被用于注册此程序所支持的扩展名列表信息。
3. `openCommand` 定义打开文件所使用的打开命令，顶层定义默认打开文件的方式，也可针对每组 `ProgId` 单独定义。
   - `{targetAppFullPath}` 会被替换为 `targetApp` 的完整路径。
   - 由于配置文件是 ini 文件，请注意转义其中需要被转义的字符，例如双引号 `"` 需要转义为 `\"`。
4. `ProgId` 可以有多组，`[ProgId/{ProgIdName}]` 名称可以自定义，但建议和该组其中的一个实际扩展名保持一致。
5. `extensions` 可以有多个扩展名，用半角逗号分隔，半角逗号后不加多余空格，例如 `jpg,jpeg,jfif`。
6. `friendlyAppName` 与 `ProgId` 下的 `name` 支持本地化。实际写入到注册表中的名称是匹配到的本地化名称。
7. `icon` 是可选字段，可以指定图标文件，路径相对于目标程序所在目录，暂不支持内嵌资源文件。当此字段被省略时，默认值为 `icons/{ProgIdName}.ico`。若找不到此图标文件，则尝试使用 `genericFileIcon` 指定的图标。若仍找不到，则不会注册此文件格式的图标（系统表现会使用目标程序本身的图标）。
8. `genericFileIcon` 是可选的顶层字段，用于指定当 ProgId 特定的图标无法找到时的通用 fallback 图标。默认值为 `icons/generic.ico`。
