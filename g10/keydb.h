/* keydb.h - Key database
 *	Copyright (C) 1998 Free Software Foundation, Inc.
 *
 * This file is part of GNUPG.
 *
 * GNUPG is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GNUPG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifndef G10_KEYDB_H
#define G10_KEYDB_H

#include "types.h"
#include "packet.h"
#include "cipher.h"



/****************
 * A Keyblock is all packets which form an entire certificate;
 * i.e. the public key, certificate, trust packets, user ids,
 * signatures, and subkey.
 *
 * This structure is also used to bind arbitrary packets together.
 */

typedef struct kbnode_struct *KBNODE;
struct kbnode_struct {
    KBNODE next;
    PACKET *pkt;
    int flag;
    int private_flag;
};

/****************
 * A data structre to hold information about the external position
 * of a keyblock.
 */
struct keyblock_pos_struct {
    int   resno;     /* resource number */
    ulong offset;    /* position information */
    unsigned count;  /* length of the keyblock in packets */
    IOBUF  fp;	     /* used by enum_keyblocks */
    int secret;      /* working on a secret keyring */
    PACKET *pkt;     /* ditto */
};
typedef struct keyblock_pos_struct KBPOS;

/* structure to hold a couple of public key certificates */
typedef struct pkc_list *PKC_LIST;
struct pkc_list {
    PKC_LIST next;
    PKT_public_cert *pkc;
    int mark;
};

/* structure to hold a couple of secret key certificates */
typedef struct skc_list *SKC_LIST;
struct skc_list {
    SKC_LIST next;
    PKT_secret_cert *skc;
    int mark;
};

/* structure to collect all information which can be used to
 * identify a public key */
typedef struct pubkey_find_info *PUBKEY_FIND_INFO;
struct pubkey_find_info {
    u32  keyid[2];
    unsigned nbits;
    byte pubkey_algo;
    byte fingerprint[20];
    char userid[1];
};


/*-- pkclist.c --*/
void release_pkc_list( PKC_LIST pkc_list );
int  build_pkc_list( STRLIST remusr, PKC_LIST *ret_pkc_list, unsigned usage );

/*-- skclist.c --*/
void release_skc_list( SKC_LIST skc_list );
int  build_skc_list( STRLIST locusr, SKC_LIST *ret_skc_list,
					    int unlock, unsigned usage );

/*-- passphrase.h --*/
void set_passphrase_fd( int fd );
int  get_passphrase_fd(void);
DEK *passphrase_to_dek( u32 *keyid, int cipher_algo, STRING2KEY *s2k, int mode);
void set_next_passphrase( const char *s );
char *get_last_passphrase(void);

/*-- getkey.c --*/
void add_keyring( const char *name );
const char *get_keyring( int sequence );
const char *get_secret_keyring( int sequence );
void add_secret_keyring( const char *name );
int get_pubkey( PKT_public_cert *pkc, u32 *keyid );
int get_pubkey_byname( PKT_public_cert *pkc, const char *name );
int get_seckey( PKT_secret_cert *skc, u32 *keyid );
int get_keyblock_byfprint( KBNODE *ret_keyblock, const byte *fprint,
						 size_t fprint_len );
int seckey_available( u32 *keyid );
int get_seckey_byname( PKT_secret_cert *skc, const char *name, int unlock );
int enum_secret_keys( void **context, PKT_secret_cert *skc );
char*get_user_id_string( u32 *keyid );
char*get_user_id( u32 *keyid, size_t *rn );

/*-- keyid.c --*/
int pubkey_letter( int algo );
u32 keyid_from_skc( PKT_secret_cert *skc, u32 *keyid );
u32 keyid_from_pkc( PKT_public_cert *pkc, u32 *keyid );
u32 keyid_from_sig( PKT_signature *sig, u32 *keyid );
unsigned nbits_from_pkc( PKT_public_cert *pkc );
unsigned nbits_from_skc( PKT_secret_cert *skc );
const char *datestr_from_pkc( PKT_public_cert *pkc );
const char *datestr_from_skc( PKT_secret_cert *skc );
const char *datestr_from_sig( PKT_signature *sig );
byte *fingerprint_from_skc( PKT_secret_cert *skc, size_t *ret_len );
byte *fingerprint_from_pkc( PKT_public_cert *pkc, size_t *ret_len );

/*-- kbnode.c --*/
KBNODE new_kbnode( PACKET *pkt );
KBNODE clone_kbnode( KBNODE node );
void release_kbnode( KBNODE n );
void delete_kbnode( KBNODE node );
void add_kbnode( KBNODE root, KBNODE node );
void insert_kbnode( KBNODE root, KBNODE node, int pkttype );
KBNODE find_prev_kbnode( KBNODE root, KBNODE node, int pkttype );
KBNODE find_next_kbnode( KBNODE node, int pkttype );
KBNODE find_kbnode( KBNODE node, int pkttype );
KBNODE walk_kbnode( KBNODE root, KBNODE *context, int all );
void clear_kbnode_flags( KBNODE n );
int  commit_kbnode( KBNODE *root );

/*-- ringedit.c --*/
int add_keyblock_resource( const char *filename, int force, int secret );
const char *keyblock_resource_name( KBPOS *kbpos );
int get_keyblock_handle( const char *filename, int secret, KBPOS *kbpos );
int find_keyblock( PUBKEY_FIND_INFO info, KBPOS *kbpos );
int find_keyblock_byname( KBPOS *kbpos, const char *username );
int find_keyblock_bypkc( KBPOS *kbpos, PKT_public_cert *pkc );
int find_secret_keyblock_byname( KBPOS *kbpos, const char *username );
int lock_keyblock( KBPOS *kbpos );
void unlock_keyblock( KBPOS *kbpos );
int read_keyblock( KBPOS *kbpos, KBNODE *ret_root );
int enum_keyblocks( int mode, KBPOS *kbpos, KBNODE *ret_root );
int insert_keyblock( KBPOS *kbpos, KBNODE root );
int delete_keyblock( KBPOS *kbpos );
int update_keyblock( KBPOS *kbpos, KBNODE root );


#endif /*G10_KEYDB_H*/
