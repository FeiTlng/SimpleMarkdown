//
// Created by yjd on 2023/1/11.
//

#ifndef SIMPLEMARKDOWN_MDTRANSFORM_HPP

#include <cstdlib>
#include <fstream>
#include <vector>
#include <cstring>
#include <utility>
#include <string>
#include <cctype>
#include <cstdio>

using namespace std;

#define maxLength 10000


enum {
    nul = 0,
    paragraph = 1,
    href = 2,
    ul = 3,
    ol = 4,
    li = 5,
    em = 6,
    strong = 7,
    hr = 8,
    br = 9,
    image = 10,
    quote = 11,
    h1 = 12,
    h2 = 13,
    h3 = 14,
    h4 = 15,
    h5 = 16,
    h6 = 17,
    blockcode = 18,
    code = 19
};

// html左标签
const string frontag[] = {
        "", "<p>", "", "<ul>", "<ol>", "<li>", "<em>", "<strong>",
        "<hr color=#CCCCCC size=1 />", "<br />",
        "", "<blockquote>",
        "<h1 ", "<h2 ", "<h3 ", "<h4 ", "<h5 ", "<h6 ", // 右边的尖括号预留给添加其他的标签属性
        "<pre><code>", "<code>"
};

//html右标签
const string backTag[] = {
        "", "</p>", "", "</ul>", "</ol>", "</li>", "</em>", "</strong>",
        "", "", "", "</blockquote>",
        "</h1>", "</h2>", "</h3>", "</h4>", "</h5>", "</h6>",
        "</code></pre>", "</code>"
};

typedef struct Cnode {
    vector<Cnode *> ch;
    string heading;
    string tag;

    Cnode(const string &hd) {}
} Cnode;

typedef struct node {
    int type;                // 节点代表的类型
    vector<node *> ch;
    string elem[3];         // 用来存放三个重要的属性, elem[0] 保存了要显示的内容
                            // elem[1] 保存了链接, elem[2] 则保存了 title
    node (int _type): type(_type) {}
} node;


class transform {

};


#endif //SIMPLEMARKDOWN_MDTRANSFORM_HPP
