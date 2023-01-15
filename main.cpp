#include <fstream>
#include <iostream>
#include "Transform.hpp"

using namespace std;

int main()
{

    // 加载Markdown文件
    TransformStruct transformStruct("/Users/yjd/Documents/C++/SimpleMarkdown/test.md");
    string table = transformStruct.getTableOfContents();
    cout<<"table:"<<table<<endl;

    string contents = transformStruct.getContents();

    std::string head = "<!DOCTYPE html><html><head>"
                       "<meta charset=\"utf-8\">"
                       "<title>Markdown</title>"
                       "<link rel=\"stylesheet\" href=\"github-markdown.css\">"
                       "</head><body><article class=\"markdown-body\">";
    string end = "</article></body></html>";
    ofstream out;
    out.open("index.html");
    out << head + table + contents + end;
    out.close();
    return 0;
}
