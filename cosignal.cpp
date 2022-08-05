#include "cosignal.h"

#include <cstdlib>

void CosignalReceiver::clearCallback() {
    for (const auto& connection: connections) {
        QObject::disconnect(connection);
    }
    connections.clear();
    callback = {};
}

void CosignalReceiver::stopCallback()
{
    clearCallback();
    done = true;
    deleteLater();
}

void Cosignal::promise_type::unhandled_exception() const noexcept {
    std::abort();
}
