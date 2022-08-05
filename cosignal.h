#pragma once

#include <functional>
#include <utility>
#include <vector>
#include <coroutine>
#include <QObject>

#include <boost/preprocessor/seq/for_each.hpp>

#define SWITCH(x) {x co_await nullptr;}

#define CASE(o, m) co_await std::tuple{o, m, [&]CASE_ARG

#define CASE_ARG(args...) (args)

class CosignalReceiver
    : public QObject
{
    Q_OBJECT

public:
    void clearCallback();
    void stopCallback();

    std::vector<QMetaObject::Connection> connections;
    std::function<void()> callback;
    bool done = false;
};

struct Cosignal
{
    struct promise_type
    {
        CosignalReceiver* const receiver = new CosignalReceiver{};

        auto get_return_object() const noexcept {
            return Cosignal{};
        }
        auto initial_suspend() const noexcept {
            return std::suspend_never{};
        }
        auto final_suspend() noexcept {
            receiver->stopCallback();
            return std::suspend_never{};
        }
        void unhandled_exception() const noexcept;
        void return_void() noexcept {}

        template <typename Obj, typename... Args, typename Callback>
        auto await_transform(std::tuple<Obj*, void(Obj::*)(Args...), Callback> t) noexcept {
            receiver->connections.push_back(QObject::connect(std::get<0>(t), std::get<1>(t), receiver, [this, callback = std::get<2>(t)](Args... args){
                if (!this->receiver->done && !this->receiver->callback) {
                    this->receiver->callback = [callback, args...]{
                        callback(args...);
                    };
                    std::coroutine_handle<promise_type>::from_promise(*this).resume();
                }
            }, Qt::QueuedConnection));
            return std::suspend_never{};
        }
        auto await_transform(std::nullptr_t) noexcept {
            struct Awaitable {
                CosignalReceiver* const receiver;
                Awaitable(CosignalReceiver* receiver)
                    : receiver{ receiver }
                { }

                bool await_ready() const noexcept { return false; }
                void await_suspend(std::coroutine_handle<>) const noexcept {}
                void await_resume() const noexcept {
                    auto callback = std::move(receiver->callback);
                    receiver->clearCallback();
                    if (callback) {
                        callback();
                    }
                }
            };
            return Awaitable{ receiver };
        }
    };
};
