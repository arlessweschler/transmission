/*
 * This file Copyright (C) 2007-2014 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 */

#pragma once

#include "transmission.h"

struct tr_metainfo_builder_file
{
    char* filename;
    uint64_t size;
};

enum tr_metainfo_builder_err
{
    TR_MAKEMETA_OK,
    TR_MAKEMETA_URL,
    TR_MAKEMETA_CANCELLED,
    TR_MAKEMETA_IO_READ, /* see builder.errfile, builder.my_errno */
    TR_MAKEMETA_IO_WRITE /* see builder.errfile, builder.my_errno */
};

struct tr_tracker_info
{
    int tier;
    char* announce;
    char* scrape;
    uint32_t id; /* unique identifier used to match to a tr_tracker_stat */
};

struct tr_metainfo_builder
{
    /**
    ***  These are set by tr_makeMetaInfoBuilderCreate()
    ***  and cleaned up by tr_metaInfoBuilderFree()
    **/

    char* top;
    tr_metainfo_builder_file* files;
    uint64_t totalSize;
    uint32_t fileCount;
    uint32_t pieceSize;
    uint32_t pieceCount;
    bool isFolder;

    /**
    ***  These are set inside tr_makeMetaInfo()
    ***  by copying the arguments passed to it,
    ***  and cleaned up by tr_metaInfoBuilderFree()
    **/

    tr_tracker_info* trackers;
    int trackerCount;
    char* comment;
    char* outputFile;
    bool isPrivate;
    char* source;

    /**
    ***  These are set inside tr_makeMetaInfo() so the client
    ***  can poll periodically to see what the status is.
    ***  The client can also set abortFlag to nonzero to
    ***  tell tr_makeMetaInfo() to abort and clean up after itself.
    **/

    uint32_t pieceIndex;
    bool abortFlag;
    bool isDone;
    tr_metainfo_builder_err result;

    /* file in use when result was set to _IO_READ or _IO_WRITE,
     * or the URL in use when the result was set to _URL */
    char errfile[2048];

    /* errno encountered when result was set to _IO_READ or _IO_WRITE */
    int my_errno;

    /**
    ***  This is an implementation detail.
    ***  The client should never use these fields.
    **/

    struct tr_metainfo_builder* nextBuilder;
};

tr_metainfo_builder* tr_metaInfoBuilderCreate(char const* topFile);

/**
 * Call this before tr_makeMetaInfo() to override the builder.pieceSize
 * and builder.pieceCount values that were set by tr_metainfoBuilderCreate()
 *
 * @return false if the piece size isn't valid; eg, isn't a power of two.
 */
bool tr_metaInfoBuilderSetPieceSize(tr_metainfo_builder* builder, uint32_t bytes);

void tr_metaInfoBuilderFree(tr_metainfo_builder*);

/**
 * @brief create a new .torrent file
 *
 * This is actually done in a worker thread, not the main thread!
 * Otherwise the client's interface would lock up while this runs.
 *
 * It is the caller's responsibility to poll builder->isDone
 * from time to time!  When the worker thread sets that flag,
 * the caller must pass the builder to tr_metaInfoBuilderFree().
 *
 * @param outputFile if nullptr, builder->top + ".torrent" will be used.

 * @param trackers An array of trackers, sorted by tier from first to last.
 *                 NOTE: only the `tier' and `announce' fields are used.
 *
 * @param trackerCount size of the `trackers' array
 */
void tr_makeMetaInfo(
    tr_metainfo_builder* builder,
    char const* outputFile,
    tr_tracker_info const* trackers,
    int trackerCount,
    char const* comment,
    bool isPrivate,
    char const* source);
