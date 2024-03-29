#ifndef LLARP_ROUTER_OUTBOUND_SESSION_MAKER_HPP
#define LLARP_ROUTER_OUTBOUND_SESSION_MAKER_HPP

#include <router/i_outbound_session_maker.hpp>

#include <router/i_rc_lookup_handler.hpp>
#include <util/threading.hpp>
#include <util/thread_pool.hpp>
#include <util/logic.hpp>

#include <unordered_map>
#include <list>
#include <memory>

struct llarp_nodedb;

namespace llarp
{
  struct PendingSession;

  struct ILinkManager;
  struct I_RCLookupHandler;

  struct OutboundSessionMaker final : public IOutboundSessionMaker
  {
    using CallbacksQueue = std::list< RouterCallback >;

   public:
    ~OutboundSessionMaker() override = default;

    bool
    OnSessionEstablished(ILinkSession *session) override;

    void
    OnConnectTimeout(ILinkSession *session) override;

    void
    CreateSessionTo(const RouterID &router, RouterCallback on_result) override
        LOCKS_EXCLUDED(_mutex);

    void
    CreateSessionTo(const RouterContact &rc, RouterCallback on_result) override
        LOCKS_EXCLUDED(_mutex);

    bool
    HavePendingSessionTo(const RouterID &router) const override
        LOCKS_EXCLUDED(_mutex);

    void
    ConnectToRandomRouters(int numDesired, llarp_time_t now) override;

    util::StatusObject
    ExtractStatus() const override;

    bool
    ShouldConnectTo(const RouterID &router) const override
        LOCKS_EXCLUDED(_mutex);

    void
    Init(ILinkManager *linkManager, I_RCLookupHandler *rcLookup,
         std::shared_ptr< Logic > logic, llarp_nodedb *nodedb,
         std::shared_ptr< llarp::thread::ThreadPool > threadpool);

    /// always maintain this many connections to other routers
    size_t minConnectedRouters = 4;
    /// hard upperbound limit on the number of router to router connections
    size_t maxConnectedRouters = 6;

   private:
    void
    DoEstablish(const RouterID &router) LOCKS_EXCLUDED(_mutex);

    void
    GotRouterContact(const RouterID &router, const RouterContact &rc)
        LOCKS_EXCLUDED(_mutex);

    void
    InvalidRouter(const RouterID &router);

    void
    RouterNotFound(const RouterID &router);

    void
    OnRouterContactResult(const RouterID &router, const RouterContact *const rc,
                          const RCRequestResult result);

    void
    VerifyRC(const RouterContact rc);

    void
    CreatePendingSession(const RouterID &router) LOCKS_EXCLUDED(_mutex);

    void
    FinalizeRequest(const RouterID &router, const SessionResult type)
        LOCKS_EXCLUDED(_mutex);

    mutable util::Mutex _mutex;  // protects pendingSessions, pendingCallbacks

    std::unordered_map< RouterID, std::shared_ptr< PendingSession >,
                        RouterID::Hash >
        pendingSessions GUARDED_BY(_mutex);

    std::unordered_map< RouterID, CallbacksQueue, RouterID::Hash >
        pendingCallbacks GUARDED_BY(_mutex);

    ILinkManager *_linkManager;
    I_RCLookupHandler *_rcLookup;
    std::shared_ptr< Logic > _logic;
    llarp_nodedb *_nodedb;
    std::shared_ptr< llarp::thread::ThreadPool > _threadpool;
  };

}  // namespace llarp

#endif  // LLARP_ROUTER_OUTBOUND_SESSION_MAKER_HPP
