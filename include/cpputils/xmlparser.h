#pragma once
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <utility>

// TODO: add optional support for non-quoted attribute values, and attributes without a value, so i can parse most html too.
// TODO: support the nested <!DOCTYPE [ ... ]>  type values.

// TODO: add method for searching attribute_list
/*
 * XmlParser based upon how the python 'html.parser' works.
 */
class XmlParser {
    //  <?xml  ... ?>      - processing instructions
    //  <!(?:DOCTYPE|ENTITY|ELEMENT|ATTLIST|INCLUDE|IGNORE|NOTATION) ...> handler
    //  <starttag ...>   handler
    //  </endtag >  handler
    //  <element />    handler
    //  <!-- comment --> handler
    //  <![CDATA[ ... ]]>  handler
    //  ... data handler
    //  &entity;
public:
    using attribute_list = std::vector<std::pair<std::string, std::string>>;

    virtual ~XmlParser()  { }

    // implement these methods when subclassing this parser
    virtual void handle_starttag(std::string tag, const attribute_list& attrs) { }
    virtual void handle_endtag(std::string tag) { }
    virtual void handle_element(std::string tag, const attribute_list& attrs)
    {
        handle_starttag(tag, attrs);
        handle_endtag(tag);
    }
    virtual void handle_xml_decl(std::string tag, const attribute_list& attrs) { }
    virtual void handle_comment(const char*first, const char*last) { }
    virtual void handle_data(const char*first, const char*last) { }

    enum {
        TOKEN_LESS_THEN,
        TOKEN_LT_QUESTION,
        TOKEN_LT_EXCLAMATION,
        TOKEN_LT_SLASH,
        //TOKEN_GREATER_THEN,
        //TOKEN_QUESTION_GT,
        //TOKEN_SLASH_GT,
        TOKEN_NAME,
        TOKEN_STRING,
        TOKEN_EQUALS,
        // note: no token for comment, cdata
    };
    struct Token {
        virtual ~Token() { }
        virtual int type() = 0;
        virtual std::string name() const { return {}; }
        virtual std::string value() const { return {}; }
    };
    struct LT_Token : Token {
        int type() override { return TOKEN_LESS_THEN; }
    };
    struct LTQ_Token : Token {
        int type() override { return TOKEN_LT_QUESTION; }
    };
    struct LTX_Token : Token {
        int type() override { return TOKEN_LT_EXCLAMATION; }
    };
    struct LTS_Token : Token {
        int type() override { return TOKEN_LT_SLASH; }
    };

//  struct GT_Token : Token {
//      int type() override { return TOKEN_GREATER_THEN; }
//  };
//  struct SGT_Token : Token {
//      int type() override { return TOKEN_SLASH_GT; }
//  };
//  struct QGT_Token : Token {
//      int type() override { return TOKEN_QUESTION_GT; }
//  };
    struct NAME_Token : Token {
        const char *first;
        const char *last;

        NAME_Token(const char *first, const char *last)
            : first(first), last(last)
        {
        }

        int type() override { return TOKEN_NAME; }

        std::string name() const override { return {first, last}; }
    };
    struct STRING_Token : Token {
        const char *first;
        const char *last;

        STRING_Token(const char *first, const char *last)
            : first(first), last(last)
        {
        }

        int type() override { return TOKEN_STRING; }

        std::string value() const override { return {first, last}; }
    };
    struct EQ_Token : Token {
        int type() override { return TOKEN_EQUALS; }
    };

private:
    std::vector<std::shared_ptr<Token>> _stack;
public:
    template<typename STR>
    static std::string decode_entities(const STR& str)
    {
        // TODO  - decode entities
        return str;
    }
    template<typename STR>
    static std::string encode_entities(const STR& str)
    {
        // TODO  - encode entities
        return str;
    }

