/*
 * This file Copyright (C) Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 */

#include <algorithm>
#include <vector>

#include "transmission.h"

#include "block-info.h"
#include "file-piece-map.h"
#include "tr-assert.h"

void tr_file_piece_map::reset(tr_block_info const& block_info, uint64_t const* file_sizes, size_t n_files)
{
    file_bytes_.resize(n_files);
    file_bytes_.shrink_to_fit();

    file_pieces_.resize(n_files);
    file_pieces_.shrink_to_fit();

    uint64_t offset = 0;
    for (tr_file_index_t i = 0; i < n_files; ++i)
    {
        auto const file_size = file_sizes[i];
        auto const begin_byte = offset;
        auto const begin_piece = block_info.pieceOf(begin_byte);
        auto end_byte = tr_byte_index_t{};
        auto end_piece = tr_piece_index_t{};

        if (file_size != 0)
        {
            end_byte = offset + file_size;
            auto const final_byte = end_byte - 1;
            auto const final_piece = block_info.pieceOf(final_byte);
            end_piece = final_piece + 1;
        }
        else
        {
            end_byte = begin_byte;
            // TODO(ckerr): should end_piece == begin_piece, same as _bytes are?
            end_piece = begin_piece + 1;
        }
        file_pieces_[i] = piece_span_t{ begin_piece, end_piece };
        file_bytes_[i] = byte_span_t{ begin_byte, end_byte };
        offset += file_size;
    }
}

void tr_file_piece_map::reset(tr_info const& info)
{
    auto const n = info.fileCount();
    auto file_sizes = std::vector<uint64_t>(n);
    for (tr_file_index_t i = 0; i < n; ++i)
    {
        file_sizes[i] = info.fileSize(i);
    }
    reset({ info.totalSize(), info.pieceSize() }, std::data(file_sizes), std::size(file_sizes));
}

tr_file_piece_map::piece_span_t tr_file_piece_map::pieceSpan(tr_file_index_t file) const
{
    return file_pieces_[file];
}

tr_file_piece_map::file_span_t tr_file_piece_map::fileSpan(tr_piece_index_t piece) const
{
    auto compare = CompareToSpan<tr_piece_index_t>{};
    auto const begin = std::begin(file_pieces_);
    auto const& [equal_begin, equal_end] = std::equal_range(begin, std::end(file_pieces_), piece, compare);
    return { tr_piece_index_t(std::distance(begin, equal_begin)), tr_piece_index_t(std::distance(begin, equal_end)) };
}

tr_file_piece_map::file_offset_t tr_file_piece_map::fileOffset(uint64_t offset) const
{
    auto compare = CompareToSpan<uint64_t>{};
    auto const begin = std::begin(file_bytes_);
    auto const it = std::lower_bound(begin, std::end(file_bytes_), offset, compare);
    tr_file_index_t const file_index = std::distance(begin, it);
    auto const file_offset = offset - it->begin;
    return file_offset_t{ file_index, file_offset };
}

/***
****
***/

tr_file_priorities::tr_file_priorities(tr_file_piece_map const* fpm)
{
    reset(fpm);
}

void tr_file_priorities::reset(tr_file_piece_map const* fpm)
{
    fpm_ = fpm;

    auto const n = std::size(*fpm_);
    priorities_.resize(n);
    priorities_.shrink_to_fit();
    std::fill_n(std::begin(priorities_), n, TR_PRI_NORMAL);
}

void tr_file_priorities::set(tr_file_index_t file, tr_priority_t priority)
{
    priorities_[file] = priority;
}

void tr_file_priorities::set(tr_file_index_t const* files, size_t n, tr_priority_t priority)
{
    for (size_t i = 0; i < n; ++i)
    {
        set(files[i], priority);
    }
}

tr_priority_t tr_file_priorities::filePriority(tr_file_index_t file) const
{
    TR_ASSERT(file < std::size(priorities_));

    return priorities_[file];
}

tr_priority_t tr_file_priorities::piecePriority(tr_piece_index_t piece) const
{
    auto const [begin_idx, end_idx] = fpm_->fileSpan(piece);
    auto const begin = std::begin(priorities_) + begin_idx;
    auto const end = std::begin(priorities_) + end_idx;
    auto const it = std::max_element(begin, end);
    if (it == end)
    {
        return TR_PRI_NORMAL;
    }
    return *it;
}

/***
****
***/

void tr_files_wanted::reset(tr_file_piece_map const* fpm)
{
    fpm_ = fpm;
    wanted_ = tr_bitfield{ std::size(*fpm) };
    wanted_.setHasAll(); // by default we want all files
}

void tr_files_wanted::set(tr_file_index_t file, bool wanted)
{
    wanted_.set(file, wanted);
}

void tr_files_wanted::set(tr_file_index_t const* files, size_t n, bool wanted)
{
    for (size_t i = 0; i < n; ++i)
    {
        set(files[i], wanted);
    }
}

bool tr_files_wanted::fileWanted(tr_file_index_t file) const
{
    return wanted_.test(file);
}

bool tr_files_wanted::pieceWanted(tr_piece_index_t piece) const
{
    auto const [begin, end] = fpm_->fileSpan(piece);
    return wanted_.count(begin, end) != 0;
}
