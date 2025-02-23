/*
 * This file Copyright (C) 2009-2015 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 */

#pragma once

#include <QToolButton>

#include <libtransmission/tr-macros.h>

class IconToolButton : public QToolButton
{
    Q_OBJECT
    TR_DISABLE_COPY_MOVE(IconToolButton)

public:
    explicit IconToolButton(QWidget* parent = nullptr);

    // QWidget
    QSize sizeHint() const override;

protected:
    // QWidget
    void paintEvent(QPaintEvent* event) override;
};
