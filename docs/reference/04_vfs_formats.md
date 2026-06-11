# Reference: Supported VFS Formats

The Sandbox Engine mounts applications using PhysicsFS. An application can be a raw folder or a compressed archive.

The `sandbox` launcher's `--mount` argument accepts the following formats:

1. **Directories**: Standard OS folders.
2. **ZIP** (`.zip`): Standard ZIP archives.
3. **7zip** (`.7z`): 7-Zip compression archives.
4. **GRP** (`.grp`): Build Engine archives (Duke Nukem 3D).
5. **WAD** (`.wad`): Doom engine archives.
6. **HOG** (`.hog`): Descent I/II archives.
7. **MVL** (`.mvl`): Descent II multiplayer archives.
8. **QPAK** (`.pak`): Quake I/II archives.
9. **SLB** (`.slb`): I-War / Independence War archives.
10. **VDF** (`.vdf`): Gothic I/II archives.
11. **ISO9660** (`.iso`): CD-ROM images.

*Note: For production releases, a `.zip` file is the highly recommended standard.*
