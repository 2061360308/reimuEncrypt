#include <iostream>
#include <string>
#include "praseHtml.h"

void testLexborFromFile() {
    // 假设你的文件名为中文，例如 "测试文件.html"，并且文件内容为有效 HTML
    std::string filePath = u8"E:\\hugo-reimu\\public\\post\\main---副本\\index.html"; // u8 前缀确保是 UTF-8 字符串字面量

    auto doc = LexborDocument::fromFile(filePath);
    if (!doc) {
        std::cout << "打开文件失败: " << filePath << std::endl;
        return;
    }
    auto root = doc->root();
    auto node = root->querySelector("body");
    if (node) {
        std::cout << "body内容: " << node->getContent() << std::endl;
    } else {
        std::cout << "未找到body节点" << std::endl;
    }
}

void testLexbor() {
    using std::cout;
    using std::endl;
    std::string html =
        "<html><body>"
        "<div id='main'><div class='test'>Hello World</div><span>Span1</span></div>"
        "<div class='test'>Second</div>"
        "</body></html>";

    LexborDocument doc(html);
    auto root = doc.root();

    // 1. querySelector/querySelectorAll
    auto node = root->querySelector(".test");
    if (node) {
        cout << "[querySelector] .test: " << node->getContent() << endl;
    }
    auto allTests = root->querySelectorAll(".test");
    cout << "[querySelectorAll] .test count: " << allTests.size() << endl;

    // 2. parent/children
    if (node) {
        auto parent = node->parent();
        cout << "[parent] tag: " << (parent ? parent->getContent() : "null") << endl;
        auto children = parent->children();
        cout << "[children] count: " << children.size() << endl;
    }

    // 3. setInnerHtml
    if (node) {
        node->setInnerHtml("<b>Bold</b><i>Italic</i>");
        cout << "[setInnerHtml] .test: " << node->getContent() << endl;
    }

    // 4. setOuterHtml
    if (node) {
        node->setOuterHtml("<section class='test'>Replaced Outer</section>");
        cout << "[setOuterHtml] .test: " << root->getContent() << endl; // node已被替换，内容不可用
    }

    // 5. replaceAll
    root->replaceAll(".test", "<div class='test'>BatchReplace</div>");
    auto replaced = root->querySelectorAll(".test");
    cout << "[replaceAll] .test count: " << replaced.size() << endl;
    for (auto& n : replaced) {
        cout << "  replaced: " << n->getContent() << endl;
    }

    // 6. insertBefore/insertAfter
    auto span = root->querySelector("span");
    if (span) {
        span->insertBefore("<p>BeforeSpan</p>");
        span->insertAfter("<p>AfterSpan</p>");
        cout << "[insertBefore/After] span: " << span->parent()->getContent() << endl;
    }

    // 7. prependHtml/appendHtml
    auto mainDiv = root->querySelector("#main");
    if (mainDiv) {
        mainDiv->prependHtml("<header>Header</header>");
        mainDiv->appendHtml("<footer>Footer</footer>");
        cout << "[prependHtml/appendHtml] #main: " << mainDiv->getContent() << endl;
    }

    // 8. 复杂嵌套插入
    if (mainDiv) {
        mainDiv->appendHtml("<div class='test'>AppendedTest</div>");
        auto tests = mainDiv->querySelectorAll(".test");
        cout << "[appendHtml] #main .test count: " << tests.size() << endl;
    }

    // 9. 输出最终HTML
    cout << "[Final HTML] " << root->getContent() << endl;
}
