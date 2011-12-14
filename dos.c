/*
 * dos.c
 * Detection of DOS parition maps and file systems
 *
 * Copyright (c) 2003-2006 Christoph Pfisterer
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 */

#include "global.h"

/*
 * DOS partition types
 *
 * Taken from fdisk/i386_sys_types.c and fdisk/common.h of
 * util-linux 2.11n (as packaged by Debian), Feb 08, 2003.
 */

struct systypes {
  unsigned char type;
  char *name;
};

struct systypes i386_sys_types[] = {
  { 0x00, "Empty" },
  { 0x01, "FAT12" },
  { 0x02, "XENIX root" },
  { 0x03, "XENIX usr" },
  { 0x04, "FAT16 <32M" },
  { 0x05, "Extended" },
  { 0x06, "FAT16" },
  { 0x07, "HPFS/NTFS" },
  { 0x08, "AIX" },
  { 0x09, "AIX bootable" },
  { 0x0a, "OS/2 Boot Manager" },
  { 0x0b, "Win95 FAT32" },
  { 0x0c, "Win95 FAT32 (LBA)" },
  { 0x0e, "Win95 FAT16 (LBA)" },
  { 0x0f, "Win95 Ext'd (LBA)" },
  { 0x10, "OPUS" },
  { 0x11, "Hidden FAT12" },
  { 0x12, "Compaq diagnostics" },
  { 0x14, "Hidden FAT16 <32M" },
  { 0x16, "Hidden FAT16" },
  { 0x17, "Hidden HPFS/NTFS" },
  { 0x18, "AST SmartSleep" },
  { 0x1b, "Hidden Win95 FAT32" },
  { 0x1c, "Hidden Win95 FAT32 (LBA)" },
  { 0x1e, "Hidden Win95 FAT16 (LBA)" },
  { 0x24, "NEC DOS" },
  { 0x39, "Plan 9" },
  { 0x3c, "PartitionMagic recovery" },
  { 0x40, "Venix 80286" },
  { 0x41, "PPC PReP Boot" },
  { 0x42, "SFS / MS LDM" },
  { 0x4d, "QNX4.x" },
  { 0x4e, "QNX4.x 2nd part" },
  { 0x4f, "QNX4.x 3rd part" },
  { 0x50, "OnTrack DM" },
  { 0x51, "OnTrack DM6 Aux1" },
  { 0x52, "CP/M" },
  { 0x53, "OnTrack DM6 Aux3" },
  { 0x54, "OnTrackDM6" },
  { 0x55, "EZ-Drive" },
  { 0x56, "Golden Bow" },
  { 0x5c, "Priam Edisk" },
  { 0x61, "SpeedStor" },
  { 0x63, "GNU HURD or SysV" },
  { 0x64, "Novell Netware 286" },
  { 0x65, "Novell Netware 386" },
  { 0x70, "DiskSecure Multi-Boot" },
  { 0x75, "PC/IX" },
  { 0x78, "XOSL" },
  { 0x80, "Old Minix" },
  { 0x81, "Minix / old Linux" },
  { 0x82, "Linux swap / Solaris" },
  { 0x83, "Linux" },
  { 0x84, "OS/2 hidden C: drive" },
  { 0x85, "Linux extended" },
  { 0x86, "NTFS volume set" },
  { 0x87, "NTFS volume set" },
  { 0x8e, "Linux LVM" },
  { 0x93, "Amoeba" },
  { 0x94, "Amoeba BBT" },
  { 0x9f, "BSD/OS" },
  { 0xa0, "IBM Thinkpad hibernation" },
  { 0xa5, "FreeBSD" },
  { 0xa6, "OpenBSD" },
  { 0xa7, "NeXTSTEP" },
  { 0xa9, "NetBSD" },
  { 0xaf, "Mac OS X" },
  { 0xb7, "BSDI fs" },
  { 0xb8, "BSDI swap" },
  { 0xbb, "Boot Wizard hidden" },
  { 0xc1, "DRDOS/sec (FAT-12)" },
  { 0xc4, "DRDOS/sec (FAT-16 < 32M)" },
  { 0xc6, "DRDOS/sec (FAT-16)" },
  { 0xc7, "Syrinx" },
  { 0xda, "Non-FS data" },
  { 0xdb, "CP/M / CTOS / ..." },
  { 0xde, "Dell Utility" },
  { 0xdf, "BootIt" },
  { 0xe1, "DOS access" },
  { 0xe3, "DOS R/O" },
  { 0xe4, "SpeedStor" },
  { 0xeb, "BeOS fs" },
  { 0xee, "EFI GPT protective" },
  { 0xef, "EFI System (FAT)" },
  { 0xf0, "Linux/PA-RISC boot" },
  { 0xf1, "SpeedStor" },
  { 0xf4, "SpeedStor" },
  { 0xf2, "DOS secondary" },
  { 0xfd, "Linux raid autodetect" },
  { 0xfe, "LANstep" },
  { 0xff, "BBT" },
  { 0, 0 }
};


