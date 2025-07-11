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

### 开发者集成
#### **1、配置主题模板生成配置文件 `encrypt.json`**

示例：（完整请见[example\encrypt.json](.\example\encrypt.json)）

```json
{
  "generatedAt": "2025-07-11T19:49:47+08:00",
  "totalCount": 1,
  "defaultPassword": "123456",  // 全局默认密码
  "encrypted-all": [            // 配置整篇文章需要加密时的操作
    {
      "name": "article",        // 传回数据时的键名
      "selector": "article",    // CSS选择器
      "selectAll": false,       // false得到article的值为字符，true时为[字符串1，字符串2, ...]，若选择器会查找到多个元素且希望全部处理则应为true
      "replace": {
        "innerHTML": false,     // 加密后替换文档原来元素的方式，false时连带标签一起替换类似js中outerHTML，ture时类似JS中innerHTML函数的替换方式
        "content": ""           // 用来替换的内容，如果只是需要删除则设置为""空字符串，否则可以设置为具体需要的HTML片段
      },
      "password": ""            // 如果需要从生成HTML中查找密码，则这里配置为存储密码元素的CSS选择器
    },
    {
      "name": "sidebar",
      "selector": "#TableOfContents",
      "selectAll": false,
      "replace": {
        "innerHTML": false,
        "content": ""
      }
    }
  ],
  "encrypted-partial": [],      // 配置文章局部需要加密时的操作
  "articles": [                 // 需要加密处理的文章列表
    {
      "title": "Markdown Basic Elements",                // 文章标题
      "filePath": "post/main/index.html",                // 相对于配置文件（encrypt.json）的路径
      "uniqueID": "51f72b80a80d6a49000862e4282ab7a0",    // 唯一id
      "password": "secretpassword123",                   // 文章的密码
      "all": true                                        // 是否加密整篇文章，false时标识局部加密会使用encrypted-partial中的配置进行加密
    }
  ]
}

```

**encrypt.json** 用于指导加密与替换，内容一般由`hugo`配置自动遍历生成

> **encrypt.json** 生成位置只能是网站根目录

`articles`存放所有需要加密的文件夹信息，reimuEncrypt会依次处理。

加密所用密码的查找顺序为：**encrypted-all/encrypted-partial** password -> **articles** password -> defaultPassword

> ***注意***：在**encrypted-all/encrypted-partial**设置**password**选项后，这个选择器只会在当前**selector**配置选择器选中元素中的对应子元素下进行查找。
> 
> 这个属性设计目的是在文章局部有多处需要加密时能够单独设置每个加密片段的密码。


#### **2、文章布局模板下添加对应js逻辑**

在编写js控制页面解密行为前，先了解reimuEncrypt已经在html的head末尾添加的两段辅助脚本
```html
<html>
  <head>
    ... 
    <script> var __ENCRYPT_DATA__ = {} </script>
    <script>
      /**
      * 解密函数
      * @param {*} base64Data base64编码的加密数据
      * @param {*} password 解密密码
      * @returns {Promise<string>} 解密后的明文数据
      */
      async function encrypt(base64Data,password){
        ...
      }
    </script>
  </head>
  ...
</html>
```

**__ENCRYPT_DATA__**: 这个变量存储了加密后的HTML内容，键名是你在**encrypted-all/encrypted-partial**中定义的**name**

**encrypt**: 这是提供的解密函数，按照示例的json配置 `encrypt(__ENCRYPT_DATA__.article, "secretpassword123")`

在后续你还要添加更多的逻辑来完善页面显示大致参考思路如下：
   1. 文档加载完成后判断是否有存储的密码，有的话调用**encrypt**函数尝试进行解密
   2. 解密失败或者没有记录密码，显示要求输入密码的占位元素
   3. 用户输入密码调用encrypt尝试解密
   4. 解密失败显示提示信息
   5. 解密成功，将结果显示到页面，并保存密码信息下次方便显示

### 用户使用
在主题集成后需要加密文章只需要在每次生成后，运行此加密工具即可，需要参数为**encrypt.json**的路径（或生成网站的根目录）