    int stack_find(int token)
    {
        int i = _stack.size()-1;
        while (i>=0) {
            if (_stack[i]->type() == token)
                return i;
        }
        return -1;
    }
    template<typename...ARGS>
    bool stack_match(int token, ARGS...args)
    {
        if (_stack.size() < 1 + sizeof...(ARGS))
            return false;
        if (_stack[_stack.size()-1-sizeof...(ARGS)]->type() != token)
            return false;

        if constexpr (sizeof...(ARGS) != 0)
            return stack_match(args...);
        else
            return true;
    }
    auto stack_pop_attr()
    {
        attribute_list attrs;
        while (stack_match(TOKEN_NAME, TOKEN_EQUALS, TOKEN_STRING)) {
            auto value = _stack.back();
            _stack.pop_back();
            _stack.pop_back(); // '='
            auto name = _stack.back();
            _stack.pop_back();

            attrs.emplace_back(std::string(name->name()), decode_entities(value->value()));
        }
        std::reverse(attrs.begin(), attrs.end());
        return attrs;
    }
    static bool isnamechar(char c)
    {
        if constexpr (std::is_signed_v<char>) {
            if (c < 0) return true;
        }
        if (c <= 0x2c) return false;
        if (c == 0x2f) return false;
        if (c <= 0x3a) return true;
        if (c <= 0x40) return false;
        if (c <= 0x5a) return true;
        if (c <= 0x5e) return false;
        if (c == 0x60) return false;
        if (c <= 0x7a) return true;
        if constexpr (std::is_unsigned_v<char>) {
            // NOTE: this should never happen for signed chars,
            //   but somehow I keep getting a compiler warning for this line: always true.
            if (c <= 0x7f) return false;
        }

        // note: not following xml spec exactly, 
        // also unicode 80-b6, b8-bf, d7, f7, 37e, 2000-200b, 200e-203e,
        //          2041-206f, 2190-2bff, 2ff0-3000,d800-f8ff, fdd0-fdef,
        //          fffe-ffff, f0000-10ffff
        // should be excluded.
        return true;
    }
    static bool is_open_token(int token)
    {
        switch (token) {
            case TOKEN_LESS_THEN:
            case TOKEN_LT_QUESTION:
            case TOKEN_LT_EXCLAMATION:
            case TOKEN_LT_SLASH:
                return true;
        }
        return false;
    }
    int most_recent_open_token()
    {
        for (auto i = _stack.rbegin() ; i != _stack.rend() ; ++i)
            if (is_open_token((*i)->type()))
                return (*i)->type();
        return -1;
    }
public:
    template<typename STR>
    void parse(const STR& str)
    {
        parse((const char*)&str[0], (const char*)&str[0] + str.size());
    }
    void parse(const char*first, const char*last)
    {
        const char *p = first;
        const char *q = last;
        while (p < last) {
            if (_stack.empty()) {
                // currently not inside a tag.

                q = std::find(p, last, '<');
                if (q == last) {
                    handle_data(p, last);
                    break;
                }
                if (p < q)
                    handle_data(p, q);

                ++q;
                if (q == last)
                    throw std::runtime_error("invalid xml#0");
                if (*q == '?') {
                    // <?  name  attr  ?>
                    _stack.push_back(std::make_shared<LTQ_Token>());
                    ++q;
                }
                else if (*q == '!') {
                    if (q+2 < last && q[1]=='-' && q[2] == '-') {
                        // <!--  ... -->
                        q += 3;
                        const char *ecmt = "-->";
                        auto e = std::search(q, last, ecmt, ecmt+3);
                        if (e==last)
                            throw std::runtime_error("invalid xml#1");
                        handle_comment(q, e);
                        q = e+3;
                    }
                    else if (q+8 < last && std::equal(q, q+8, "![CDATA[")) {
                        // <![CDATA[ ... ]]>
                        q += 8;
                        const char *ecmt = "]]>";
                        auto e = std::search(q, last, ecmt, ecmt+3);
                        if (e==last)
                            throw std::runtime_error("invalid xml#2");
                        handle_data(q, e);
                        q = e+3;
                    }
                    else {
                        // <! KEYWORD
                        _stack.push_back(std::make_shared<LTX_Token>());
                        ++q;
                    }
                }
                else if (*q == '/') {
                    //  </  name >
                    _stack.push_back(std::make_shared<LTS_Token>());
                    ++q;
                }
                else {
                    // <  name ...
                    _stack.push_back(std::make_shared<LT_Token>());
                }
            }
            else {  // inside a tag

                // skip whitespace
                q = std::find_if(p, last, [](char c) { return !isspace(c); });
                if (q==last)
                    throw std::runtime_error("invalid xml#3");
                char c = *q++;
                if (c == '"' || c=='\'') {
                    // string
                    auto e = std::find(q, last, c);
                    if (e==last)
                        throw std::runtime_error("invalid xml#4");
                    _stack.push_back(std::make_shared<STRING_Token>(q, e));
                    q = e+1;
                }
                else if (c == '/' && q < last && *q == '>') {
                    // <  name  k=v ... />
                    ++q;
                    // stack: LT_Token + NAME_Token + [ NAME = STRING ]* + SGT
                    auto attrs = stack_pop_attr();
                    if (!stack_match(TOKEN_LESS_THEN, TOKEN_NAME))
                        throw std::runtime_error("invalid xml#5");
                    auto tagname = _stack.back();
                    _stack.pop_back();
                    _stack.pop_back();  // '<'

                    handle_element(tagname->name(), attrs);
                }
                else if (c == '?' && q < last && *q == '>') {
                    // <?  ...  ?>
                    ++q;
                    auto attrs = stack_pop_attr();
                    if (!stack_match(TOKEN_LT_QUESTION, TOKEN_NAME))
                        throw std::runtime_error("invalid xml#6");
                    auto tagname = _stack.back();
                    _stack.pop_back();
                    _stack.pop_back();  // '<?'

                    handle_xml_decl(tagname->name(), attrs);
                }
                else if (c == '>') {
                    if (stack_match(TOKEN_LT_SLASH, TOKEN_NAME)) {
                        // </ name >
                        auto tagname = _stack.back();
                        _stack.pop_back();
                        _stack.pop_back();  // '</'

                        handle_endtag(tagname->name());
                    }
                    else if (most_recent_open_token() == TOKEN_LESS_THEN) {
                        // < name k=v ... >
                        auto attrs = stack_pop_attr();
                        if (!stack_match(TOKEN_LESS_THEN, TOKEN_NAME)) 
                            throw std::runtime_error("invalid xml#7");
                        auto tagname = _stack.back();
                        _stack.pop_back();
                        _stack.pop_back();  // '<'

                        handle_starttag(tagname->name(), attrs);
                    }
                    else {
                        // discard  !DOCTYPE stuff.
                        while (!_stack.empty() && _stack.back()->type() != TOKEN_LT_EXCLAMATION)
                            _stack.pop_back();

                        if (_stack.empty())
                            throw std::runtime_error("invalid xml#8");
                        _stack.pop_back(); // pop <!
                    }
                }
                else if (c == '=') {
                    _stack.push_back(std::make_shared<EQ_Token>());
                }
                else if (isnamechar(c)) {
                    auto e = std::find_if(q, last, [](auto c){ return !XmlParser::isnamechar(c); });
                    if (e==last)
                        throw std::runtime_error("invalid xml#9");
                    _stack.push_back(std::make_shared<NAME_Token>(q-1, e));

                    q = e;
                }
                else {
                    throw std::runtime_error("invalid xml#10");
                }
            }
            p = q;
        }
    }
};