char * get_name_for_mbrtype(int type)
{
  int i;

  for (i = 0; i386_sys_types[i].name; i++)
    if (i386_sys_types[i].type == type)
      return i386_sys_types[i].name;
  return "Unknown";
}

/*
 * DOS-style partition map / MBR
 */

static void detect_dos_partmap_ext(SECTION *section, u8 extbase,
				   int level, int *extpartnum);

void detect_dos_partmap(SECTION *section, int level)
{
  unsigned char *buf;
  int i, off, used, type, types[4], bootflags[4];
  u4 start, size, starts[4], sizes[4];
  int extpartnum = 5;
  char s[256], append[64];

  /* partition maps only occur at the start of a device */
  if (section->pos != 0)
    return;

  if (get_buffer(section, 0, 512, (void **)&buf) < 512)
    return;

  /* check signature */
  if (buf[510] != 0x55 || buf[511] != 0xAA)
    return;

  /* get entries and check */
  used = 0;
  for (off = 446, i = 0; i < 4; i++, off += 16) {
    /* get data */
    bootflags[i] = buf[off];
    types[i] = buf[off + 4];
    starts[i] = get_le_long(buf + off + 8);
    sizes[i] = get_le_long(buf + off + 12);

    /* bootable flag: either on or off */
    if (bootflags[i] != 0x00 && bootflags[i] != 0x80)
      return;
    /* size non-zero -> entry in use */
    if (starts[i] && sizes[i])
      used = 1;
  }
  if (!used)
    return;

  /* parse the data for real */
  print_line(level, "DOS/MBR partition map");
  for (i = 0; i < 4; i++) {
    start = starts[i];
    size = sizes[i];
    type = types[i];
    if (start == 0 || size == 0)
      continue;

    sprintf(append, " from %lu", start);
    if (bootflags[i] == 0x80)
      strcat(append, ", bootable");
    format_blocky_size(s, size, 512, "sectors", append);
    print_line(level, "Partition %d: %s",
	       i+1, s);

    print_line(level + 1, "Type 0x%02X (%s)", type, get_name_for_mbrtype(type));

    if (type == 0x05 || type == 0x0f || type == 0x85) {
      /* extended partition */
      detect_dos_partmap_ext(section, start, level + 1, &extpartnum);
    } else if (type != 0xee) {
      /* recurse for content detection */
      analyze_recursive(section, level + 1,
			(u8)start * 512, (u8)size * 512, 0);
    }
  }
}

static void detect_dos_partmap_ext(SECTION *section, u8 extbase,
				   int level, int *extpartnum)
{
  unsigned char *buf;
  u8 tablebase, nexttablebase;
  int i, off, type, types[4];
  u4 start, size, starts[4], sizes[4];
  char s[256], append[64];

  for (tablebase = extbase; tablebase; tablebase = nexttablebase) {
    /* read sector from linked list */
    if (get_buffer(section, tablebase << 9, 512, (void **)&buf) < 512)
      return;

    /* check signature */
    if (buf[510] != 0x55 || buf[511] != 0xAA) {
      print_line(level, "Signature missing");
      return;
    }

    /* get entries */
    for (off = 446, i = 0; i < 4; i++, off += 16) {
      types[i] = buf[off + 4];
      starts[i] = get_le_long(buf + off + 8);
      sizes[i] = get_le_long(buf + off + 12);
    }

    /* parse the data for real */
    nexttablebase = 0;
    for (i = 0; i < 4; i++) {
      start = starts[i];
      size = sizes[i];
      type = types[i];
      if (size == 0)
	continue;

      if (type == 0x05 || type == 0x85) {
	/* inner extended partition */

	nexttablebase = extbase + start;

      } else {
	/* logical partition */

	sprintf(append, " from %llu+%lu", tablebase, start);
	format_blocky_size(s, size, 512, "sectors", append);
	print_line(level, "Partition %d: %s",
		   *extpartnum, s);
	(*extpartnum)++;
	print_line(level + 1, "Type 0x%02X (%s)", type, get_name_for_mbrtype(type));

	/* recurse for content detection */
	if (type != 0xee) {
	  analyze_recursive(section, level + 1,
			    (tablebase + start) * 512, (u8)size * 512, 0);
	}
      }
    }
  }
}

