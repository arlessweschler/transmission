/******************************************************************************
 * Copyright (c) 2009-2012 Transmission authors and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *****************************************************************************/

#import <Cocoa/Cocoa.h>

#include <libtransmission/transmission.h>

@class Torrent;

@interface TrackerNode : NSObject

@property(nonatomic, weak, readonly) Torrent* torrent;

- (instancetype)initWithTrackerView:(tr_tracker_view const*)stat torrent:(Torrent*)torrent;

- (BOOL)isEqual:(id)object;

@property(nonatomic, readonly) NSString* host;
@property(nonatomic, readonly) NSString* fullAnnounceAddress;

@property(nonatomic, readonly) NSInteger tier;

@property(nonatomic, readonly) NSUInteger identifier;

@property(nonatomic, readonly) NSInteger totalSeeders;
@property(nonatomic, readonly) NSInteger totalLeechers;
@property(nonatomic, readonly) NSInteger totalDownloaded;

@property(nonatomic, readonly) NSString* lastAnnounceStatusString;
@property(nonatomic, readonly) NSString* nextAnnounceStatusString;
@property(nonatomic, readonly) NSString* lastScrapeStatusString;

@end
