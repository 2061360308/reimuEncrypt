#pragma once

#include <lexbor/html/html.h>
#include "lexbor/css/css.h"
#include <iostream>
#include <string>
#include <memory>
#include <vector>


// #include <string>
// #include <vector>
// #include <iostream>
// #include <fstream>
// #include <sstream>
// #include "lexbor/html/html.h"
// #include "lexbor/css/css.h"
// #include "lexbor/selectors/selectors.h"


// /**
//  * 使用Lexbor实现的CSS选择器解析函数
//  * 
//  * @param html HTML字符串
//  * @param selector CSS选择器
//  * @param index 要返回的元素索引(0-based)，默认为0（第一个匹配元素）
//  * @return 指定索引的匹配元素内容，如果不存在则返回空字符串
//  *
//  * @details 支持的CSS选择器语法：
//  *  - 基本选择器:
//  *    - 标签选择器: div, p, article
//  *    - ID选择器: #header, #content
//  *    - 类选择器: .container, .menu
//  *    - 通配符: *
//  * 
//  *  - 组合选择器:
//  *    - 后代选择器: div p
//  *    - 子元素选择器: div > p
//  *    - 相邻兄弟选择器: div + p
//  *    - 通用兄弟选择器: div ~ p
//  * 
//  *  - 属性选择器:
//  *    - [attr] - 具有attr属性的元素
//  *    - [attr=value] - attr属性值为value的元素
//  *    - [attr~=value] - attr属性值包含value单词的元素
//  *    - [attr^=value] - attr属性值以value开头的元素
//  *    - [attr$=value] - attr属性值以value结尾的元素
//  *    - [attr*=value] - attr属性值包含value子串的元素
//  * 
//  *  - 伪类选择器:
//  *    - :first-child, :last-child - 第一个/最后一个子元素
//  *    - :nth-child(n), :nth-of-type(n) - 第n个子元素/同类型第n个元素
//  *    - :not(selector) - 不匹配指定选择器的元素
//  * 
//  * @example
//  *  // 获取第一个文章元素
//  *  std::string article = querySelector(html, "article");
//  *  
//  *  // 获取ID为header的元素
//  *  std::string header = querySelector(html, "#header");
//  *  
//  *  // 获取所有菜单项中的第三个
//  *  std::string thirdMenuItem = querySelector(html, ".menu-item", 2); // 索引从0开始
//  */
// std::string querySelector(const std::string& html, const std::string& selector, size_t index = 0);

// // 从文件获取HTML内容
// string getHtmlContent(const string& filePath, const string& selector);

class LexborNode;

class LexborDocument {
public:
    LexborDocument(const std::string& html);
    ~LexborDocument();

    std::shared_ptr<LexborNode> root();

private:
    lxb_html_document_t* document_;
};

class LexborNode {
public:
    LexborNode(lxb_html_document_t* doc, lxb_dom_node_t* node);
    ~LexborNode();

    // 查询第一个匹配的子节点
    std::shared_ptr<LexborNode> querySelector(const std::string& selector);

    // 查询所有匹配的子节点
    std::vector<std::shared_ptr<LexborNode>> querySelectorAll(const std::string& selector);

    // 获取节点内容（含标签）
    std::string getContent();

    // 替换节点内容（innerHTML）
    void setContent(const std::string& html);

    // 获取底层原始指针（如有需要）
    lxb_dom_node_t* raw() { return node_; }

private:
    lxb_html_document_t* document_;
    lxb_dom_node_t* node_;
};
