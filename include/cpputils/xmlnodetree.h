#pragma once

#include <cpputils/formatter.h>
#include <cpputils/xmlparser.h>

struct XmlNode {
    using ptr = std::shared_ptr<XmlNode>;
    using XmlAttr = XmlParser::attribute_list;

    std::string tag;
    XmlAttr attrs;
    std::string data;
    std::vector<XmlNode::ptr> children;


    XmlNode(const std::string& tag, const XmlAttr& attrs)
        : tag(tag), attrs(attrs)
    {
    }
    XmlNode(const std::string& data)
        : data(data)
    {
    }

    static auto make(const std::string& tag, const XmlAttr& attrs)
    {
        return std::make_shared<XmlNode>(tag, attrs);
    }
    static auto makedata(const std::string& data)
    {
        return std::make_shared<XmlNode>(data);
    }

    void addchild(ptr child)
    {
        children.push_back(child);
    }
    void adddata(const std::string& moredata)
    {
        data += moredata;
    }

    std::string find_attr(const std::string& key) const
    {
        for (auto& kv : attrs)
            if (kv.first == key)
                return kv.second;
        return {};
    }
    bool has_attr(const std::string& key) const
    {
        for (auto& kv : attrs)
            if (kv.first == key)
                return true;
        return false;
    }
    std::string asstring() const
    {
        return stringformat("tag:%s, %d at, %d ch, %d data", tag, attrs.size(), children.size(), data.size());
    }
    template<typename FN>
    void recurse(FN fn)
    {
        for (auto c : children)
            fn(c);
    }

    std::string asxml() const
    {
        if (data.empty() && children.empty())
            return stringformat("<%s%s/>", tag, attrstring());
        else
            return stringformat("<%s%s>%s%s</%s>", tag, attrstring(), data, childstring(), tag);
    }
    std::string attrstring() const
    {
        std::string res;
        for (auto a : attrs)
            res += stringformat(" %s=\"%s\"", a.first, a.second);
        return res;
    }
    std::string childstring() const
    {
        std::string res;
        for (auto c : children)
            res += c->asxml();
        return res;
    }
};

class XmlNodeTree : public XmlParser {
    using XmlAttr = XmlParser::attribute_list;

    std::vector<XmlNode::ptr> _stack;
    std::vector<XmlNode::ptr> _roots;

public:
    // interface for xmlparser
    void handle_starttag(std::string tag, const XmlAttr& attrs) override
    {
        auto item = XmlNode::make(tag, attrs);
        _stack.push_back(item);
    }
    void handle_endtag(std::string tag) override
    {
        auto item = _stack.back();
        if (item->tag != tag)
            throw std::runtime_error("tag mismatch");
        _stack.pop_back();
        if (_stack.empty())
            _roots.push_back(item);
        else
            _stack.back()->addchild(item);
    }
    void handle_data(const char*first, const char*last) override
    {
        auto sv = std::string(first, last-first);
        if (_stack.empty())
            _roots.push_back(XmlNode::makedata(sv));
        else
            _stack.back()->adddata(sv);
    }


    // interface for completed tree
    XmlNode::ptr root() const
    {
        XmlNode::ptr res;
        int nroots = 0;
        for (auto n : _roots)
            if (!n->tag.empty()) {
                if (!res)
                    res = n;
                nroots++;
            }
        if (nroots>1)
            print("warn: found %d roots\n", nroots);

        return res;
    }
    bool validate() const
    {
        int nroots = 0;
        for (auto n : _roots)
            if (!n->tag.empty())
                nroots++;
        return _stack.empty() && nroots==1;
    }
    void dump() const
    {
        if (!_stack.empty()) {
            print("== stack\n");
            for (auto p : _stack)
                print("%s\n", p->asstring());
        }
        if (!_roots.empty()) {
            print("== roots\n");
            for (auto p : _roots)
                print("%s\n", p->asstring());
        }
        print("--\n");
    }
    void dumpxml() const
    {
        if (!_stack.empty()) {
            print("== stack\n");
            for (auto p : _stack)
                print("* %s\n", p->asxml());
        }
        if (!_roots.empty()) {
            print("== roots\n");
            for (auto p : _roots)
                print("* %s\n", p->asxml());
        }
        print("--\n");
    }
};