/*
 * EFI GPT partition map
 */

struct gpttypes {
  char *guid;
  char *name;
};

struct gpttypes gpt_types[] = {
  { "\x28\x73\x2A\xC1\x1F\xF8\xD2\x11\xBA\x4B\x00\xA0\xC9\x3E\xC9\x3B", "EFI System (FAT)" },
  { "\x41\xEE\x4D\x02\xE7\x33\xD3\x11\x9D\x69\x00\x08\xC7\x81\xF3\x9F", "MBR partition scheme" },
  { "\x16\xE3\xC9\xE3\x5C\x0B\xB8\x4D\x81\x7D\xF9\x2D\xF0\x02\x15\xAE", "MS Reserved" },
  { "\xA2\xA0\xD0\xEB\xE5\xB9\x33\x44\x87\xC0\x68\xB6\xB7\x26\x99\xC7", "Basic Data" },
  { "\xAA\xC8\x08\x58\x8F\x7E\xE0\x42\x85\xD2\xE1\xE9\x04\x34\xCF\xB3", "MS LDM Metadata" },
  { "\xA0\x60\x9B\xAF\x31\x14\x62\x4F\xBC\x68\x33\x11\x71\x4A\x69\xAD", "MS LDM Data" },
  { "\x1E\x4C\x89\x75\xEB\x3A\xD3\x11\xB7\xC1\x7B\x03\xA0\x00\x00\x00", "HP/UX Data" },
  { "\x28\xE7\xA1\xE2\xE3\x32\xD6\x11\xA6\x82\x7B\x03\xA0\x00\x00\x00", "HP/UX Service" },
  { "\x0F\x88\x9D\xA1\xFC\x05\x3B\x4D\xA0\x06\x74\x3F\x0F\x84\x91\x1E", "Linux RAID" },
  { "\x6D\xFD\x57\x06\xAB\xA4\xC4\x43\x84\xE5\x09\x33\xC8\x4B\x4F\x4F", "Linux Swap" },
  { "\x79\xD3\xD6\xE6\x07\xF5\xC2\x44\xA2\x3C\x23\x8F\x2A\x3D\xF9\x28", "Linux LVM" },
  { "\x39\x33\xA6\x8D\x07\x00\xC0\x60\xC4\x36\x08\x3A\xC8\x23\x09\x08", "Linux Reserved" },
  { "\xB4\x7C\x6E\x51\xCF\x6E\xD6\x11\x8F\xF8\x00\x02\x2D\x09\x71\x2B", "FreeBSD Data" },
  { "\xB5\x7C\x6E\x51\xCF\x6E\xD6\x11\x8F\xF8\x00\x02\x2D\x09\x71\x2B", "FreeBSD Swap" },
  { "\xB6\x7C\x6E\x51\xCF\x6E\xD6\x11\x8F\xF8\x00\x02\x2D\x09\x71\x2B", "FreeBSD UFS" },
  { "\xB8\x7C\x6E\x51\xCF\x6E\xD6\x11\x8F\xF8\x00\x02\x2D\x09\x71\x2B", "FreeBSD Vinum" },
  { "\x00\x53\x46\x48\x00\x00\xAA\x11\xAA\x11\x00\x30\x65\x43\xEC\xAC", "Mac HFS+" },
  { 0, 0 }
};

static char * get_name_for_guid(void *guid)
{
  int i;

  for (i = 0; gpt_types[i].name; i++)
    if (memcmp(gpt_types[i].guid, guid, 16) == 0)
      return gpt_types[i].name;
  return "Unknown";
}

