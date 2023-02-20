#include "unittestframework.h"

// include twice to detect proper header behaviour
#include <cpputils/xmlparser.h>
#include <cpputils/xmlparser.h>

using namespace std::string_literals;

class MyXml : public XmlParser {
public:
    enum { START, END, ELEM, DECL, CMT, DATA };
    std::vector<std::tuple<int, std::string, attribute_list>> parsed;

    void handle_starttag(std::string tag, const attribute_list& attrs) override { parsed.emplace_back(START, tag, attrs); }
    void handle_endtag(std::string tag) override { parsed.emplace_back(END, tag, attribute_list{}); }
    void handle_element(std::string tag, const attribute_list& attrs) override { parsed.emplace_back(ELEM, tag, attrs); }
    void handle_xml_decl(std::string tag, const attribute_list& attrs) override { parsed.emplace_back(DECL, tag, attrs); }
    void handle_comment(const char*first, const char*last) override { parsed.emplace_back(CMT, std::string(first, last), attribute_list{}); }
    void handle_data(const char*first, const char*last) override { parsed.emplace_back(DATA, std::string(first, last), attribute_list{}); }

#if 0
    auto mk(int type, const std::string& tag)
    {
        return decltype(parsed){std::make_tuple(type, tag, attribute_list{})};
    }
    auto mk(int type0, const std::string& tag0, int type1, const std::string& tag1)
    {
        return decltype(parsed){std::make_tuple(type0, tag0, attribute_list{}), std::make_tuple(type1, tag1, attribute_list{})};
    }
#endif

    /*  create a vector of { type, tag, [] } */
    template<typename...ARGS>
    auto mk(ARGS&&...args)
    {
        decltype(parsed) v;
        if constexpr (sizeof...(ARGS) > 0)
            add(v, std::forward<ARGS>(args)...);
        return v;
    }
    template<typename...ARGS>
    void add(decltype(parsed)& v, int type, const std::string& tag, ARGS&&...args)
    {
        v.emplace_back(std::make_tuple(type, tag, attribute_list{}));
        if constexpr (sizeof...(ARGS) > 0)
            add(v, std::forward<ARGS>(args)...);
    }

    /* create a vector of { type, tag, attrlist } */
    template<typename...ARGS>
    auto mkx(ARGS&&...args)
    {
        decltype(parsed) v;
        if constexpr (sizeof...(ARGS) > 0)
            addx(v, std::forward<ARGS>(args)...);
        return v;
    }

    template<typename...ARGS>
    void addx(decltype(parsed)& v, int type, const std::string& tag, const attribute_list& attrs, ARGS&&...args)
    {
        v.emplace_back(std::make_tuple(type, tag, attrs));
        if constexpr (sizeof...(ARGS) > 0)
            addx(v, std::forward<ARGS>(args)...);
    }

