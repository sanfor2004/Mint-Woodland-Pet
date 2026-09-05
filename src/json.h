// SPDX-FileCopyrightText: 2026 sanfor2004
// SPDX-License-Identifier: MIT

#pragma once
#include <map>
#include <string>
#include <vector>
#include <stdexcept>
#include <cctype>

// Deliberately small parser for this project's UTF-8, string/array/object schema.
// Unsupported JSON primitives are rejected rather than silently interpreted.
struct Json {
    std::string text;
    std::map<std::string, Json> object;
    std::vector<Json> array;
    char kind = 0;
    const Json& at(const std::string& key) const { return object.at(key); }
};
class JsonParser {
    const std::string& s; size_t p = 0;
    void ws() { while (p < s.size() && std::isspace(static_cast<unsigned char>(s[p]))) ++p; }
    char take() { if (p == s.size()) throw std::runtime_error("Unexpected end of JSON"); return s[p++]; }
    void expect(char c) { ws(); if (take() != c) throw std::runtime_error("Invalid JSON punctuation"); }
    std::string str() {
        expect('"'); std::string v;
        for (;;) {
            char c = take(); if (c == '"') return v;
            if (static_cast<unsigned char>(c) < 32) throw std::runtime_error("Invalid JSON string");
            if (c == '\\') {
                c = take();
                switch (c) {
                case '"': case '\\': case '/': break;
                case 'n': c = '\n'; break; case 'r': c = '\r'; break; case 't': c = '\t'; break;
                default: throw std::runtime_error("Use literal UTF-8 instead of escaped Unicode");
                }
            }
            v += c;
        }
    }
    Json value(int depth) {
        if (depth > 32) throw std::runtime_error("JSON nesting too deep");
        ws(); Json v;
        if (p == s.size()) throw std::runtime_error("Missing JSON value");
        v.kind = s[p];
        if (s[p] == '"') v.text = str();
        else if (s[p] == '[') {
            ++p; ws(); if (p < s.size() && s[p] == ']') { ++p; return v; }
            for (;;) { v.array.push_back(value(depth + 1)); ws(); char c = take(); if (c == ']') break; if (c != ',') throw std::runtime_error("Invalid array"); }
        } else if (s[p] == '{') {
            ++p; ws(); if (p < s.size() && s[p] == '}') { ++p; return v; }
            for (;;) { std::string key = str(); expect(':'); if (!v.object.emplace(key, value(depth + 1)).second) throw std::runtime_error("Duplicate JSON key"); ws(); char c = take(); if (c == '}') break; if (c != ',') throw std::runtime_error("Invalid object"); }
        } else throw std::runtime_error("Expected string, array or object");
        return v;
    }
public:
    explicit JsonParser(const std::string& source) : s(source) {}
    Json parse() { Json v = value(0); ws(); if (p != s.size()) throw std::runtime_error("Trailing JSON data"); return v; }
};
