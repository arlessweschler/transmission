/*
 * This file Copyright (C) 2007-2014 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 */

#pragma once

#ifndef __TRANSMISSION__
#error only libtransmission should #include this header.
#endif

#include <optional>

#include "transmission.h"
#include "net.h"

/** @addtogroup peers Peers
    @{ */

class tr_peerIo;

/** @brief opaque struct holding hanshake state information.
           freed when the handshake is completed. */
struct tr_handshake;

struct tr_handshake_result
{
    struct tr_handshake* handshake;
    tr_peerIo* io;
    bool readAnythingFromPeer;
    bool isConnected;
    void* userData;
    std::optional<tr_peer_id_t> peer_id;
};

/* returns true on success, false on error */
using tr_handshake_done_func = bool (*)(tr_handshake_result const& result);

/** @brief create a new handshake */
tr_handshake* tr_handshakeNew(
    tr_peerIo* io,
    tr_encryption_mode encryption_mode,
    tr_handshake_done_func when_done,
    void* when_done_user_data);

tr_address const* tr_handshakeGetAddr(struct tr_handshake const* handshake, tr_port* port);

void tr_handshakeAbort(tr_handshake* handshake);

tr_peerIo* tr_handshakeStealIO(tr_handshake* handshake);

/** @} */
