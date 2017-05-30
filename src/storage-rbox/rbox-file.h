/* Copyright (c) 2007-2017 Dovecot authors, see the included COPYING file */
/* Copyright (c) 2017 Tallence AG and the authors, see the included COPYING file */

#ifndef SRC_STORAGE_RBOX_RBOX_FILE_H_
#define SRC_STORAGE_RBOX_RBOX_FILE_H_

#include "dbox-file.h"

/* Globally unique identifier for the message. Preserved when
   copying. */
#define RBOX_METADATA_ALL "METADATA"
#define RBOX_METADATA_GUID "G"
/* POP3 UIDL overriding the default format */
#define RBOX_METADATA_POP3_UIDL "P"
/* POP3 message ordering (for migrated mails) */
#define RBOX_METADATA_POP3_ORDER "O"
/* Received UNIX timestamp in hex */
#define RBOX_METADATA_RECEIVED_TIME "R"
/* Physical message size in hex. Necessary only if it differs from
   the dbox_message_header.message_size_hex, for example because the
   message is compressed. */
#define RBOX_METADATA_PHYSICAL_SIZE "Z"
/* Virtual message size in hex (line feeds counted as CRLF) */
#define RBOX_METADATA_VIRTUAL_SIZE "V"
/* Pointer to external message data. Format is:
   1*(<start offset> <byte count> <options> <ref>) */
#define RBOX_METADATA_EXT_REF "X"
/* Mailbox name where this message was originally saved to.
   When rebuild finds a message whose mailbox is unknown, it's
   placed to this mailbox. */
#define RBOX_METADATA_ORIG_MAILBOX "B"

struct rbox_file {
  struct dbox_file file;
  struct rbox_mailbox *mbox;

  /* 0 while file is being created */
  uint32_t uid;
  /* ceph oid */
  char oid[GUID_128_SIZE];
  /* list of attachment paths while saving/copying message */
  pool_t attachment_pool;
  ARRAY_TYPE(const_string) attachment_paths;
  bool written_to_disk;
};

struct dbox_file *rbox_file_init(struct rbox_mailbox *mbox, uint32_t uid);
struct dbox_file *rbox_file_create(struct rbox_mailbox *mbox);
void rbox_file_free(struct dbox_file *file);

/* Get file's extrefs metadata. */
int rbox_file_get_attachments(struct dbox_file *file, const char **extrefs_r);
/* Returns attachment path for this file, given the source path. The result is
   always <hash>-<guid>-<mailbox_guid>-<uid>. The source path is expected to
   contain <hash>-<guid>[-*]. */
const char *rbox_file_attachment_relpath(struct rbox_file *file, const char *srcpath);

/* Assign UID for a newly created file (by renaming it) */
int rbox_file_assign_uid(struct rbox_file *file, uint32_t uid, bool ignore_if_exists);

int rbox_file_create_fd(struct dbox_file *file, const char *path, bool parents);
/* Move the file to alt path or back. */
int rbox_file_move(struct dbox_file *file, bool alt_path);
/* Unlink file and all of its referenced attachments. */
int rbox_file_unlink_with_attachments(struct rbox_file *sfile);
/* Unlink file and its attachments when rollbacking a saved message. */
int rbox_file_unlink_aborted_save(struct rbox_file *file);

#endif /* SRC_STORAGE_RBOX_RBOX_FILE_H_ */