void detect_gpt_partmap(SECTION *section, int level)
{
  unsigned char *buf;
  u8 diskblocks, partmap_start, start, end, size;
  u4 partmap_count, partmap_entry_size;
  u4 i;
  char s[256], append[64];
  int last_unused;

  /* partition maps only occur at the start of a device */
  if (section->pos != 0)
    return;

  /* get LBA 1: GPT header */
  if (get_buffer(section, 512, 512, (void **)&buf) < 512)
    return;

  /* check signature */
  if (memcmp(buf, "EFI PART", 8) != 0)
    return;

  /* get header information */
  if (get_le_quad(buf + 0x18) != 1)
    return;
  diskblocks = get_le_quad(buf + 0x20) + 1;
  partmap_start = get_le_quad(buf + 0x48);
  partmap_count = get_le_long(buf + 0x50);
  partmap_entry_size = get_le_long(buf + 0x54);

  print_line(level, "GPT partition map, %d entries", (int)partmap_count);
  format_blocky_size(s, diskblocks, 512, "sectors", NULL);
  print_line(level+1, "Disk size %s", s);
  format_guid(buf + 0x38, s);
  print_line(level+1, "Disk GUID %s", s);

  /* get entries */
  last_unused = 0;
  for (i = 0; i < partmap_count; i++) {
    if (get_buffer(section, (partmap_start * 512) + i * partmap_entry_size, partmap_entry_size, (void **)&buf) < partmap_entry_size)
      return;

    if (memcmp(buf, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16) == 0) {
      if (last_unused == 0)
	print_line(level, "Partition %d: unused", i+1);
      last_unused = 1;
      continue;
    }
    last_unused = 0;

    /* size */
    start = get_le_quad(buf + 0x20);
    end = get_le_quad(buf + 0x28);
    size = end + 1 - start;

    sprintf(append, " from %llu", start);
    format_blocky_size(s, size, 512, "sectors", append);
    print_line(level, "Partition %d: %s", i+1, s);

    /* type */
    format_guid(buf, s);
    print_line(level+1, "Type %s (GUID %s)", get_name_for_guid(buf), s);

    /* partition name */
    format_utf16_le(buf + 0x38, 72, s);
    print_line(level+1, "Partition Name \"%s\"", s);

    /* GUID */
    format_guid(buf + 0x10, s);
    print_line(level+1, "Partition GUID %s", s);

    /* recurse for content detection */
    if (start > 0 && size > 0) {  /* avoid recursion on self */
      analyze_recursive(section, level + 1,
			start * 512, size * 512, 0);
    }
  }
}

/*
 * FAT12/FAT16/FAT32 file systems
 */

static char *fatnames[] = { "FAT12", "FAT16", "FAT32" };

void detect_fat(SECTION *section, int level)
{
  int i, score, fattype;
  u4 sectsize, clustersize, reserved, fatcount, dirsize, fatsize;
  u8 sectcount, clustercount;
  u2 atari_csum;
  unsigned char *buf;
  char s[256];

  if (get_buffer(section, 0, 512, (void **)&buf) < 512)
    return;

  /* first, some hard tests */
  /* sector size has four allowed values */
  sectsize = get_le_short(buf + 11);
  if (sectsize != 512 && sectsize != 1024 &&
      sectsize != 2048 && sectsize != 4096)
    return;
  /* sectors per cluster: must be a power of two */
  clustersize = buf[13];
  if (clustersize == 0 || (clustersize & (clustersize - 1)))
    return;
  /* since the above is also present on NTFS, make sure it's not NTFS... */
  if (memcmp(buf + 3, "NTFS    ", 8) == 0)
    return;

  /* next, some soft tests, taking score */
  score = 0;

  /* boot jump */
  if ((buf[0] == 0xEB && buf[2] == 0x90) || 
      buf[0] == 0xE9)
    score++;
  /* boot signature */
  if (buf[510] == 0x55 && buf[511] == 0xAA)
    score++;
  /* reserved sectors */
  reserved = get_le_short(buf + 14);
  if (reserved == 1 || reserved == 32)
    score++;
  /* number of FATs */
  fatcount = buf[16];
  if (fatcount == 2)
    score++;
  /* number of root dir entries */
  dirsize = get_le_short(buf + 17);
  /* sector count (16-bit and 32-bit versions) */
  sectcount = get_le_short(buf + 19);
  if (sectcount == 0)
    sectcount = get_le_long(buf + 32);
  /* media byte */
  if (buf[21] == 0xF0 || buf[21] >= 0xF8)
    score++;
  /* FAT size in sectors */
  fatsize = get_le_short(buf + 22);
  if (fatsize == 0)
    fatsize = get_le_long(buf + 36);

  /* determine FAT type */
  dirsize = ((dirsize * 32) + (sectsize - 1)) / sectsize;
  clustercount = sectcount - (reserved + (fatcount * fatsize) + dirsize);
  clustercount /= clustersize;

  if (clustercount < 4085)
    fattype = 0;
  else if (clustercount < 65525)
    fattype = 1;
  else
    fattype = 2;

  /* check for ATARI ST boot checksum */
  atari_csum = 0;
  for (i = 0; i < 512; i += 2)
    atari_csum += get_be_short(buf + i);

  /* tell the user */
  s[0] = 0;
  if (atari_csum == 0x1234)
    strcpy(s, ", ATARI ST bootable");
  print_line(level, "%s file system (hints score %d of %d%s)",
	     fatnames[fattype], score, 5, s);

  if (sectsize > 512)
    print_line(level + 1, "Unusual sector size %lu bytes", sectsize);

  format_blocky_size(s, clustercount, clustersize * sectsize,
		     "clusters", NULL);
  print_line(level + 1, "Volume size %s", s);

  /* get the cached volume name if present */
  if (fattype < 2) {
    if (buf[38] == 0x29) {
      memcpy(s, buf + 43, 11);
      s[11] = 0;
      for (i = 10; i >= 0 && s[i] == ' '; i--)
	s[i] = 0;
      if (strcmp(s, "NO NAME") != 0)
	print_line(level + 1, "Volume name \"%s\"", s);
    }
  } else {
    if (buf[66] == 0x29) {
      memcpy(s, buf + 71, 11);
      s[11] = 0;
      for (i = 10; i >= 0 && s[i] == ' '; i--)
	s[i] = 0;
      if (strcmp(s, "NO NAME") != 0)
	print_line(level + 1, "Volume name \"%s\"", s);
    }
  }
}

