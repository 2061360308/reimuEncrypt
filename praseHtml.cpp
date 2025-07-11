#include <lexbor/html/html.h>
#include "lexbor/css/css.h"
#include "lexbor/selectors/selectors.h"
#include <lexbor/core/fs.h>
#include "lexbor/html/interface.h"
#include "lexbor/dom/interfaces/element.h"

#include "praseHtml.h"
#include "tool.h"

using namespace std;

// 创建 Document 对象
LexborDocument::LexborDocument(const std::string& html) {
    document_ = lxb_html_document_create();
    if (document_) {
        lxb_status_t status = lxb_html_document_parse(document_, 
                                        (const lxb_char_t*)html.c_str(), 
                                        html.length());
        if (status != LXB_STATUS_OK) {
            cout << "HTML解析失败" << endl;
            logToFile("HTML解析失败", LogLevel::ERROR);
        }
    }
}
LexborDocument::~LexborDocument() {
    if (document_) lxb_html_document_destroy(document_);
}
std::shared_ptr<LexborDocument> LexborDocument::fromFile(const std::string& filePath) {
    string htmlContent = readFileToString(filePath);
    if (htmlContent.empty()) {
        cerr << "无法读取HTML文件或文件为空: " << filePath << endl;
        logToFile("无法读取HTML文件或文件为空: " + filePath, LogLevel::ERROR);
        return nullptr;
    }
    return std::make_shared<LexborDocument>(htmlContent);
}
std::shared_ptr<LexborNode> LexborDocument::root() {
    if (!document_) return nullptr;
    return std::make_shared<LexborNode>(document_, lxb_dom_interface_node(document_));
}

LexborNode::LexborNode(lxb_html_document_t* doc, lxb_dom_node_t* node)
    : document_(doc), node_(node) {}
LexborNode::~LexborNode() {}

std::shared_ptr<LexborNode> LexborNode::querySelector(const std::string& selector) {
    // 只返回第一个匹配
    lxb_status_t status;
    lxb_css_parser_t* parser = lxb_css_parser_create();
    lxb_css_parser_init(parser, nullptr);
    lxb_css_selectors_t* css_selectors = lxb_css_selectors_create();
    lxb_css_selectors_init(css_selectors);
    lxb_css_parser_selectors_set(parser, css_selectors);
    lxb_selectors_t* selectors = lxb_selectors_create();
    lxb_selectors_init(selectors);

    lxb_css_selector_list_t* list = lxb_css_selectors_parse(
        parser, (const lxb_char_t*)selector.c_str(), selector.length());
    if (!list) {
        lxb_selectors_destroy(selectors, true);
        lxb_css_selectors_destroy(css_selectors, true);
        lxb_css_parser_destroy(parser, true);
        return nullptr;
    }

    lxb_dom_collection_t* collection = lxb_dom_collection_create(lxb_dom_interface_document(document_));
    lxb_dom_collection_init(collection, 16);

    struct CollectNodes {
        static lxb_status_t callback(lxb_dom_node_t *node, lxb_css_selector_specificity_t, void *ctx) {
            lxb_dom_collection_t *collection = (lxb_dom_collection_t *)ctx;
            lxb_dom_collection_append(collection, node);
            return LXB_STATUS_OK;
        }
    };

    status = lxb_selectors_find(selectors, node_, list, CollectNodes::callback, collection);

    std::shared_ptr<LexborNode> result = nullptr;
    if (lxb_dom_collection_length(collection) > 0) {
        lxb_dom_node_t* child = lxb_dom_collection_node(collection, 0);
        result = std::make_shared<LexborNode>(document_, child);
    }

    lxb_dom_collection_destroy(collection, true);
    lxb_selectors_destroy(selectors, true);
    lxb_css_selectors_destroy(css_selectors, true);
    lxb_css_parser_destroy(parser, true);
    return result;
}

std::vector<std::shared_ptr<LexborNode>> LexborNode::querySelectorAll(const std::string& selector) {
    std::vector<std::shared_ptr<LexborNode>> results;
    lxb_status_t status;
    lxb_css_parser_t* parser = lxb_css_parser_create();
    lxb_css_parser_init(parser, nullptr);
    lxb_css_selectors_t* css_selectors = lxb_css_selectors_create();
    lxb_css_selectors_init(css_selectors);
    lxb_css_parser_selectors_set(parser, css_selectors);
    lxb_selectors_t* selectors = lxb_selectors_create();
    lxb_selectors_init(selectors);

    lxb_css_selector_list_t* list = lxb_css_selectors_parse(
        parser, (const lxb_char_t*)selector.c_str(), selector.length());
    if (!list) {
        lxb_selectors_destroy(selectors, true);
        lxb_css_selectors_destroy(css_selectors, true);
        lxb_css_parser_destroy(parser, true);
        return results;
    }

    lxb_dom_collection_t* collection = lxb_dom_collection_create(lxb_dom_interface_document(document_));
    lxb_dom_collection_init(collection, 16);

    struct CollectNodes {
        static lxb_status_t callback(lxb_dom_node_t *node, lxb_css_selector_specificity_t, void *ctx) {
            lxb_dom_collection_t *collection = (lxb_dom_collection_t *)ctx;
            lxb_dom_collection_append(collection, node);
            return LXB_STATUS_OK;
        }
    };

    status = lxb_selectors_find(selectors, node_, list, CollectNodes::callback, collection);

    for (size_t i = 0; i < lxb_dom_collection_length(collection); ++i) {
        lxb_dom_node_t* child = lxb_dom_collection_node(collection, i);
        results.push_back(std::make_shared<LexborNode>(document_, child));
    }

    lxb_dom_collection_destroy(collection, true);
    lxb_selectors_destroy(selectors, true);
    lxb_css_selectors_destroy(css_selectors, true);
    lxb_css_parser_destroy(parser, true);
    return results;
}