    /* create a attributelist of [ { k, v }, ... ] */
    template<typename...ARGS>
    auto mkattr(ARGS&&...args)
    {
        attribute_list v;
        if constexpr (sizeof...(ARGS) > 0)
            addattr(v, std::forward<ARGS>(args)...);
        return v;
    }
    template<typename...ARGS>
    void addattr(attribute_list& v, const std::string& key, const std::string& val, ARGS&&...args)
    {
        v.emplace_back(key, val);
        if constexpr (sizeof...(ARGS) > 0)
            addattr(v, std::forward<ARGS>(args)...);
    }

};
#ifdef USE_CATCH
std::ostream& operator<<(std::ostream& os, const std::pair<std::string, std::string>& item)
{
    return os << '(' << item.first << ", " << item.second << ')';
}
std::ostream& operator<<(std::ostream& os, const std::vector<std::pair<std::string, std::string>>& item)
{
    os << '[';
    bool first = true;
    for (auto & ent : item)
    {
        if (!first)
            os << ", ";
        os << ent;
        first = false;
    }
    os << ']';
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::tuple<int, std::string, std::vector<std::pair<std::string, std::string>>>& item)
{
    return os << '{' << std::get<0>(item) << ", " << std::get<1>(item) << ", " << std::get<1>(item) << '}';
}
#endif
TEST_SUITE("xml") {
    TEST_CASE("construct_xml") {
        XmlParser tst;

        CHECK(true);
    }
    TEST_CASE("testchars1") {
        XmlParser tst;

        CHECK_NOTHROW(tst.parse(""s));
        CHECK_NOTHROW(tst.parse("."s));
        CHECK_NOTHROW(tst.parse(" "s));
        CHECK_NOTHROW(tst.parse("<xx>"s));
    }
    TEST_CASE("testchars") {
        for (int c = 0 ; c < 256 ; c++) {
            XmlParser tst;
            char buf[2] = { (char)c, 0 };
            if (c=='<')
                CHECK_THROWS(tst.parse(buf, buf+1));
            else
                CHECK_NOTHROW(tst.parse(buf, buf+1));
        }
    }
    TEST_CASE("test2chars") {
        for (int a = 0 ; a < 256 ; a++) {
        for (int b = 0 ; b < 256 ; b++) {
            XmlParser tst;
            char buf[3] = { (char)a, (char)b, 0 };
            INFO("char = " << a << ',' << b);
            if (b=='<')
                CHECK_THROWS(tst.parse(buf, buf+2));
            else if (a=='<' && "!?/="s.find(b)==std::string::npos)
                CHECK_THROWS(tst.parse(buf, buf+2));
            else
                CHECK_NOTHROW(tst.parse(buf, buf+2));
        }
        }
    }

    TEST_CASE("tstfail") {
        XmlParser tst;
        CHECK_NOTHROW(tst.parse("<a>"s));
        CHECK_NOTHROW(tst.parse("</a>"s));
        CHECK_NOTHROW(tst.parse("<a k='x'>"s));
        CHECK_NOTHROW(tst.parse("<a k='x'/>"s));
        CHECK_NOTHROW(tst.parse("<a k='x' k='x'/>"s));
        CHECK_NOTHROW(tst.parse("<a k='x' />"s));
        CHECK_NOTHROW(tst.parse("<a k=\"x\" />"s));
        CHECK_NOTHROW(tst.parse("<a k='x' ></a>"s));
        CHECK_NOTHROW(tst.parse("<a k='x' > </a>"s));
        CHECK_NOTHROW(tst.parse("<!a>"s));
        CHECK_NOTHROW(tst.parse("<!a k=\"xx\" >"s));
        CHECK_NOTHROW(tst.parse("<?a?>"s));
        CHECK_NOTHROW(tst.parse("<?a v='123'?>"s));

        // todo:  <!xxx <!...> >
        // <=>
    }
    TEST_CASE("validate") {
        MyXml tst;

        tst.parse("<a>"s);
        tst.parse("</a>"s);
        CHECK(tst.parsed == tst.mk(MyXml::START, "a", MyXml::END, "a"));
        tst.parsed.clear();

        tst.parse("<a/>"s);
        CHECK(tst.parsed == tst.mk(MyXml::ELEM, "a"));
        tst.parsed.clear();

        tst.parse("< a/>"s);
        CHECK(tst.parsed == tst.mk(MyXml::ELEM, "a"));
        tst.parsed.clear();

        tst.parse("<a />"s);
        CHECK(tst.parsed == tst.mk(MyXml::ELEM, "a"));
        tst.parsed.clear();


        tst.parse("< a />"s);
        CHECK(tst.parsed == tst.mk(MyXml::ELEM, "a"));
        tst.parsed.clear();

        tst.parse("<a k=\"v\" l=\"w\"/>"s);
        CHECK(tst.parsed == tst.mkx(MyXml::ELEM, "a", tst.mkattr("k", "v", "l", "w")));
        tst.parsed.clear();

    }
};
