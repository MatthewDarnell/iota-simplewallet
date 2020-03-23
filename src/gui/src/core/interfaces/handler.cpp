// Copyright (c) 2018-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <interfaces/handler.h>

namespace boost {
namespace signals2 {
class connection;
} // namespace signals2
} // namespace boost

namespace interfaces {

class HandlerImpl : public Handler
{
    // Handler interface
public:
    HandlerImpl(std::function<void()> cleanup) :
        _cleanup(cleanup)
    { }

    void disconnect() override
    {
        if(_cleanup)
        {
            _cleanup();
        }
    }

private:
    std::function<void()> _cleanup;
};

//! Return handler wrapping a cleanup function.
std::unique_ptr<Handler> MakeHandler(std::function<void()> cleanup)
{
    return std::make_unique<HandlerImpl>(cleanup);
}

} // namespace interfaces
