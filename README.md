Checkstream / Genstream
=======================

What are they?
--------------

Genstream and checkstream are a pair of utilities which are useful
in the diagnosis of file corruption problems, and have proven useful
when dealing with several different NFS corruption problems on IRIX
and Linux.

They were developed in the Melbourne group at SGI in the early 2000s,
used there for testing NFS, XFS and Linux HA, and released as open
source in 2009.

How do they work?
-----------------

The basic theory of operation is as follows.

- Genstream generates a stream of data, which comprises a sequence of
  carefully constructed 8 byte records.  Each record contains the file
  offset they're supposed to appear at, and a checksum.

- Checkstream reads such a stream, uses the information in each record
  to checks the stream for corruption, and emits a report describing
  which ranges of records are bad.

Checkstream finds bad records using two criteria.

1. A record is malformed; the record checksum cannot be verified.
   (Note that a record that is completely zero, e.g. due to reading
   holes in the file, will be detected this way).
2. A well-formed record is mislocated; the file offset encoded in the
   record does not match the file offset at which the record was read.

Both utilities do IO in configurable ways (blocksize, file open
flags, using mmap, etc) which allows various modes of getting IO
into the kernel to be tested directly.  They also have a mode where
the record contains an 8-bit tag, allowing for testing for failure
modes involving data being incorrectly moved to the wrong file.

Checkstream also has an option to cause a kernel dump when it detects
an error, which is super useful for finding bugs in kernel filesystem
implementations.

How do I build them?
--------------------

The build system is GNU autotools.  Read the file `INSTALL` for details.

The build produces two binaries `genstream` and `checkstream`.  Their only
dependencies are libc, so you can copy these to your test systems and run them
directly.

How do I use them?
-------------------

Read the manual pages for a discussion of some common testing scenarios.

[![Build Status](https://github.com/gnb/checkstream/workflows/Build%20and%20Test/badge.svg?branch=main)](https://github.com/gnb/checkstream/actions/workflows/build_on_push.yaml)

