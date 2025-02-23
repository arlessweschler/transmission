/*
 * This file Copyright (C) Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 */

#pragma once

#ifndef __TRANSMISSION__
#error only libtransmission should #include this header.
#endif

#include <cstddef> // size_t
#include <vector>

#include "transmission.h"

#include "bitfield.h"

struct tr_block_info;

class tr_file_piece_map
{
public:
    template<typename T>
    struct index_span_t
    {
        T begin;
        T end;
    };
    using file_span_t = index_span_t<tr_file_index_t>;
    using piece_span_t = index_span_t<tr_piece_index_t>;

    template<typename T>
    struct offset_t
    {
        T index;
        uint64_t offset;
    };

    using file_offset_t = offset_t<tr_file_index_t>;

    explicit tr_file_piece_map(tr_info const& info)
    {
        reset(info);
    }
    tr_file_piece_map(tr_block_info const& block_info, uint64_t const* file_sizes, size_t n_files)
    {
        reset(block_info, file_sizes, n_files);
    }
    void reset(tr_block_info const& block_info, uint64_t const* file_sizes, size_t n_files);
    void reset(tr_info const& info);

    [[nodiscard]] piece_span_t pieceSpan(tr_file_index_t file) const;
    [[nodiscard]] file_span_t fileSpan(tr_piece_index_t piece) const;

    [[nodiscard]] file_offset_t fileOffset(uint64_t offset) const;

    [[nodiscard]] size_t size() const
    {
        return std::size(file_pieces_);
    }

    // TODO(ckerr) minor wart here, two identical span types
    [[nodiscard]] tr_byte_span_t byteSpan(tr_file_index_t file) const
    {
        auto const& span = file_bytes_.at(file);
        return tr_byte_span_t{ span.begin, span.end };
    }

private:
    using byte_span_t = index_span_t<uint64_t>;
    std::vector<byte_span_t> file_bytes_;

    std::vector<piece_span_t> file_pieces_;

    template<typename T>
    struct CompareToSpan
    {
        using span_t = index_span_t<T>;

        int compare(T item, span_t span) const // <=>
        {
            if (item < span.begin)
            {
                return -1;
            }

            if (item >= span.end)
            {
                return 1;
            }

            return 0;
        }

        bool operator()(T item, span_t span) const // <
        {
            return compare(item, span) < 0;
        }

        int compare(span_t span, T item) const // <=>
        {
            return -compare(item, span);
        }

        bool operator()(span_t span, T item) const // <
        {
            return compare(span, item) < 0;
        }
    };
};

class tr_file_priorities
{
public:
    explicit tr_file_priorities(tr_file_piece_map const* fpm);
    void reset(tr_file_piece_map const*);
    void set(tr_file_index_t file, tr_priority_t priority);
    void set(tr_file_index_t const* files, size_t n, tr_priority_t priority);

    [[nodiscard]] tr_priority_t filePriority(tr_file_index_t file) const;
    [[nodiscard]] tr_priority_t piecePriority(tr_piece_index_t piece) const;

private:
    tr_file_piece_map const* fpm_;
    std::vector<tr_priority_t> priorities_;
};

class tr_files_wanted
{
public:
    explicit tr_files_wanted(tr_file_piece_map const* fpm)
        : wanted_(std::size(*fpm))
    {
        reset(fpm);
    }
    void reset(tr_file_piece_map const* fpm);

    void set(tr_file_index_t file, bool wanted);
    void set(tr_file_index_t const* files, size_t n, bool wanted);

    [[nodiscard]] bool fileWanted(tr_file_index_t file) const;
    [[nodiscard]] bool pieceWanted(tr_piece_index_t piece) const;

private:
    tr_file_piece_map const* fpm_;
    tr_bitfield wanted_;
};
