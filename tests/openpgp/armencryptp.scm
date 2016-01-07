#!/usr/bin/env gpgscm

(load (with-path "defs.scm"))

(for-each-p
 "Checking armored encryption and decryption using pipes"
 (lambda (source)
   (tr:do
    (tr:open source)
    (tr:pipe-do
     (pipe:gpg `(--yes -ea --recipient ,usrname2))
     (pipe:gpg '(--yes)))
    (tr:assert-identity source)))
 (append plain-files data-files))