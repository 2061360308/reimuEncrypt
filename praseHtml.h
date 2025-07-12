#pragma once

#include <lexbor/html/html.h>
#include "lexbor/css/css.h"
#include <iostream>
#include <string>
#include <memory>
#include <vector>

class LexborNode;

/**
 * LexborDocument：封装HTML文档的加载与根节点访问
 *
 * 用于加载HTML字符串或文件，管理文档生命周期，并提供根节点访问接口。
 */
class LexborDocument {
public:
    /**
     * 构造函数
     * @param html HTML字符串内容，用于初始化文档
     */
    LexborDocument(const std::string& html);

    /**
     * 析构函数
     * 释放文档相关资源
     */
    ~LexborDocument();

    /**
     * 从文件加载HTML并创建LexborDocument对象
     * @param filePath HTML文件路径
     * @return 智能指针，指向新建的LexborDocument对象，加载失败返回nullptr
     */
    static std::shared_ptr<LexborDocument> fromFile(const std::string& filePath);

    /**
     * 获取文档的根节点（通常为<html>节点）
     * @return 根节点LexborNode智能指针
     */
    std::shared_ptr<LexborNode> root();

private:
    lxb_html_document_t* document_; ///< 底层Lexbor HTML文档指针
};

/**
 * LexborNode：封装单个HTML节点的操作与遍历
 *
 * 支持CSS选择器查找、父子节点遍历、内容获取与设置、节点插入与替换等常用DOM操作。
 */
class LexborNode {
public:
    /**
     * 构造函数
     * @param doc  所属HTML文档指针
     * @param node 节点指针
     */
    LexborNode(lxb_html_document_t* doc, lxb_dom_node_t* node);
    ~LexborNode();

    /**
     * 查询第一个匹配的子节点
     * @param selector CSS选择器
     * @return 匹配的LexborNode智能指针，未找到返回nullptr
     */
    std::shared_ptr<LexborNode> querySelector(const std::string& selector);

    /**
     * 查询所有匹配的子节点
     * @param selector CSS选择器
     * @return 匹配的LexborNode智能指针数组
     */
    std::vector<std::shared_ptr<LexborNode>> querySelectorAll(const std::string& selector);

    /**
     * 获取父节点
     * @return 父节点LexborNode智能指针，若无父节点返回nullptr
     */
    std::shared_ptr<LexborNode> parent();

    /**
     * 获取所有直接子节点
     * @return 子节点LexborNode智能指针数组
     */
    std::vector<std::shared_ptr<LexborNode>> children();

    /**
     * 获取节点内容（含标签）
     * @return 节点的HTML字符串
     */
    std::string getHtml();

    /**
     * 获取节点内容（不含标签）
     * @return 节点的文本内容字符串
     */
    std::string getContent();

    /**
     * 设置节点的innerHTML（替换所有子节点）
     * @param html 新的HTML内容
     */
    void setInnerHtml(const std::string& html);

    /**
     * 设置节点的outerHTML（用新内容替换自身）
     * @param html 新的HTML内容
     */
    void setOuterHtml(const std::string& html);

    /**
     * 批量替换所有匹配selector的子节点
     * @param selector CSS选择器
     * @param html 替换用的新HTML内容
     */
    void replaceAll(const std::string& selector, const std::string& html);

    /**
     * 在当前节点前插入HTML片段
     * @param html 要插入的HTML内容
     */
    void insertBefore(const std::string& html);

    /**
     * 在当前节点后插入HTML片段
     * @param html 要插入的HTML内容
     */
    void insertAfter(const std::string& html);

    /**
     * 在子节点最前方插入HTML片段
     * @param html 要插入的HTML内容
     */
    void prependHtml(const std::string& html);

    /**
     * 在子节点最后插入HTML片段
     * @param html 要插入的HTML内容
     */
    void appendHtml(const std::string& html);

    /**
     * 获取底层原始节点指针
     * @return lxb_dom_node_t* 指针
     */
    lxb_dom_node_t* raw() { return node_; }

private:
    lxb_html_document_t* document_;
    lxb_dom_node_t* node_;
};