// 获取父节点
std::shared_ptr<LexborNode> LexborNode::parent() {
    if (!node_ || !node_->parent) return nullptr;
    return std::make_shared<LexborNode>(document_, node_->parent);
}

// 获取所有子节点
std::vector<std::shared_ptr<LexborNode>> LexborNode::children() {
    std::vector<std::shared_ptr<LexborNode>> result;
    if (!node_) return result;
    for (lxb_dom_node_t* child = node_->first_child; child; child = child->next) {
        result.push_back(std::make_shared<LexborNode>(document_, child));
    }
    return result;
}


std::string LexborNode::getContent() {
    lexbor_str_t str = {0};
    std::string result;
    if (lxb_html_serialize_tree_str(node_, &str) == LXB_STATUS_OK && str.data) {
        result.assign((const char*)str.data, str.length);
        lexbor_str_destroy(&str, document_->dom_document.text, false);
    }
    return result;
}

void LexborNode::setInnerHtml(const std::string& html) {
    // 只对元素节点有效
    if (node_ && node_->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        lxb_html_element_inner_html_set(
            (lxb_html_element_t*)node_,
            (const lxb_char_t*)html.c_str(),
            html.length()
        );
    }
}

// 设置outerHTML
void LexborNode::setOuterHtml(const std::string& html) {
    if (!node_ || !node_->parent) return;
    lxb_dom_node_t* fragment = lxb_html_document_parse_fragment(
        document_, (lxb_dom_element_t*)node_, (const lxb_char_t*)html.c_str(), html.length());
    if (!fragment) return;
    lxb_dom_node_t* child = fragment->first_child;
    lxb_dom_node_t* next = nullptr;
    while (child) {
        next = child->next;
        lxb_dom_node_insert_before(node_, child);
        child = next;
    }
    lxb_dom_node_remove(node_);
}

// 替换所有符合条件的子节点
void LexborNode::replaceAll(const std::string& selector, const std::string& html) {
    auto nodes = this->querySelectorAll(selector);
    for (auto& n : nodes) {
        n->setOuterHtml(html);
    }
}

// 在当前节点前插入
void LexborNode::insertBefore(const std::string& html) {
    if (!node_ || !node_->parent) return;
    lxb_dom_node_t* fragment = lxb_html_document_parse_fragment(
        document_, (lxb_dom_element_t*)node_, (const lxb_char_t*)html.c_str(), html.length());
    if (!fragment) return;
    lxb_dom_node_t* child = fragment->first_child;
    lxb_dom_node_t* next = nullptr;
    while (child) {
        next = child->next;
        lxb_dom_node_insert_before(node_, child);
        child = next;
    }
}

// 在当前节点后插入
void LexborNode::insertAfter(const std::string& html) {
    if (!node_ || !node_->parent) return;
    lxb_dom_node_t* fragment = lxb_html_document_parse_fragment(
        document_, (lxb_dom_element_t*)node_, (const lxb_char_t*)html.c_str(), html.length());
    if (!fragment) return;
    lxb_dom_node_t* ref = node_->next;
    lxb_dom_node_t* child = fragment->first_child;
    lxb_dom_node_t* next = nullptr;
    while (child) {
        next = child->next;
        if (ref)
            lxb_dom_node_insert_before(ref, child);
        else
            lxb_dom_node_insert_child(node_->parent, child);
        child = next;
    }
}

// 在子节点最前方插入
void LexborNode::prependHtml(const std::string& html) {
    if (!node_ || node_->type != LXB_DOM_NODE_TYPE_ELEMENT) return;
    lxb_dom_node_t* fragment = lxb_html_document_parse_fragment(
        document_, (lxb_dom_element_t*)node_, (const lxb_char_t*)html.c_str(), html.length());
    if (!fragment) return;
    lxb_dom_node_t* ref = node_->first_child;
    lxb_dom_node_t* child = fragment->first_child;
    lxb_dom_node_t* next = nullptr;
    while (child) {
        next = child->next;
        if (ref)
            lxb_dom_node_insert_before(ref, child);
        else
            lxb_dom_node_insert_child(node_, child);
        child = next;
    }
}

// 在子节点最后插入
void LexborNode::appendHtml(const std::string& html) {
    if (!node_ || node_->type != LXB_DOM_NODE_TYPE_ELEMENT) return;
    lxb_dom_node_t* fragment = lxb_html_document_parse_fragment(
        document_, (lxb_dom_element_t*)node_, (const lxb_char_t*)html.c_str(), html.length());
    if (!fragment) return;
    lxb_dom_node_t* child = fragment->first_child;
    lxb_dom_node_t* next = nullptr;
    while (child) {
        next = child->next;
        lxb_dom_node_insert_child(node_, child);
        child = next;
    }
}

