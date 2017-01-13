/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Protocol.hpp"
#include "config.h"

#include <cassert>
#include <cstring>
#include <map>
#include <string>

#define LOK_USE_UNSTABLE_API
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <Poco/StringTokenizer.h>

using Poco::StringTokenizer;

namespace LOOLProtocol
{
    std::tuple<int, int, std::string> ParseVersion(const std::string& version)
    {
        int major = -1;
        int minor = -1;
        std::string patch;

        StringTokenizer firstTokens(version, ".", StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
        if (firstTokens.count() > 0)
        {
            major = std::stoi(firstTokens[0]);

            StringTokenizer secondTokens(firstTokens[1], "-", StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
            minor = std::stoi(secondTokens[0]);

            if (secondTokens.count() > 1)
                patch = secondTokens[1];
        }
        return std::make_tuple(major, minor, patch);
    }

    bool stringToInteger(const std::string& input, int& value)
    {
        try
        {
            value = std::stoi(input);
        }
        catch (std::invalid_argument&)
        {
            return false;
        }

        return true;
    }

    bool stringToUInt64(const std::string& input, uint64_t& value)
    {
        try
        {
            value = std::stoull(input);
        }
        catch (std::invalid_argument&)
        {
            return false;
        }

        return true;
    }

    bool getTokenInteger(const std::string& token, const std::string& name, int& value)
    {
        size_t nextIdx;
        try
        {
            if (token.size() < name.size() + 2 ||
                token.substr(0, name.size()) != name ||
                token[name.size()] != '=' ||
                (value = std::stoi(token.substr(name.size() + 1), &nextIdx), false) ||
                nextIdx != token.size() - name.size() - 1)
            {
                return false;
            }
        }
        catch (std::invalid_argument&)
        {
            return false;
        }

        return true;
    }

    bool getTokenUInt64(const std::string& token, const std::string& name, uint64_t& value)
    {
        size_t nextIdx;
        try
        {
            if (token.size() < name.size() + 2 ||
                token.substr(0, name.size()) != name ||
                token[name.size()] != '=' ||
                (value = std::stoull(token.substr(name.size() + 1), &nextIdx), false) ||
                nextIdx != token.size() - name.size() - 1)
            {
                return false;
            }
        }
        catch (std::invalid_argument&)
        {
            return false;
        }

        return true;
    }

    bool getTokenString(const std::string& token, const std::string& name, std::string& value)
    {
        try
        {
            if (token.size() < name.size() + 2 ||
                token.substr(0, name.size()) != name ||
                token[name.size()] != '=')
            {
                return false;
            }
        }
        catch (std::invalid_argument&)
        {
            return false;
        }

        value = token.substr(name.size() + 1);
        return true;
    }

    bool getTokenKeyword(const std::string& token, const std::string& name, const std::map<std::string, int>& map, int& value)
    {
        if (token.size() < name.size() + 2 ||
            token.substr(0, name.size()) != name ||
            token[name.size()] != '=')
            return false;

        std::string t = token.substr(name.size()+1);
        if (t[0] == '\'' && t[t.size()-1] == '\'')
            t = t.substr(1, t.size()-2);

        auto p = map.find(t);
        if (p == map.cend())
            return false;

        value = p->second;
        return true;
    }

    bool getTokenInteger(const Poco::StringTokenizer& tokens, const std::string& name, int& value)
    {
        for (size_t i = 0; i < tokens.count(); i++)
        {
            if (getTokenInteger(tokens[i], name, value))
                return true;
        }
        return false;
    }

    bool getTokenString(const Poco::StringTokenizer& tokens, const std::string& name, std::string& value)
    {
        for (size_t i = 0; i < tokens.count(); i++)
        {
            if (getTokenString(tokens[i], name, value))
                return true;
        }
        return false;
    }

    bool getTokenKeyword(const Poco::StringTokenizer& tokens, const std::string& name, const std::map<std::string, int>& map, int& value)
    {
        for (size_t i = 0; i < tokens.count(); i++)
        {
            if (getTokenKeyword(tokens[i], name, map, value))
                return true;
        }
        return false;
    }

    bool getTokenIntegerFromMessage(const std::string& message, const std::string& name, int& value)
    {
        Poco::StringTokenizer tokens(message, " \n", StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
        return getTokenInteger(tokens, name, value);
    }

    bool getTokenStringFromMessage(const std::string& message, const std::string& name, std::string& value)
    {
        if (message.size() > name.size() + 1)
        {
            auto pos = message.find(name);
            while (pos != std::string::npos)
            {
                const auto beg = pos + name.size();
                if (message[beg] == '=')
                {
                    const auto end = message.find_first_of(" \n", beg);
                    value = message.substr(beg + 1, end - beg - 1);
                    return true;
                }

                pos = message.find(name, pos + name.size());
            }
        }

        return false;
    }

    bool getTokenKeywordFromMessage(const std::string& message, const std::string& name, const std::map<std::string, int>& map, int& value)
    {
        Poco::StringTokenizer tokens(message, " \n", StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
        return getTokenKeyword(tokens, name, map, value);
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
