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


enum
{
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

typedef struct Cnode
{
    vector<Cnode *> ch;
    string heading;
    string tag;

    Cnode(const string &hd)
    {}
} Cnode;

typedef struct node
{
    int type;                // 节点代表的类型
    vector<node *> ch;
    /*
     * 用来存放三个重要的属性, elem[0] 保存了要显示的内容
     * elem[1] 保存了链接, elem[2] 则保存了 title
     * */
    string elem[3];

    node(int _type) : type(_type)
    {}
} node;


class transform
{
private:
    node *root, *now;
    Cnode *Croot;
    string content, TOC;
    int cntTag = 0;
    char s[maxLength];

    // 判断是否为标题
    inline bool isHeading(node *v)
    {
        return (v->type >= h1 && v->type <= h6);
    }

    // 判断是否为图片
    inline bool isImage(node *v)
    {
        return (v->type == image);
    }

    // 判断是否为超链接
    inline bool isHref(node *v)
    {
        return (v->type == href);
    }

    // 换行
    inline bool isCutline(char *src)
    {
        int cnt = 0;
        char *ptr = src;
        while (*ptr)
        {
            if (*ptr != ' ' && *ptr != '\t' && *ptr != '-')
                return false;
            if (*ptr == '-')
                cnt++;
            ptr++;
        }
        return (cnt >= 3);
    }

    // 生成段落
    inline void mkpara(node *v)
    {
        if (v->ch.size() == 1u && v->ch.back()->type == paragraph)
            return;
        if (v->type == paragraph)
            return;
        if (v->type == nul)
        {
            v->type = paragraph;
            return;
        }
        node *x = new node(paragraph);
        x->ch = v->ch;
        v->ch.clear();
        v->ch.push_back(x);
    }

    // 解析空格和tab
    inline pair<int, char *> start(char *src)
    {
        if ((int) strlen(src) == 0)
            return make_pair(0, nullptr);
        int cntSpace = 0, cntTab = 0;
        for (int i = 0; src[i] != '\0'; i++)
        {
            if (src[i] == ' ') cntSpace++;
            else if (src[i] == '\t') cntTab++;
            return make_pair(cntTab + cntSpace / 4, src + i);
        }

        return make_pair(0, nullptr);
    }

    // 声明模板
    // 销毁节点
    template<typename T>
    void destroy(T *v)
    {
        for (int i = 0; i < (int) v->ch.size(); i++)
        {
            destroy(v->ch[i]);
        }
        delete v;
    }

    void Cdfs(Cnode *v, string index)
    {
        TOC += "<li>\n";
        TOC += "<a href=\"#" + v->tag + "\">" + index + " " + v->heading + "</a>\n";
        int n = (int) v->ch.size();
        if (n)
        {
            TOC += "<ul>\n";
            for (int i = 0; i < n; i++)
            {
                Cdfs(v->ch[i], index + to_string(i + 1) + ".");
            }
            TOC += "</ul>\n";
        }
        TOC += "</li>\n";
    }

    void Cins(Cnode *v, int x, const string &hd, int tag)
    {
        int n = (int) v->ch.size();
        if (x == 1)
        {
            v->ch.push_back(new Cnode(hd));
            v->ch.back()->tag = "tag" + to_string(tag);
            return;
        }
        if (!n || v->ch.back()->heading.empty())
        {
            v->ch.push_back(new Cnode(""));
        }
        Cins(v->ch.back(), x - 1, hd, tag);
    }

    void dfs(node *v)
    {
        if (v->type == paragraph && v->elem[0].empty() && v->ch.empty())
            return;

        content += frontag[v->type];
        bool flag = true;

        //标题，可以目录跳转
        if (isHeading(v))
        {
            content += "id=\"" + v->elem[0] + "\">";
            flag = false;
        }

        // 超链接
        if (isHref(v))
        {
            content += "<a href=\"" + v->elem[1] + "\" title=\"" + v->elem[2] + "\">" + v->elem[0] + "</a>";
            flag = false;
        }

        // 图片
        if (isImage(v))
        {
            content += "<img alt=\"" + v->elem[0] + "\" src=\"" + v->elem[1] + "\" title=\"" + v->elem[2] + "\" />";
            flag = false;
        }

        if (flag)
        {
            content += v->elem[0];
            flag = false;
        }

        // 遍历内容
        for (int i = 0; i < (int) v->ch.size(); ++i)
        {
            dfs(v->ch[i]);
        }
        content += backTag[v->type];
    }


};


#endif //SIMPLEMARKDOWN_MDTRANSFORM_HPP
