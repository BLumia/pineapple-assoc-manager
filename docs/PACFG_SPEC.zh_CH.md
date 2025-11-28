# PACFG 格式说明

`.pacfg` 是一个 `ini` 格式的文件，用以描述“菠萝文件格式管理器”所需帮你管理的目标程序的文件扩展名相关的属性信息。

## 配置文件示例

```ini
; 顶层配置
targetApp=passoc.exe ; 目标程序的可执行文件名，位置相对于 .pacfg 所在目录
friendlyAppName=Pineapple Assoc Manager ; 目标程序的友好名称
friendlyAppName[zh_CN]=菠萝格式关联管理器 ; 可以添加多语言的支持
openCommand="\"{targetAppFullPath}\" \"%1\"" ; 打开文件的命令（若 ProgId 内没有指定则使用此命令）

[ProgId/pacfg] ; 文件格式（对应注册表的 ProgId）
name=Pineapple Association Configuration File ; 此扩展名对用户的显示名称
name[zh_CN]=菠萝格式关联配置文件 ; 同样支持多语言本地化支持
extensions=pacfg ; 扩展名列表，多个扩展名时用半角逗号分隔。省略此字段时，将使用 ProgId 的名称作为扩展名。
openCommand="\"{targetAppFullPath}\" -c \"%1\"" ; 打开文件的命令，可选，不提供则使用顶层 openCommand 的内容
icon=pacfg.ico ; 图标（可选）
```

## 配置文件内容说明和注意事项

0. `.pacfg` 配置文件必须放置于与目标文件同级的目录下，其他用法不受支持。
1. `targetApp` 的值必须和目标文件的可执行文件名称完全一致，例如希望为 `ppic.exe` 关联文件扩展名，则 `targetApp` 的值必须为 `ppic.exe`。
2. `friendlyAppName` 不可省略，将被用于注册此程序所支持的扩展名列表信息。
3. `openCommand` 定义打开文件所使用的打开命令，顶层定义默认打开文件的方式，也可针对每组 `ProgId` 单独定义。
   - `{targetAppFullPath}` 会被替换为 `targetApp` 的完整路径。
   - 由于配置文件是 ini 文件，请注意转义其中需要被转义的字符，例如双引号 `"` 需要转义为 `\"`。
4. `ProgId` 可以有多组，`[ProgId/{ProgIdName}]` 名称可以自定义，但建议和该组其中的一个实际扩展名保持一致。
5. `extensions` 可以有多个扩展名，用半角逗号分隔，半角逗号后不加多余空格，例如 `jpg,jpeg,jfif`。
6. `friendlyAppName` 与 `ProgId` 下的 `name` 支持本地化。实际写入到注册表中的名称是匹配到的本地化名称。
7. `icon` 可以指定图标文件。