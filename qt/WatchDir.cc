/*
 * This file Copyright (C) 2009-2015 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 */

#include <memory>

#include <QDir>
#include <QFileSystemWatcher>
#include <QTimer>

#include <libtransmission/transmission.h>

#include <libtransmission/torrent-metainfo.h>

#include "Prefs.h"
#include "TorrentModel.h"
#include "WatchDir.h"

/***
****
***/

WatchDir::WatchDir(TorrentModel const& model)
    : model_(model)
{
}

/***
****
***/

int WatchDir::metainfoTest(QString const& filename) const
{
    auto metainfo = tr_torrent_metainfo();
    if (!metainfo.parseTorrentFile(filename.toUtf8().constData()))
    {
        return ERROR;
    }

    if (model_.hasTorrent(TorrentHash{ metainfo.infoHash() }))
    {
        return DUPLICATE;
    }

    return OK;
}

void WatchDir::onTimeout()
{
    auto* t = qobject_cast<QTimer*>(sender());
    QString const filename = t->objectName();

    if (metainfoTest(filename) == OK)
    {
        emit torrentFileAdded(filename);
    }

    t->deleteLater();
}

void WatchDir::setPath(QString const& path, bool is_enabled)
{
    // clear out any remnants of the previous watcher, if any
    watch_dir_files_.clear();
    watcher_.reset();

    // maybe create a new watcher
    if (is_enabled)
    {
        watcher_ = std::make_unique<QFileSystemWatcher>(QStringList{ path });
        connect(watcher_.get(), &QFileSystemWatcher::directoryChanged, this, &WatchDir::watcherActivated);
        // trigger the watchdir for .torrent files in there already
        QTimer::singleShot(0, this, SLOT(rescanAllWatchedDirectories()));
    }
}

void WatchDir::watcherActivated(QString const& path)
{
    QDir const dir(path);

    // get the list of files currently in the watch directory
    QSet<QString> files;

    for (QString const& str : dir.entryList(QDir::Readable | QDir::Files))
    {
        files.insert(str);
    }

    // try to add any new files which end in .torrent
    auto const new_files = files - watch_dir_files_;
    auto const torrent_suffix = QStringLiteral(".torrent");

    for (QString const& name : new_files)
    {
        if (name.endsWith(torrent_suffix, Qt::CaseInsensitive))
        {
            QString const filename = dir.absoluteFilePath(name);

            switch (metainfoTest(filename))
            {
            case OK:
                emit torrentFileAdded(filename);
                break;

            case DUPLICATE:
                break;

            case ERROR:
                {
                    // give the .torrent a few seconds to finish downloading
                    auto* t = new QTimer(this);
                    t->setObjectName(dir.absoluteFilePath(name));
                    t->setSingleShot(true);
                    connect(t, &QTimer::timeout, this, &WatchDir::onTimeout);
                    t->start(5000);
                }
            }
        }
    }

    // update our file list so that we can use it
    // for comparison the next time around
    watch_dir_files_ = files;
}

void WatchDir::rescanAllWatchedDirectories()
{
    if (!watcher_)
    {
        return;
    }

    for (auto const& path : watcher_->directories())
    {
        watcherActivated(path);
    }
}
