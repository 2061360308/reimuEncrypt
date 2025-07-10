#include "praseHtml.h"
#include "tool.h"
#include <lexbor/html/html.h>
#include "lexbor/css/css.h"
#include "lexbor/selectors/selectors.h"

using namespace std;

// /**
//  * 读取整个文件内容到字符串
//  * 
//  * @param filePath 文件路径
//  * @param encoding 文件编码 (如 "utf-8", "gbk", "gb2312" 等)
//  * @return 文件内容字符串
//  */
// std::string readFileToString(const std::string& filePath, const std::string& encoding) {
//     std::ifstream file(filePath, std::ios::binary); // 使用二进制模式打开
//     if (!file.is_open()) {
//         std::cerr << "无法打开文件: " << filePath << std::endl;
//         return "";
//     }
    
//     // 读取文件全部内容到字符串
//     std::ostringstream ss;
//     ss << file.rdbuf();
//     std::string content = ss.str();
    
//     // 检测BOM标记并处理编码
//     if (content.size() >= 3 && 
//         static_cast<unsigned char>(content[0]) == 0xEF &&
//         static_cast<unsigned char>(content[1]) == 0xBB &&
//         static_cast<unsigned char>(content[2]) == 0xBF) {
//         // 移除UTF-8 BOM
//         content = content.substr(3);
//     }
    
//     // 如果指定了非UTF-8编码，尝试转换
//     if (encoding != "utf-8" && encoding != "UTF-8") {
//         std::cerr << "注意：目前只支持直接读取UTF-8编码。对于其他编码，请先使用外部工具转换。" << std::endl;
//         // 这里需要更复杂的编码转换库，比如iconv或ICU
//     }
    
//     return content;
// }

// /**
//  * 将字符串内容写入文件
//  */
// bool writeStringToFile(const std::string& filePath, const std::string& content) {
//     std::ofstream file(filePath);
//     if (!file.is_open()) {
//         std::cerr << "无法打开文件进行写入: " << filePath << std::endl;
//         return false;
//     }
    
//     file << content;
//     return file.good();
// }

// /**
//  * 使用Lexbor实现的CSS选择器解析函数
//  * 
//  * @param html HTML字符串
//  * @param selector CSS选择器
//  * @param index 要返回的元素索引(0-based)，默认为0（第一个匹配元素）
//  * @return 指定索引的匹配元素内容，如果不存在则返回空字符串
//  */
// std::string querySelector(const std::string& html, const std::string& selector, size_t index) 
// {
//     logToFile("开始解析HTML内容...", LogLevel::INFO);

//     // 解析和选择器相关的变量声明
//     lxb_status_t status;                            // 用于接收各API的返回状态
//     lxb_html_document_t* document = nullptr;        // HTML文档对象指针
//     lxb_dom_collection_t* collection = nullptr;     // 用于存放匹配元素的集合
//     lxb_css_selector_list_t* list = nullptr;        // CSS选择器列表
//     lxb_css_parser_t* parser = nullptr;             // CSS解析器对象
//     lxb_css_selectors_t* css_selectors = nullptr;   // CSS选择器对象
//     lxb_selectors_t* selectors = nullptr;           // 选择器对象
//     std::string result = "";                        // 最终返回的结果
    
//     // 输入参数检查
//     if (html.empty() || selector.empty()) {
//         cout << "HTML内容或选择器为空" << endl;
//         logToFile("HTML内容或选择器为空", LogLevel::ERROR);
//         return result;
//     }
    
//     // 创建HTML解析器并解析HTML
//     document = lxb_html_document_create();
//     if (document == nullptr) {
//         cout << "HTML文档创建失败" << endl;
//         logToFile("无法创建HTML文档", LogLevel::ERROR);
//         return result;
//     }
//     logToFile("HTML文档创建成功", LogLevel::DEBUG);

//     status = lxb_html_document_parse(document, 
//                                     (const lxb_char_t*)html.c_str(), 
//                                     html.length());
//     if (status != LXB_STATUS_OK) {
//         cout << "HTML解析失败" << endl;
//         logToFile("HTML解析失败", LogLevel::ERROR);
//         lxb_html_document_destroy(document);
//         return result;
//     }
//     logToFile("HTML解析成功", LogLevel::DEBUG);

