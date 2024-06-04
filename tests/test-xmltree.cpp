#include "unittestframework.h"

#include <cpputils/xmlnodetree.h>
#include <cpputils/xmlnodetree.h>

TEST_CASE("xmlnode") {
    XmlNode x("tag", {});

    CHECK(x.tag == "tag");
    CHECK(x.attrs.empty());
    CHECK(x.data.empty());
    CHECK(x.children.empty());

    XmlNode y("data");

    CHECK(y.tag.empty());
    CHECK(y.attrs.empty());
    CHECK(y.data == "data");
    CHECK(y.children.empty());

    auto z = XmlNode::make("tag", {});
    CHECK(z->tag == "tag");
    CHECK(z->attrs.empty());
    CHECK(z->data.empty());
    CHECK(z->children.empty());

    auto w = XmlNode::makedata("data");
    CHECK(w->tag.empty());
    CHECK(w->attrs.empty());
    CHECK(w->data == "data");
    CHECK(w->children.empty());


    x.addchild(z);
    x.adddata("more");
    CHECK(x.find_attr("TEST").empty());
    CHECK(!x.has_attr("TEST"));

    CHECK(x.tag == "tag");
    CHECK(x.attrs.empty());
    CHECK(x.data == "more");
    CHECK(x.children.size()==1);
}
TEST_CASE("xmltree") {
    XmlNodeTree tree;
    using namespace std::string_literals;
    tree.parse("<plist><dict><key>abc</key><integer>213</integer></dict></plist>"s);

    CHECK(tree.validate());
    CHECK(tree.root());
    CHECK(tree.root()->tag == "plist");
}
TEST_CASE("xmltree-2") {  // with extra whitespace
    XmlNodeTree tree;
    using namespace std::string_literals;
    tree.parse("  \n<plist><dict><key>abc</key><integer>213</integer></dict></plist>\n"s);

    CHECK(tree.validate());
    CHECK(tree.root());
    CHECK(tree.root()->tag == "plist");
}
TEST_CASE("xmltree-3") {   // with two roots.
    XmlNodeTree tree;
    using namespace std::string_literals;
    tree.parse("  \n<plist></plist><dict></dict>\n"s);

    CHECK(!tree.validate());
}
TEST_CASE("xmltree-4") {   // with mismatched tags
    XmlNodeTree tree;
    using namespace std::string_literals;
    CHECK_THROWS(tree.parse("  \n<plist><dict></plist></dict>\n"s));
}
TEST_CASE("xmltree-5") {   // unclosed tag.
    XmlNodeTree tree;
    using namespace std::string_literals;
    tree.parse("  \n<plist>\n"s);

    CHECK(!tree.validate());
}
TEST_CASE("xmltree-6") {   // empty string
    XmlNodeTree tree;
    using namespace std::string_literals;
    tree.parse(""s);

    CHECK(!tree.validate());
}



