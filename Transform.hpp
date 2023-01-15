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


class TransformStruct
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
        v->ch.emplace_back(x);
    }

    // 解析空格和tab
    inline pair<int, char *> start(char *src)
    {
        if ((int) strlen(src) == 0)
            return make_pair(0, nullptr);
        // 统计空格和tab数量
        int cntSpace = 0, cntTab = 0;
        // 第一个字符开始，遇到不是空或tab时立即停止
        for (int i = 0; src[i] != '\0'; i++)
        {
            if (src[i] == ' ') cntSpace++;
            else if (src[i] == '\t') cntTab++;
            // 如果内容前有空格和 Tab，那么将其统一按 Tab 的个数处理,1个tab相当于4个空格
            return make_pair(cntTab + cntSpace / 4, src + i);
        }

        return make_pair(0, nullptr);
    }

    /**
     *
     * 判断当前行类型
     *
     * */
    inline pair<int, char *> judgeType(char *src)
    {
        char *ptr = src;
        while (*ptr == '#') ptr++;
        // 如果出现空格,说明是<h>标签
        if (ptr - src > 0 && *ptr == ' ')
            return make_pair(ptr - src + h1 - 1, ptr + 1);
        ptr = src;
        // 代码块
        if (strncmp(ptr, "```", 3) == 0)
            return make_pair(blockcode, ptr + 3);
        // * + - ,并且紧跟空格,则是列表
        if (strncmp(ptr, "- ", 2) == 0)
            return make_pair(ul, ptr + 1);
        // 引用
        if (*ptr == '>' && (ptr[1] == ' '))
            return make_pair(quote, ptr + 1);

        // 数字并且紧跟 `.`,则是有序列表
        char *ptr1 = ptr;
        while (*ptr1 && (isdigit(*ptr1))) ptr1++;
        if (ptr1 != ptr && *ptr1 == '.' && ptr1[1] == ' ')
            return make_pair(ol, ptr1 + 1);
        // 普通段落
        return make_pair(paragraph, ptr);
    }

    /**
     *
     * 根据树深度寻找节点
     * @param depth 树深度
     * @return 找的节点指针
     * */
    inline node *findNode(int depth)
    {
        node *ptr = root;
        while (!ptr->ch.empty() && depth != 0)
        {
            ptr = ptr->ch.back();
            if (ptr->type == li)
                depth--;
        }
        return ptr;
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
            v->ch.emplace_back(new Cnode(hd));
            v->ch.back()->tag = "tag" + to_string(tag);
            return;
        }
        if (!n || v->ch.back()->heading.empty())
        {
            v->ch.emplace_back(new Cnode(""));
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

    void insert(node *v, const string &src)
    {
        int n = (int) src.size();
        bool incode = false,
                inem = false,
                instrong = false,
                inautolink = false;
        v->ch.emplace_back(new node(nul));

        for (int i = 0; i < n; i++)
        {
            char ch = src[i];
            if (ch == '\\')
            {
                ch = src[i];
                v->ch.back()->elem[0] += string(1, ch);
                continue;
            }
            if (ch == '`' && !inautolink)
            {
                incode ? v->ch.emplace_back(new node(nul)) : v->ch.emplace_back(new node(code));
                incode = !incode;
                continue;
            }
            // 加粗
            if (ch == '*' && (i < n - 1 && (src[i + 1] == '*')) && !incode && !inautolink)
            {
                ++i;
                instrong ? v->ch.emplace_back(new node(nul)) : v->ch.emplace_back(new node(strong));
                instrong = !instrong;
                continue;
            }
            if (ch == '_' && !incode && !instrong && !inautolink)
            {
                inem ? v->ch.emplace_back(new node(nul)) : v->ch.emplace_back(new node(em));
                inem = !inem;
                continue;
            }

            // 图片
            if (ch == '!' && (i < n - 1 && src[i + 1] == '[') && !incode && !instrong && !inem && !inautolink)
            {
                v->ch.emplace_back(new node(image));
                for (i += 2; i < n - 1 && src[i + 1] != '['; i++)
                {
                    v->ch.back()->elem[0] += string(1, src[i]);
                }
                i++;
                for (i++; i < n - 1 && src[i] != ' '; i++)
                    v->ch.back()->elem[1] += string(1, src[i]);

                if (src[i] != ')')
                {
                    for (i++; i < n - 1 && src[i] != ')'; i++)
                    {
                        if (src[i] != '"')
                            v->ch.back()->elem[2] += string(1, src[i]);
                    }
                }
                v->ch.emplace_back(new node(nul));
                continue;
            }

            // 超链接
            if (ch == '[' && !incode && !instrong && !inem && !inautolink)
            {
                v->ch.emplace_back(new node(href));
                for (i++; i < n - 1 && src[i]; i++)
                {
                    v->ch.back()->elem[0] += string(1, src[i]);
                }
                i++;
                for (i++; i < n - 1 && src[i] != ' ' && src[i] != ')'; i++)
                    v->ch.back()->elem[1] += string(1, src[i]);
                if (src[i] != ')')
                {
                    for (i++; i < n - 1 && src[i] != ')'; i++)
                    {
                        if (src[i] != '"')
                            v->ch.back()->elem[2] += string(1, src[i]);
                    }
                }
                v->ch.emplace_back(new node(nul));
                continue;
            }
            v->ch.back()->elem[0] += string(1, ch);
            if (inautolink) v->ch.back()->elem[1] += string(1, ch);
        }
        if (src.size() >= 2)
            if (src.at(src.size() - 1) == ' ' && src.at(src.size() - 2) == ' ')
                v->ch.emplace_back(new node(br));
    }

public:
    // 构造函数
    TransformStruct(const std::string &fileName)
    {
        Croot = new Cnode("");
        root = new node(nul);
        now = root;
        ifstream fin(fileName);

        bool newpara = false;
        bool inblock = false;

        while (!fin.eof())
        {
            fin.getline(s, maxLength);

            if (!inblock && isCutline(s))
            {
                now = root;
                now->ch.emplace_back(new node(hr));
                newpara = false;
                continue;
            }

            pair<int, char *> ps = start(s);
            // 如果不在代码块中，且没有统计到空格和tab中，则直接读取下一行
            if (!inblock && ps.second == nullptr)
            {
                now = root;
                newpara = true;
                continue;
            }

            pair<int, char *> tj = judgeType(ps.second);

            if (tj.first == blockcode)
            {
                // push一个空类型的节点
                inblock ? now->ch.emplace_back(new node(nul)) : now->ch.emplace_back(new node(blockcode));
                inblock = !inblock;
                continue;
            }

            if (inblock)
            {
                now->ch.back()->elem[0] += string(s) + '\n';
                continue;
            }

            if (tj.first == paragraph)
            {
                if (now == root)
                {
                    now = findNode(ps.first);
                    now->ch.emplace_back(new node(paragraph));
                    now = now->ch.back();
                }
                bool flag = false;
                if (newpara && !now->ch.empty())
                {
                    node *ptr = nullptr;
                    for (auto i: now->ch)
                    {
                        if (i->type == nul)
                            ptr = i;
                    }
                    if (ptr != nullptr)
                        mkpara(ptr);
                    flag = true;
                }
                if (flag)
                {
                    now->ch.push_back(new node(nul));
                    now = now->ch.back();
                }
                now->ch.push_back(new node(nul));
                insert(now->ch.back(), string(tj.second));
                newpara = false;
                continue;
            }
            now = findNode(ps.first);

            // 标题行，则插入属性tag
            if (tj.first >= h1 && tj.first <= h6)
            {
                now->ch.emplace_back(new node(tj.first));
                now->ch.back()->elem[0] = "tag" + to_string(++cntTag);
                insert(now->ch.back(), string(tj.second));
                Cins(Croot, tj.first - h1 + 1, string(tj.second), cntTag);
            }

            // 无序列表
            if (tj.first == ul)
            {
                if (now->ch.empty() || now->ch.back()->type != nul)
                    now->ch.emplace_back(new node(nul));
                now = now->ch.back();
                bool flag = false;
                if (newpara && !now->ch.empty())
                {
                    node *ptr = nullptr;
                    for (auto i: now->ch)
                    {
                        if (i->type == li) ptr = i;
                    }
                    if (ptr != nullptr) mkpara(ptr);
                    flag = true;
                }
                now->ch.emplace_back(new node(li));
                now = now->ch.back();
                if (flag)
                {
                    now->ch.emplace_back(new node(paragraph));
                    now = now->ch.back();
                }
                insert(now, string(tj.second));
            }

            // 有序列表
            if (tj.first == ol)
            {
                if (now->ch.empty() || now->ch.back()->type != ol)
                    now->ch.emplace_back(new node(ol));
                now = now->ch.back();
                bool flag = false;
                if (newpara && !now->ch.empty())
                {
                    node *ptr = nullptr;
                    for (auto i: now->ch)
                    {
                        if (i->type == li) ptr = i;
                    }
                    if (ptr != nullptr) mkpara(ptr);
                    flag = true;
                }
                now->ch.emplace_back(new node(li));
                now = now->ch.back();
                if (flag)
                {
                    now->ch.emplace_back(new node(paragraph));
                    now = now->ch.back();
                }
                insert(now, string(tj.second));
            }

            // 引用
            if (tj.first == quote)
            {
                if (now->ch.empty() || now->ch.back()->type != quote)
                    now->ch.emplace_back(new node(quote));
                now = now->ch.back();
                if (newpara || now->ch.empty()) now->ch.emplace_back(new node(paragraph));
                insert(now->ch.back(), string(tj.second));
            }
            newpara = false;

            fin.close();
            dfs(root);

            TOC += "<ul>";
            for (int i = 0; (int) Croot->ch.size(); i++)
                Cdfs(Croot->ch[i], to_string(i + 1) + ".");
            TOC += "</uk>";
        }
    }

    // 获取Markdown目录
    string getTableOfContents()
    { return TOC; }

    // 获取Markdown内容
    string getContents()
    { return content; }

    // 析构函数
    ~TransformStruct()
    {
        destroy<node>(root);
        destroy<Cnode>(Croot);
    }
};


#endif