//     // 创建CSS解析器
//     parser = lxb_css_parser_create();
//     if (parser == nullptr) {
//         cout << "CSS解析器创建失败" << endl;
//         logToFile("无法创建CSS解析器", LogLevel::ERROR);
//         lxb_html_document_destroy(document);
//         return result;
//     }
//     logToFile("CSS解析器创建成功", LogLevel::DEBUG);

//     status = lxb_css_parser_init(parser, nullptr);
//     if (status != LXB_STATUS_OK) {
//         cout << "CSS解析器初始化失败" << endl;
//         logToFile("CSS解析器初始化失败", LogLevel::ERROR);
//         lxb_css_parser_destroy(parser, true);
//         lxb_html_document_destroy(document);
//         return result;
//     }
//     logToFile("CSS解析器初始化成功", LogLevel::DEBUG);

//     // 创建CSS选择器
//     css_selectors = lxb_css_selectors_create();
//     if (css_selectors == nullptr) {
//         cout << "CSS选择器对象创建失败" << endl;
//         logToFile("无法创建CSS选择器对象", LogLevel::ERROR);
//         lxb_css_parser_destroy(parser, true);
//         lxb_html_document_destroy(document);
//         return result;
//     }
//     logToFile("CSS选择器对象创建成功", LogLevel::DEBUG);

//     status = lxb_css_selectors_init(css_selectors);
//     if (status != LXB_STATUS_OK) {
//         cout << "CSS选择器初始化失败" << endl;
//         logToFile("CSS选择器创建失败", LogLevel::ERROR);
//         lxb_css_selectors_destroy(css_selectors, true);
//         lxb_css_parser_destroy(parser, true);
//         lxb_html_document_destroy(document);
//         return result;
//     }
//     logToFile("CSS选择器初始化成功", LogLevel::DEBUG);

//     // 设置选择器到解析器
//     lxb_css_parser_selectors_set(parser, css_selectors);

//     // 创建selectors对象
//     selectors = lxb_selectors_create();
//     if (selectors == nullptr) {
//         cout << "选择器对象创建失败" << endl;
//         logToFile("无法创建选择器对象", LogLevel::ERROR);
//         lxb_css_selectors_destroy(css_selectors, true);
//         lxb_css_parser_destroy(parser, true);
//         lxb_html_document_destroy(document);
//         return result;
//     }
//     logToFile("选择器对象创建成功", LogLevel::DEBUG);

//     status = lxb_selectors_init(selectors);
//     if (status != LXB_STATUS_OK) {
//         cout << "选择器对象初始化失败" << endl;
//         logToFile("选择器对象初始化失败", LogLevel::ERROR);
//         lxb_selectors_destroy(selectors, true);
//         lxb_css_selectors_destroy(css_selectors, true);
//         lxb_css_parser_destroy(parser, true);
//         lxb_html_document_destroy(document);
//         return result;
//     }
//     logToFile("选择器对象初始化成功", LogLevel::DEBUG);

//     // 解析CSS选择器
//     list = lxb_css_selectors_parse(parser, 
//                                   (const lxb_char_t*)selector.c_str(),
//                                   selector.length());
//     if (parser->status != LXB_STATUS_OK || list == nullptr) {
//         cout << "CSS选择器解析失败" << endl;
//         logToFile("CSS选择器解析失败: " + selector, LogLevel::ERROR);
//         lxb_selectors_destroy(selectors, true);
//         lxb_css_selectors_destroy(css_selectors, true);
//         lxb_css_parser_destroy(parser, true);
//         lxb_html_document_destroy(document);
//         return result;
//     }
//     logToFile("CSS选择器解析成功: " + selector, LogLevel::DEBUG);

//     // 创建集合用于保存匹配的元素
//     collection = lxb_dom_collection_create(lxb_dom_interface_document(document));
//     if (collection == nullptr) {
//         cout << "DOM集合创建失败" << endl;
//         logToFile("无法创建DOM集合", LogLevel::ERROR);
//         lxb_selectors_destroy(selectors, true);
//         lxb_css_selectors_destroy(css_selectors, true);
//         lxb_css_parser_destroy(parser, true);
//         lxb_html_document_destroy(document);
//         return result;
//     }
//     logToFile("DOM集合创建成功", LogLevel::DEBUG);

