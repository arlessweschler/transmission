/*
 * This file Copyright (C) 2012-2021 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 */

#include <array>
#include <memory>
#include <string>

#include <glib/gstdio.h> /* g_remove() */

#include <libtransmission/transmission.h>
#include <libtransmission/web.h> /* tr_webRun() */

#include "FaviconCache.h"
#include "Utils.h" /* gtr_get_host_from_url() */

namespace
{

std::array<char const*, 4> const image_types = { "ico", "png", "gif", "jpg" };

struct favicon_data
{
    tr_session* session = nullptr;
    std::function<void(Glib::RefPtr<Gdk::Pixbuf> const&)> func;
    std::string host;
    std::string contents;
    size_t type = 0;
};

Glib::ustring get_url(std::string const& host, size_t image_type)
{
    return gtr_sprintf("http://%s/favicon.%s", host, image_types[image_type]);
}

std::string favicon_get_cache_dir()
{
    static std::string dir;

    if (dir.empty())
    {
        dir = Glib::build_filename(Glib::get_user_cache_dir(), "transmission", "favicons");
        g_mkdir_with_parents(dir.c_str(), 0777);
    }

    return dir;
}

std::string favicon_get_cache_filename(std::string const& host)
{
    return Glib::build_filename(favicon_get_cache_dir(), host);
}

void favicon_save_to_cache(std::string const& host, std::string const& data)
{
    Glib::file_set_contents(favicon_get_cache_filename(host), data);
}

Glib::RefPtr<Gdk::Pixbuf> favicon_load_from_cache(std::string const& host)
{
    auto const filename = favicon_get_cache_filename(host);

    try
    {
        return Gdk::Pixbuf::create_from_file(filename, 16, 16, false);
    }
    catch (Glib::Error const&)
    {
        g_remove(filename.c_str());
        return {};
    }
}

void favicon_web_done_cb(tr_session*, bool, bool, long, std::string_view, gpointer);

bool favicon_web_done_idle_cb(std::unique_ptr<favicon_data> fav)
{
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;

    if (!fav->contents.empty()) /* we got something... try to make a pixbuf from it */
    {
        favicon_save_to_cache(fav->host, fav->contents);
        pixbuf = favicon_load_from_cache(fav->host);
    }

    if (pixbuf == nullptr && ++fav->type < image_types.size()) /* keep trying */
    {
        fav->contents.clear();
        auto* const session = fav->session;
        auto const next_url = get_url(fav->host, fav->type);
        tr_webRun(session, next_url.c_str(), favicon_web_done_cb, fav.release());
    }

    // Not released into the next web request, means we're done trying (even if `pixbuf` is still invalid)
    if (fav != nullptr)
    {
        fav->func(pixbuf);
    }

    return false;
}

void favicon_web_done_cb(
    tr_session* /*session*/,
    bool /*did_connect*/,
    bool /*did_timeout*/,
    long /*code*/,
    std::string_view data,
    gpointer vfav)
{
    auto* fav = static_cast<favicon_data*>(vfav);
    fav->contents.assign(std::data(data), std::size(data));

    Glib::signal_idle().connect([fav]() { return favicon_web_done_idle_cb(std::unique_ptr<favicon_data>(fav)); });
}

} // namespace

void gtr_get_favicon(
    tr_session* session,
    std::string const& host,
    std::function<void(Glib::RefPtr<Gdk::Pixbuf> const&)> const& pixbuf_ready_func)
{
    auto pixbuf = favicon_load_from_cache(host);

    if (pixbuf != nullptr)
    {
        pixbuf_ready_func(pixbuf);
    }
    else
    {
        auto data = std::make_unique<favicon_data>();
        data->session = session;
        data->func = pixbuf_ready_func;
        data->host = host;

        tr_webRun(session, get_url(host, 0).c_str(), favicon_web_done_cb, data.release());
    }
}

void gtr_get_favicon_from_url(
    tr_session* session,
    Glib::ustring const& url,
    std::function<void(Glib::RefPtr<Gdk::Pixbuf> const&)> const& pixbuf_ready_func)
{
    gtr_get_favicon(session, gtr_get_host_from_url(url), pixbuf_ready_func);
}
