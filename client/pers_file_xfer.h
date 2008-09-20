// This file is part of Synecdoche.
// http://synecdoche.googlecode.com/
// Copyright (C) 2005 University of California
//
// Synecdoche is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Synecdoche is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License with Synecdoche.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _PERS_FILE_XFER_H
#define _PERS_FILE_XFER_H

#include <vector>

#include "client_types.h"
#include "file_xfer.h"

/// \file
/// PERS_FILE_XFER represents a "persistent file transfer",
/// i.e. a long-term effort to upload or download a file.
/// This may consist of several "episodes",
/// which are HTTP operations to a particular server.
/// PERS_FILE_XFER manages:
/// - the choice of data servers.
/// - the retry and giveup policies.
/// - restarting partial transfers.
///
/// The FILE_INFO has a list of URLs.
/// For download, the object attempts to download the file
/// from any combination of the URLs.
/// For upload, try to upload the file in its entirety to one of the URLs.
///
/// A PERS_FILE_XFER is created and added to pers_file_xfer_set
/// -# when read from the client state file
///   in FILE_INFO::parse(), CLIENT_STATE::parse_state_file().
/// -# when a FILE_INFO is ready to transfer
///   in CLIENT_STATE::handle_pers_file_xfers().
///
/// A PERS_FILE_XFER p is removed from pers_file_xfer_set and freed when
/// p->pers_xfer_done is true in CLIENT_STATE::handle_pers_file_xfers()
///
/// A FILE_XFER is created and added to file_xfer_set and linked from
/// PERS_FILE_XFER in PERS_FILE_XFER::start_xfer().
///
/// A FILE_XFER is erased from file_xfer_set, unlinked from PERS_FILE_XFER and freed
/// -# when the FILE_XFER is done, in
///   - PERS_FILE_XFER::poll()
///   - PERS_FILE_XFER::check_giveup()
/// -# user request, in
///   - PERS_FILE_XFER::abort()
///   - PERS_FILE_XFER::suspend()
/// -# if the FILE_XFER_SET::insert() fails
///   - PERS_FILE_XFER::start_xfer()
/// 
/// NOTE: when this is done, pers_xfer_done is set
///
/// Pointers:
/// - PERS_FILE_XFER -> FILE_XFER:
///   set in PERS_FILE_XFER::start_xfer()
///    zeroed (see above)
/// - PERS_FILE_XFER -> FILE_INFO:
///   set in PERS_FILE_XFER::init()
/// - FILE_INFO -> PERS_FILE_XFER:
///   set in FILE_INFO::parse(), CLIENT_STATE::handle_pers_file_xfers()
///   zeroed in PERS_FILE_XFER destructor


// Default values for exponential backoff
#define PERS_RETRY_DELAY_MIN    60                // 1 minute
#define PERS_RETRY_DELAY_MAX    (60*60*4)         // 4 hours
/// give up on xfer if this time elapses since last byte xferred
#define PERS_GIVEUP             (60*60*24*7*2)    // 2 weeks

class PERS_FILE_XFER {
    int nretry;                ///< number of retries so far
    double first_request_time;    ///< time of first transfer request
    void do_backoff();

public:
    bool is_upload;
    double next_request_time;     ///< time to next retry the file request
    /// Total time there's been an active FILE_XFER for this PERS_FILE_XFER
    /// Currently not used for anything; not meaningful for throughput
    /// because could include repeated transfer.
    double time_so_far;
    /// When time_so_far was last updated.
    /// Defined only while a transfer is active.
    double last_time;
    /// Save how much is transferred when transfer isn't active, used
    /// to display progress in GUI.
    double last_bytes_xferred;
    bool pers_xfer_done;
    FILE_XFER* fxp;     ///< nonzero if file xfer in progress
    FILE_INFO* fip;

    PERS_FILE_XFER();
    ~PERS_FILE_XFER();
    int init(FILE_INFO*, bool is_file_upload);
    bool poll();
    void transient_failure(int);
    void permanent_failure(int);
    void abort();
    int write(MIOFILE& fout);
    int parse(MIOFILE& fin);
    int create_xfer();
    int start_xfer();
    void suspend();
};

class PERS_FILE_XFER_SET {
public:
    FILE_XFER_SET* file_xfers;
    std::vector<PERS_FILE_XFER*>pers_file_xfers;

    PERS_FILE_XFER_SET(FILE_XFER_SET*);
    int insert(PERS_FILE_XFER*);
    int remove(PERS_FILE_XFER*);
    bool poll();
    void suspend();
};

#endif
