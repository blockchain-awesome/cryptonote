// Copyright (c) 2011-2016 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <atomic>

#include <common/ObserverManager.h>

#include "cryptonote/core/ICore.h"

#include "cryptonote/protocol/definitions.h"
#include "cryptonote/protocol/handler_common.h"
#include "cryptonote/protocol/i_observer.h"
#include "cryptonote/protocol/i_query.h"

#include "p2p/P2pProtocolDefinitions.h"
#include "p2p/NetNodeCommon.h"
#include "p2p/ConnectionContext.h"

#include <logging/LoggerRef.h>

namespace System {
  class Dispatcher;
}

namespace cryptonote
{
  class Currency;

  class CryptoNoteProtocolHandler : 
    public ICryptonoteProtocol, 
    public ICryptoNoteProtocolQuery
  {
  public:

    CryptoNoteProtocolHandler(const Currency& currency, System::Dispatcher& dispatcher, ICore& rcore, IP2pEndpoint* p_net_layout, Logging::ILogger& log);

    virtual ~CryptoNoteProtocolHandler() {};
    virtual bool addObserver(ICryptoNoteProtocolObserver* observer) override;
    virtual bool removeObserver(ICryptoNoteProtocolObserver* observer) override;

    void set_p2p_endpoint(IP2pEndpoint* p2p);
    // ICore& get_core() { return m_core; }
    virtual bool isSynchronized() const override { return m_synchronized; }
    void log_connections();

    // Interface t_payload_net_handler, where t_payload_net_handler is template argument of nodetool::node_server
    void stop();
    bool start_sync(CryptoNoteConnectionContext& context);
    bool on_idle();
    void onConnectionOpened(CryptoNoteConnectionContext& context);
    void onConnectionClosed(CryptoNoteConnectionContext& context);
    bool get_stat_info(CoreStateInfo& stat_inf);
    bool get_payload_sync_data(CORE_SYNC_DATA& hshd);
    bool process_payload_sync_data(const CORE_SYNC_DATA& hshd, CryptoNoteConnectionContext& context, bool is_inital);
    int handleCommand(bool is_notify, int command, const binary_array_t& in_buff, binary_array_t& buff_out, CryptoNoteConnectionContext& context, bool& handled);
    virtual size_t getPeerCount() const override;
    virtual uint32_t getObservedHeight() const override;
    void requestMissingPoolTransactions(const CryptoNoteConnectionContext& context);
    const Currency& m_currency;

  private:
    //----------------- commands handlers ----------------------------------------------
    int handle_notify_new_block(int command, NOTIFY_NEW_BLOCK::request& arg, CryptoNoteConnectionContext& context);
    int handle_notify_new_transactions(int command, NOTIFY_NEW_TRANSACTIONS::request& arg, CryptoNoteConnectionContext& context);
    int handle_request_get_objects(int command, NOTIFY_REQUEST_GET_OBJECTS::request& arg, CryptoNoteConnectionContext& context);
    int handle_response_get_objects(int command, NOTIFY_RESPONSE_GET_OBJECTS::request& arg, CryptoNoteConnectionContext& context);
    int handle_request_chain(int command, NOTIFY_REQUEST_CHAIN::request& arg, CryptoNoteConnectionContext& context);
    int handle_response_chain_entry(int command, NOTIFY_RESPONSE_CHAIN_ENTRY::request& arg, CryptoNoteConnectionContext& context);
    int handleRequestTxPool(int command, NOTIFY_REQUEST_TX_POOL::request& arg, CryptoNoteConnectionContext& context);

    //----------------- ICryptonoteProtocol ----------------------------------
    virtual void relay_block(NOTIFY_NEW_BLOCK::request& arg) override;
    virtual void relay_transactions(NOTIFY_NEW_TRANSACTIONS::request& arg) override;

    //----------------------------------------------------------------------------------
    uint32_t get_current_blockchain_height();
    bool request_missing_objects(CryptoNoteConnectionContext& context, bool check_having_blocks);
    bool on_connection_synchronized();
    void updateObservedHeight(uint32_t peerHeight, const CryptoNoteConnectionContext& context);
    void recalculateMaxObservedHeight(const CryptoNoteConnectionContext& context);
    int processObjects(CryptoNoteConnectionContext& context, const std::vector<block_complete_entry>& blocks);
    Logging::LoggerRef logger;

  private:
    System::Dispatcher& m_dispatcher;
    ICore& m_core;

    p2p_endpoint_stub m_p2p_stub;
    IP2pEndpoint* m_p2p;
    std::atomic<bool> m_synchronized;
    std::atomic<bool> m_stop;

    mutable std::mutex m_observedHeightMutex;
    uint32_t m_observedHeight;

    std::atomic<size_t> m_peersCount;
    Tools::ObserverManager<ICryptoNoteProtocolObserver> m_observerManager;
  };
}
