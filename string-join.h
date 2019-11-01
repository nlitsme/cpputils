#pragma once

template<typename S, typename V>
auto stringjoin(const S& sep, const V& v)
{
    S result;
    for (auto & s : v)
    {
        if (!result.empty())
            result += sep;
        result += s;
    }
    return result;
}
