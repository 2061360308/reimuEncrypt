# ✨ reimuEncrypt

[![MIT License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![GitHub stars](https://img.shields.io/github/stars/2061360308/reimuEncrypt.svg?style=social)](https://github.com/2061360308/reimuEncrypt/stargazers)

> 一个专为静态博客内容加密与提取设计的高性能 C++ 工具  
> 提供多平台二进制文件，完美适配中文路径与 UTF-8 内容

---

## 🚀 项目简介

**reimuEncrypt** 是一款面向 Hugo 等静态博客系统的内容加密工具。  
它支持通过 CSS 选择器批量提取 HTML 内容、AES 加密、Base64 编码，并可灵活配置加密字段和模板。  
项目专为中文环境优化，兼容 Windows/Linux/macOS，适合博客内容加密场景。

---

## 🛠️ 功能特性

- 🌐 **跨平台支持**：Windows / Linux / macOS
- 🔒 **AES 加密**：安全保护敏感内容
- 🏷️ **CSS 选择器提取**：精准获取页面元素
- 📝 **Base64 编码**：便于前端解密与传输
- 🧩 **灵活配置**：支持多文章、多字段批量处理
- 🀄 **中文路径无忧**：全面支持 UTF-8 路径与内容
- 📋 **详细日志**：控制台简洁，log.txt 记录详细

---

## 📦 依赖环境

- C++17 标准
- [nlohmann/json](https://github.com/nlohmann/json)（JSON 解析）
- [Crypto++](https://www.cryptopp.com/)（加密）
- [Lexbor](https://github.com/lexbor/lexbor)（HTML 解析）
- [cpp-base64](https://github.com/ReneNyffenegger/cpp-base64)（Base64编码）
- Windows 下需 VS2019+，支持 UTF-8 路径

---

## 🏗️ 编译方法

### Windows

1. 安装 Visual Studio 2019 或更高版本
2. 配置 Crypto++、nlohmann/json、Lexbor 头文件与库
3. 使用 CMake 或 VS 打开项目编译

### Linux/macOS

```bash
sudo apt install g++ cmake libcryptopp-dev
# 或自行编译依赖
mkdir build && cd build
cmake ..
make
```


## ⚡ 快速上手

1. **准备配置文件 `encrypt.json`**  
   示例：（完整请见[example\encrypt.json](.\example\encrypt.json)）

```json
    {
      "defaultPassword": "123456",
      "template": "encrypt/index.html",
      "rootDir": "../",
      "baseURL": "https://example.org/",
      "encrypted": [
        { "name": "title", "selector": "#header-title h1", "encrypt": false },
        { "name": "sidebar", "selector": "#sidebar", "encrypt": true }
      ],
      "articles": [
        {
          "title": "Markdown Basic Elements",
          "permalink": "https://example.org/post/main/",
          "filePath": "post/main/index.html",
          "password": "secretpassword123"
        }
      ]
    }
```

encrypt.json 用于指导加密与替换，内容一般由`hugo`配置自动遍历生成

`articles`存放所有需要加密的文件夹信息，reimuEncrypt会依次处理。

对于每一个article,按照`encrypted`描述进行进一步处理，其用来指导需要从原始文件中提取哪些信息，并根据`encrypt`字段决定提取出内容是否进行加密，提取数据会与name相对应（{name:value}），上述最终得到结果是

```json
{
    "title": "提取到的title（原始文本内容）",
    "sidebar": "提取到的sidebar（base64格式密文）"
}
```

1. **准备模板文件 `index.html`**

reimuEncrypt 不关心模板文件的具体内容，在提取文章数据后会将其写入模板文件，最终结果替换原有位置html文件。

至于模板文件，你可以按照需求自由编写，例如提供一个密码框，在输入正确密码后数据并将其显示在页面上，这一步和编写普通html类似，只是记得留下占位符方便reimuEncrypt注入数据。

```js
let data = __ENCRYPT_DATA__;
```
> __ENCRYPT_DATA__ 会被替换为json数据，在js中操作十分方便

有关解密代码可以参照[example/decrypt.js](./example/decrypt.js)

1. **运行加密工具**

```sh
./reimuEncrypt encrypt.json
```
在[Releases](https://github.com/2061360308/reimuEncrypt/releases)页面下载对应版本可执行文件

2. **查看输出与日志**

- 生成的加密内容文件
- 日志文件 `log.txt`

## 📁 目录结构

```
reimuEncrypt/
├── aceEncrypt.cpp 加密
├── praseHtml.cpp  解析并提取html
├── tool.cpp       辅助函数
├── main.cpp       入口
├── README.md
├── deps           项目依赖
├── example        示例内容
└── ...
```

## Hugo主题集成指南

需要配置主题以便每次构建后自动生成`模板文件`以及`encrypt.json`
> 模板文件可以是静态的HTML，这里假设我们需要根据用户配置选项动态生成

在主题目录的`layouts\encrypt`下新建两个文件用于指导文件生成：
- `section.encrypt.json`
- `section.html`

为了让hugo生成对应文件，我们在博客目录下还需要创建一篇文章`content\encrypt\_index.md`

这一步只会生成html，为了生成json文件需要配置`hugo.toml`

```toml
[outputs]
home = ["Algolia", "HTML", "RSS"]
section = ["HTML", "Encrypt"] // 添加这一行，意思是不光生成html还要生成Encrypt

// 下面的也需要添加
[outputFormats.Encrypt]
mediaType = "application/json"
baseName = "encrypt"
isPlainText = true
notAlternative = true
```

这样一来就会在`public/encrypt`目录下生成需要的两个文件，后续在执行hugo build后调用本程序即可完成加密

## 📜 License

MIT

## 🤝 贡献

欢迎 issue、PR 或建议！如有问题可通过 GitHub Issues 联系。