/*
 * NTFS file system
 */

void detect_ntfs(SECTION *section, int level)
{
  u4 sectsize, clustersize;
  u8 sectcount;
  unsigned char *buf;
  char s[256];

  if (get_buffer(section, 0, 512, (void **)&buf) < 512)
    return;

  /* check signatures */
  if (memcmp(buf + 3, "NTFS    ", 8) != 0)
    return;
  /* disabled for now, mkntfs(8) doesn't generate it
  if (memcmp(buf + 0x24, "\x80\x00\x80\x00", 4) != 0)
    return;
  */

  /* sector size: must be a power of two */
  sectsize = get_le_short(buf + 11);
  if (sectsize < 512 || (sectsize & (sectsize - 1)))
    return;
  /* sectors per cluster: must be a power of two */
  clustersize = buf[13];
  if (clustersize == 0 || (clustersize & (clustersize - 1)))
    return;

  /* get size in sectors */
  sectcount = get_le_quad(buf + 0x28);

  /* tell the user */
  print_line(level, "NTFS file system");

  format_blocky_size(s, sectcount, sectsize, "sectors", NULL);
  print_line(level + 1, "Volume size %s", s);
}

/*
 * HPFS file system
 */

void detect_hpfs(SECTION *section, int level)
{
  unsigned char *buf;
  char s[256];
  u8 sectcount;

  if (get_buffer(section, 16*512, 512, (void **)&buf) < 512)
    return;

  if (memcmp(buf, "\xF9\x95\xE8\x49\xFA\x53\xE9\xC5", 8) != 0)
    return;

  print_line(level, "HPFS file system (version %d, functional version %d)",
	     (int)buf[8], (int)buf[9]);

  sectcount = get_le_long(buf + 16);
  format_blocky_size(s, sectcount, 512, "sectors", NULL);
  print_line(level + 1, "Volume size %s", s);

  /* TODO: BPB in boot sector, volume label -- information? */
}

/*
 * DOS/Windows boot loaders
 */

void detect_dos_loader(SECTION *section, int level)
{
  int fill;
  unsigned char *buf;

  if (section->flags & FLAG_IN_DISKLABEL)
    return;

  fill = get_buffer(section, 0, 2048, (void **)&buf);
  if (fill < 512)
    return;

  if (find_memory(buf, fill, "NTLDR", 5) >= 0)
    print_line(level, "Windows NTLDR boot loader");
  else if (find_memory(buf, 512, "WINBOOT SYS", 11) >= 0)
    print_line(level, "Windows 95/98/ME boot loader");
  else if (find_memory(buf, 512, "MSDOS   SYS", 11) >= 0)
    print_line(level, "Windows / MS-DOS boot loader");
}

/* EOF */
