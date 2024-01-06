# ZFS keyfinder

Did you corrupt your (non-ZFS) filesystem and in doing so lose the `keyformat=raw`
32-byte keyfile to unlock your encrypted ZFS datasets?

Because these keyfiles don't have a well-known header on them, regular file recovery
tools struggle to locate them.

ZFS keyfinder looks for 32-byte randomly-generated keyfiles in a raw volume, and 
assumes that they appear at 1024-byte aligned positions in the volume which are 
then padded out with null bytes to 1024 bytes. That's how these keyfiles are stored 
on FAT or ext3 filesystems.

## Building

Just run "make" and your C++ compiler will be used to build zfs-keyfinder.

## Usage

Pass zfs-keyfinder the name of a raw device or disk image file to search, or
pipe the volume into its standard input. It will print the offset it found a candidate
match at and the key in hex format:

```bash
$ ./zfs-keyfinder /dev/sda
0x000080880400 31C40524A7082C94A09823F36B0C8E061BB59BF064D9E5E326187338E4F64D61
0x000080980400 E93A854AE8C10B2526E3B4FBE2F8B95215995F44CC9C73FC7580020A9572E611
Found 2 candidates
````

You may need to use "sudo" if you're having zfs-keyfinder read a raw disk device.

You can use the utility "pv" like this to get a progress bar:

```bash
$ pv /dev/sda | ./zfs-keyfinder
```

You can have ZFS automatically try out the candidate keys that are found using Bash
like so, just replace `/dev/sda` with the volume to search for keys, and `tank/my-dataset` 
with the name of the dataset you want to try loading keys for:

```bash
searchvolume=/dev/sda
dataset=tank/my-dataset

pv "${searchvolume}" | ./zfs-keyfinder | unbuffer -p cut -d' ' -f2 |
while read -r candidate; do
    zfs load-key -n -L file://<(echo -n "${candidate}" | xxd -r -p) "${dataset}" \
      && echo "Found working key: ${candidate}" \
      && break
done
```

```
40.0MiB 0:00:00 [ 276MiB/s] [====================================================>] 100%
Key load error: Incorrect key provided for 'tank/my-dataset'.
0 / 1 key(s) successfully verified
1 / 1 key(s) successfully verified
Found working key: E93A854AE8C10B2526E3B4FBE2F8B95215995F44CC9C73FC7580020A9572E611
Found 2 candidates
```

If you don't have the utility "unbuffer", remove "unbuffer -p" from the command. With this
unavailable, potential keys will not be tested until pretty much the whole volume has been 
searched.