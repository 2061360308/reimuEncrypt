# reimuEncrypt 主流程说明

简要介绍 main.cpp 的整体处理流程，帮助开发者快速理解 reimuEncrypt 的加密处理逻辑。

---

## 1. 启动与参数解析

- 程序启动后，首先记录启动日志。
- 调用 `parseInputPath` 解析命令行参数，确定加密配置文件（如 `encrypt.json`）和根目录路径。
    - 支持传入文件夹或 JSON 文件路径。
    - 如果未传参数，默认在当前目录查找 `encrypt.json`。

---

## 2. 加载加密配置

- 调用 `loadEncryptConfig` 读取并解析 JSON 配置文件，填充全局 `EncryptConfig config` 对象。
- 如果配置文件读取失败，输出错误并退出。

---

## 3. 处理每一篇文章

- 遍历配置中的 `articles` 列表，对每篇文章调用 `processArticle` 进行加密处理。

### `processArticle` 主要流程：

1. **加载 HTML 文件**
    - 根据 `filePath` 加载 HTML 文件，构建 DOM 树。
    - 若文件不存在或解析失败，输出错误并跳过。

2. **选择加密配置**
    - 根据 `article.all` 字段，选择全局加密规则（`encryptedAll`）或局部加密规则（`encryptedPartial`）。

3. **遍历加密规则**
    - 对每个加密规则（`EncryptedItem`）：
        - 日志输出当前处理的规则信息。
        - 设置默认加密密码（优先用文章密码，否则用全局默认密码）。
        - 根据 `selectAll` 字段，决定是处理所有匹配节点还是第一个匹配节点。
        - 对每个目标节点，调用 `processNode` 进行加密处理，并将加密结果写入 `result` JSON。

4. **写入加密数据到 HTML**
    - 在 `<head>` 节点插入包含加密数据的 `<script>` 标签和解密 JS 代码。
    - 如果未找到 `<head>` 节点，输出错误。

5. **保存加密后的 HTML**
    - 将修改后的 HTML 内容写回原文件。
    - 日志记录写入结果。

---

### `processNode` 主要流程：

1. 根据加密规则（`EncryptedItem`）和默认密码，确定当前节点的加密密码（支持从节点内再查找密码）。
2. 获取节点内容，使用 AES 加密并进行 base64 编码，得到加密后的字符串。
3. 根据配置，决定是替换节点的 innerHTML 还是整个 outerHTML，并记录日志。
4. 返回加密后的 base64 字符串，供上层写入结果 JSON。


## 4. 删除加密配置文件

- 调用 `removeEncryptConfigFile` 删除已处理的配置文件，避免重复加密。

---

## 5. 结束日志

- 输出处理完成日志，程序退出。

---

## 主要函数说明

- **parseInputPath**：解析命令行参数，确定配置文件和根目录路径。
- **loadEncryptConfig**：读取并解析加密配置 JSON 文件。
- **processArticle**：处理单篇文章的加密流程。
- **processNode**：对单个节点进行加密、内容替换等操作。
- **removeEncryptConfigFile**：删除加密配置文件。

---

## 总结

本程序实现了**批量 HTML 节点内容加密**、**自动插入解密脚本**、**配置驱动**的静态站点加密流程，适合博客、文档等场景的静态加密需求。  
通过合理的配置和流程控制，保证了加密处理的灵活性和安全性。