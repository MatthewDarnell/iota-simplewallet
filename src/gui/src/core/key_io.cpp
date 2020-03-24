// Copyright (c) 2014-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <key_io.h>

#include <util/strencodings.h>

bool IsValidDestinationString(const std::string &str)
{
    if(str.size() != 81 && str.size() != 90)
    {
        return false;
    }

    for(auto &&c : str)
    {
        auto lowered = std::tolower(c);
        if((lowered >= 'a' && lowered <= 'z') || lowered == '9')
        {

        }
        else
        {
            return false;
        }
    }

    return true;
}