//     status = lxb_dom_collection_init(collection, 16);
//     if (status != LXB_STATUS_OK) {
//         cout << "DOM集合初始化失败" << endl;
//         logToFile("DOM集合初始化失败", LogLevel::ERROR);
//         lxb_dom_collection_destroy(collection, true);
//         lxb_selectors_destroy(selectors, true);
//         lxb_css_selectors_destroy(css_selectors, true);
//         lxb_css_parser_destroy(parser, true);
//         lxb_html_document_destroy(document);
//         return result;
//     }
//     logToFile("DOM集合初始化成功", LogLevel::DEBUG);

//     // 自定义回调函数来收集匹配的元素
//     struct CollectNodes {
//         static lxb_status_t callback(lxb_dom_node_t *node, lxb_css_selector_specificity_t spec, void *ctx) {
//             lxb_dom_collection_t *collection = (lxb_dom_collection_t *)ctx;
//             lxb_dom_collection_append(collection, node);
//             return LXB_STATUS_OK;
//         }
//     };

//     // 使用CSS选择器查找元素
//     status = lxb_selectors_find(selectors, 
//                               lxb_dom_interface_node(document), 
//                               list, 
//                               CollectNodes::callback, 
//                               collection);
//     if (status != LXB_STATUS_OK) {
//         cout << "元素查找失败" << endl;
//         logToFile("选择器查找失败", LogLevel::ERROR);
//         lxb_dom_collection_destroy(collection, true);
//         lxb_selectors_destroy(selectors, true);
//         lxb_css_selectors_destroy(css_selectors, true);
//         lxb_css_parser_destroy(parser, true);
//         lxb_html_document_destroy(document);
//         return result;
//     }
//     logToFile("选择器查找成功", LogLevel::DEBUG);

//     // 控制台只输出数量
//     logToFile("找到匹配元素数量: " + std::to_string(lxb_dom_collection_length(collection)), LogLevel::INFO);

//     // 获取指定索引的元素
//     if (index < lxb_dom_collection_length(collection)) {
//         lxb_dom_node_t* node = lxb_dom_collection_node(collection, index);
//         if (node != nullptr) {
//             logToFile("获取到匹配的节点", LogLevel::DEBUG);
//             // 创建内存缓冲区用于保存序列化结果
//             lexbor_str_t str = {0};
            
//             // 使用 lxb_html_serialize_tree_str 确保序列化整个元素（包括标签本身）
//             status = lxb_html_serialize_tree_str(node, &str);
//             if (status == LXB_STATUS_OK && str.data != nullptr) {
//                 logToFile("序列化成功，结果长度: " + std::to_string(str.length), LogLevel::INFO);
//                 result = std::string((const char*)str.data, str.length);
//                 lexbor_str_destroy(&str, document->dom_document.text, false);
//             }
//             else {
//                 logToFile("序列化失败，错误码: " + std::to_string(status), LogLevel::ERROR);
//             }
//         }
//         else {
//             logToFile("获取的节点为空", LogLevel::WARN);
//         }
//     }
//     else {
//         logToFile("索引超出范围: " + std::to_string(index), LogLevel::WARN);
//     }
    
//     // 清理资源
//     lxb_dom_collection_destroy(collection, true);
//     lxb_selectors_destroy(selectors, true);
//     lxb_css_selectors_destroy(css_selectors, true);
//     lxb_css_parser_destroy(parser, true);
//     lxb_html_document_destroy(document);
    
//     return result;
// }

// 从文件获取HTML内容
// string getHtmlContent(const string& filePath, const string& selector) {
//     string htmlContent = readFileToString(filePath);
//     if (htmlContent.empty()) {
//         cerr << "无法读取HTML文件或文件为空: " << filePath << endl;
//         logToFile("无法读取HTML文件或文件为空: " + filePath, LogLevel::ERROR);
//         return "";
//     }
//     string result = querySelector(htmlContent, selector, 0);
//     logToFile("getHtmlContent: file=" + filePath + " selector=" + selector + " result.length=" + std::to_string(result.length()), LogLevel::DEBUG);
//     return result;
// }


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

std::string LexborNode::getContent() {
    lexbor_str_t str = {0};
    std::string result;
    if (lxb_html_serialize_tree_str(node_, &str) == LXB_STATUS_OK && str.data) {
        result.assign((const char*)str.data, str.length);
        lexbor_str_destroy(&str, document_->dom_document.text, false);
    }
    return result;
}

void LexborNode::setContent(const std::string& html) {
    // 只对元素节点有效
    if (node_ && node_->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        lxb_html_element_inner_html_set(
            (lxb_html_element_t*)node_,
            (const lxb_char_t*)html.c_str(),
            html.length()
        );
    }
}