```sh
./reimuEncrypt encrypt.json
```
在[Releases](https://github.com/2061360308/reimuEncrypt/releases)页面下载对应版本可执行文件


要了解程序运行情况可以查看输出与日志
- 简略信息会在控制台输出
- 详细信息可以查看日志文件 `log.txt`

## 📁 目录结构

```
reimuEncrypt/
├── aceEncrypt.cpp 加密
├── praseHtml.cpp  解析并提取html
├── encryptConfig.cpp  # 加载加密配置json
├── tool.cpp       辅助函数
├── main.cpp       入口
├── README.md
├── deps           项目依赖
├── example        示例内容
└── ...
```

## Hugo主题集成详细指南

> 上述介绍了大概的集成思路，下面指定Hugo提供更加具体的集成方案

## 定义相关参数

为文章加密功能在站点配置以及文章front matter参数中规定相关参数

参考案例如下:

params.yaml
```yaml
########################################
# Encrypted Articles
########################################
encrypt:
  enable: true # enable encrypted articles
  defaultPassword: "123456" # default password for encrypted articles
```

front matter
```yaml
---
title: Markdown Basic Elements
description: 测试 Markdown 基本元素
date: 2022-06-09T20:12:52+08:00
lastmod: 2022-06-09T20:12:52+08:00
...
encrypt:
  enable: true
  password: "secretpassword123"
  all: true
---
```

### 添加 index.encrypt.json 文件

添加文件 `/layouts/_default/index.encrypt.json`
内容为`encrypt.json`的生成模板，示例如下（需要根据具体情况适配）：
```
{{- $encryptedArticles := slice -}}

{{- range .Site.RegularPages -}}
  {{- if and .Site.Params.encrypt.enable .Params.encrypt (eq .Params.encrypt.enable true) -}}
    {{- $article := dict 
      "title" .Title
      "relPermalink" .RelPermalink
      "uniqueID" .File.UniqueID
    -}}
    
    {{- if and .Params.encrypt.password (ne .Params.encrypt.password "") -}}
      {{- $article = merge $article (dict "password" .Params.encrypt.password) -}}
    {{- end -}}

    {{- if isset .Params.encrypt "all" -}}
      {{- $article = merge $article (dict "all" .Params.encrypt.all) -}}
    {{- else -}}
      {{- $article = merge $article (dict "all" false) -}}
    {{- end -}}
    
    {{- $encryptedArticles = $encryptedArticles | append $article -}}
  {{- end -}}
{{- end -}}

{
  "generatedAt": "{{ now.Format "2006-01-02T15:04:05Z07:00" }}",
  "totalCount": {{ len $encryptedArticles }},
  "defaultPassword": {{ .Site.Params.encrypt.defaultPassword | jsonify }},
  {{- /* 全篇文章加密配置  */ -}}
  "encrypted-all": [
    {
      "name": "article",        {{- /* 名称 */ -}}
      "selector": "article",    {{- /* 元素CSS选择器 */ -}}
      "selectAll": false,       {{- /* 是否查询并操作所有匹配的子节点 */ -}}
      "replace": {
        "innerHTML": false,     {{- /* true保留标签替换内部内容，false替换整个标签元素 */ -}}
        "content": ""           {{- /* 替换内容 */ -}}
      },
      "password": ""            {{- /* 如果需要从元素获取密码，提供CSS选择器（会从上级selector选中元素中查找） */ -}}
    },
    {
      "name": "sidebar",
      "selector": "#TableOfContents",
      "selectAll": false,
      "replace": {
        "innerHTML": false,
        "content": ""
      }
    }
  ],
  {{- /* 文章局部加密配置  */ -}}
  "encrypted-partial": [],
  {{- /* 加密文章列表  */ -}}
  "articles": [
    {{- range $index, $article := $encryptedArticles -}}
      {{- if gt $index 0 -}},{{- end -}}
      {
        "title": {{ $article.title | jsonify }},
        "filePath": {{ (print (replace $article.relPermalink "/" "" 1) "index.html") | jsonify }},
        "uniqueID": {{ $article.uniqueID | jsonify }}
        {{- if isset $article "password" -}}
        , "password": {{ $article.password | jsonify }}
        {{- end -}}
        {{- if isset $article "password" -}}
        , "all": {{ $article.all | jsonify }}
        {{- end -}}
      }
    {{- end -}}
  ]
}
```

### 配置 hugo.toml

在Hugo项目下的`hugo.toml`中进行配置

末尾添加以下内容
```toml
[outputFormats.Encrypt]
mediaType = "application/json"
baseName = "encrypt"
isPlainText = true
notAlternative = true
```

在**outputs**中的**home**后添加**Encrypt**
```
[outputs]
home = ["Algolia", "HTML", "RSS", "Encrypt"]


```

### 页面功能集成
参照上述**文章布局模板下添加对应js逻辑**的内容更改主题

## 📜 License

MIT

## 🤝 贡献

欢迎 issue、PR 或建议！如有问题可通过 GitHub Issues 联系。