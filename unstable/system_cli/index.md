# System CLI Tools

This directory contains documentation for Tizen system command-line interface tools, organized by category.

## Overview

This directory is organized into subdirectories, each representing a specific category of system CLI tools. Each subdirectory contains individual markdown documentation files for specific commands.

**Total Tools: 182** across 26 categories

## Directory Structure

| Category | Description | Tools Count |
|----------|-------------|-------------|
| [accessibility/](accessibility/) | Accessibility-related CLI tools | 1 |
| [action_framework/](action_framework/) | Action Framework, alarms, preferences, themes | 9 |
| [application/](application/) | Application lifecycle and management | 10 |
| [audio/](audio/) | ALSA and MIDI audio tools | 17 |
| [compression/](compression/) | File compression and archive tools | 9 |
| [configuration/](configuration/) | System configuration and library management | 7 |
| [database/](database/) | Database management tools | 2 |
| [dbus/](dbus/) | D-Bus inter-process communication tools | 1 |
| [device/](device/) | Device management and hardware abstraction | 6 |
| [device_mapper/](device_mapper/) | Device mapper administration tools | 3 |
| [display/](display/) | Display and window management tools | 1 |
| [dotnet/](dotnet/) | .NET runtime tools | 1 |
| [filesystem/](filesystem/) | Filesystem management (btrfs, f2fs, etc.) | 22 |
| [gstreamer/](gstreamer/) | GStreamer multimedia framework tools | 5 |
| [json/](json/) | JSON processing tools | 2 |
| [kernel_module/](kernel_module/) | Linux kernel module management tools | 7 |
| [locale/](locale/) | Locale, i18n, character encoding tools | 10 |
| [logging/](logging/) | System logging and DLog tools | 4 |
| [network/](network/) | Network configuration and management | 11 |
| [package_management/](package_management/) | Tizen package management tools | 12 |
| [resource/](resource/) | Resource management tools | 2 |
| [security/](security/) | Security, SMACK, and encryption tools | 15 |
| [service/](service/) | System service and daemon management tools | 4 |
| [system/](system/) | System monitoring and control tools | 17 |
| [text_processing/](text_processing/) | Text processing and transformation tools | 8 |
| [tizen_platform/](tizen_platform/) | Tizen platform-specific utilities | 6 |

## Category Details

### accessibility
Accessibility-related CLI tools for Tizen.
- `at_spi2_tool` - AT-SPI2 accessibility tool

See [accessibility/](accessibility/) directory.

### action_framework
Action Framework related tools for managing Tizen actions, alarms, preferences, and themes.
- `actdctl` - Action daemon control
- `action-db-recovery` - Action database recovery
- `action-tool` - Action management tool
- `action-upgrade` - Action upgrade utility
- `alarmmgr_tool` - Alarm manager tool
- `component-db-recovery` - Component database recovery
- `preference_tool` - Preference management tool
- `theme-cmd` - Theme command tool
- `tizen_id` - Tizen ID utility

See [action_framework/](action_framework/) directory.

### application
Application lifecycle and management tools.
- `app_launcher` - Application launcher
- `appgroup_info` - Application group information
- `appid2pid` - Application ID to PID converter
- `appmgr_tool` - Application manager tool
- `appsvc-db-recovery` - App service database recovery
- `aul_test` - AUL (Application Utility Library) test tool
- `aul_window` - AUL window utility
- `aulctl` - AUL control tool
- `launch_app` - Application launcher
- `launch_debug` - Debug application launcher

See [application/](application/) directory.

### audio
ALSA (Advanced Linux Sound Architecture) and MIDI audio tools.
- `aconnect` - ALSA sequencer connection manager
- `alsabat` - ALSA BAT (Basic Audio Tester)
- `alsactl` - ALSA control tool
- `alsaloop` - ALSA loopback tool
- `alsaucm` - ALSA Use Case Manager
- `amidi` - ALSA MIDI tool
- `amixer` - ALSA mixer control
- `aplay` - ALSA playback tool
- `aplaymidi` - ALSA MIDI playback
- `arecord` - ALSA recording tool
- `arecordmidi` - ALSA MIDI recording
- `aseqdump` - ALSA sequencer dump
- `aseqnet` - ALSA sequencer network
- `aseqsend` - ALSA sequencer send
- `axfer` - ALSA transfer tool
- `iecset` - IEC958 settings
- `speaker-test` - Speaker test utility

See [audio/](audio/) directory.

### compression
File compression and archive tools.
- `funzip` - Filter for unzip
- `unzip` - Extract ZIP archives
- `unzipsfx` - Self-extracting ZIP creator
- `zip` - Create ZIP archives
- `zipcloak` - ZIP encryption tool
- `zipgrep` - Search in ZIP files
- `zipinfo` - ZIP file information
- `zipnote` - ZIP note editor
- `zipsplit` - Split ZIP archives

See [compression/](compression/) directory.

### configuration
System configuration and library management tools.
- `cgroup_remount` - Cgroup remount utility
- `ldconfig` - Dynamic linker configuration
- `ldd` - Library dependency display
- `make-update-control-config.sh` - Update control config script
- `model-config-cli` - Model configuration CLI
- `update-alternatives` - Alternatives management
- `vconftool` - Value configuration tool

See [configuration/](configuration/) directory.

### database
Database management tools.
- `buxton2ctl` - Buxton2 database control
- `sqlite3` - SQLite database CLI

See [database/](database/) directory.

### dbus
D-Bus inter-process communication tools.
- `dbuspolicy-checker` - D-Bus policy checker

See [dbus/](dbus/) directory.

### device
Device management and hardware abstraction tools.
- `devicectl` - Device control tool
- `hal-compatibility-checker` - HAL compatibility checker
- `lshal` - HAL listing tool
- `nhlt-dmic-info` - NHLT DMIC information
- `parse-dynparts` - Parse dynamic parts
- `wayland-tbm-monitor` - Wayland TBM monitor

See [device/](device/) directory.

### device_mapper
Device mapper administration tools.
- `blkdeactivate` - Block device deactivation
- `dmsetup` - Device mapper setup
- `dmstats` - Device mapper statistics

See [device_mapper/](device_mapper/) directory.

### display
Display and window management tools.
- `winfo` - Window information tool

See [display/](display/) directory.

### dotnet
.NET runtime tools.
- `dotnettool` - .NET CLI tool

See [dotnet/](dotnet/) directory.

### filesystem
Filesystem management and maintenance tools (btrfs, f2fs, etc.).
- `btrfs` - Btrfs filesystem tool
- `btrfs-convert` - Btrfs conversion tool
- `btrfs-find-root` - Find Btrfs root
- `btrfs-map-logical` - Map logical Btrfs addresses
- `btrfs-select-super` - Select Btrfs superblock
- `btrfsck` - Btrfs filesystem check
- `btrfstune` - Btrfs tuning tool
- `defrag.f2fs` - F2FS defragmentation
- `dump.f2fs` - F2FS dump utility
- `f2fs_io` - F2FS I/O tool
- `f2fscrypt` - F2FS encryption
- `f2fslabel` - F2FS label tool
- `fibmap.f2fs` - F2FS fibmap
- `fsck.btrfs` - Btrfs filesystem check
- `fsck.f2fs` - F2FS filesystem check
- `mkfs.btrfs` - Create Btrfs filesystem
- `mkfs.f2fs` - Create F2FS filesystem
- `newfs_msdos` - Create MSDOS filesystem
- `parse.f2fs` - F2FS parser
- `resize.f2fs` - Resize F2FS filesystem
- `sload.f2fs` - F2FS load tool
- `tar` - Tape archive tool

See [filesystem/](filesystem/) directory.

### gstreamer
GStreamer multimedia framework tools.
- `ges-launch-1.0` - GES (GStreamer Editing Services) launcher
- `gst-inspect-1.0` - GStreamer plugin inspector
- `gst-launch-1.0` - GStreamer pipeline launcher
- `gst-stats-1.0` - GStreamer statistics
- `gst-typefind-1.0` - GStreamer type finder

See [gstreamer/](gstreamer/) directory.

### json
JSON processing tools.
- `json-glib-format` - JSON-GLib formatter
- `json-glib-validate` - JSON-GLib validator

See [json/](json/) directory.

### kernel_module
Linux kernel module management tools.
- `depmod` - Module dependency generator
- `insmod` - Insert kernel module
- `kmod` - Kernel module management
- `lsmod` - List kernel modules
- `modinfo` - Module information
- `modprobe` - Module probe utility
- `rmmod` - Remove kernel module

See [kernel_module/](kernel_module/) directory.

### locale
Locale, internationalization, and character encoding tools.
- `gencat` - Generate catalog
- `getconf` - Get configuration
- `getent` - Get entry from database
- `iconv` - Character encoding conversion
- `iconvconfig` - Iconv configuration
- `locale` - Locale information
- `localedef` - Locale definition compiler
- `tzselect` - Timezone selection
- `zdump` - Timezone dumper
- `zic` - Timezone compiler

See [locale/](locale/) directory.

### logging
System logging and DLog tools.
- `dlog_redirect_stdout` - DLog stdout redirect
- `dlogctl` - DLog control tool
- `dlogsend` - DLog send utility
- `dlogutil` - DLog utility tool

See [logging/](logging/) directory.

### network
Network configuration and management tools.
- `connmanctl` - Connection manager control
- `ip6tables` - IPv6 packet filtering
- `ip6tables-restore` - Restore IPv6 tables
- `ip6tables-save` - Save IPv6 tables
- `iptables` - IPv4 packet filtering
- `iptables-restore` - Restore IPv4 tables
- `iptables-save` - Save IPv4 tables
- `nbd-client` - Network block device client
- `rndis.sh` - RNDIS configuration script
- `wifi_connect_tool` - WiFi connection tool
- `xtables-multi` - xtables multi-call binary

See [network/](network/) directory.

### package_management
Tizen package management tools.
- `mediadb-update` - Media database update
- `pkg_initdb` - Package database initialization
- `pkg_recovery` - Package recovery
- `pkg-db-creator` - Package database creator
- `pkg-db-recovery` - Package database recovery
- `pkgcmd` - Package command tool
- `pkgdir-tool` - Package directory tool
- `pkginfo` - Package information
- `tpk-backend` - TPK backend
- `wgt-backend` - Widget backend
- `widget_recovery` - Widget recovery
- `widget-mgr` - Widget manager

See [package_management/](package_management/) directory.

### resource
Resource management tools.
- `res-copy` - Resource copy tool
- `rsc-slice` - Resource slice tool

See [resource/](resource/) directory.

### security
Security, SMACK, and encryption tools.
- `chsmack` - Change SMACK attributes
- `ckm_tool` - CKM tool
- `cryptsetup` - Disk encryption setup
- `cyad` - CYA (Credential Access) daemon
- `isu` - ISU utility
- `isu-sandbox` - ISU sandbox
- `nsjail` - Namespace jail
- `nsjail.sh` - NSJail script
- `scepclient` - SCEP client
- `security-manager-cmd` - Security manager command
- `smackaccess` - SMACK access control
- `smackcipso` - SMACK CIPSO
- `smackctl` - SMACK control
- `smackload` - SMACK load
- `veritysetup` - Verity setup

See [security/](security/) directory.

### service
System service and daemon management tools.
- `compmgr_tool` - Component manager tool
- `crash-json2info` - Crash JSON to info converter
- `minicoredumper` - Mini core dumper
- `resource-monitor` - Resource monitor

See [service/](service/) directory.

### system
System monitoring and control tools.
- `dump_systemstate` - Dump system state
- `free` - Memory usage display
- `memps` - Memory process statistics
- `pgrep` - Process grep
- `pidof` - Find PID of process
- `pkill` - Process kill
- `pmap` - Process memory map
- `ps` - Process status
- `pwdx` - Print working directory
- `reboot-haltest` - Reboot for HAL test
- `reboot-normal` - Normal reboot
- `slabtop` - SLAB top display
- `sysctl` - System control
- `tload` - Terminal load display
- `uptime` - System uptime
- `vmstat` - Virtual memory statistics
- `w` - Logged in users

See [system/](system/) directory.

### text_processing
Text processing and transformation tools.
- `awk` - AWK text processing
- `gawk` - GNU AWK
- `gawk-3.1.5` - GNU AWK 3.1.5
- `igawk` - Interactive GAWK
- `pgawk` - Profiled GAWK
- `pgawk-3.1.5` - Profiled GAWK 3.1.5
- `sed` - Stream editor
- `xsltproc` - XSLT processor

See [text_processing/](text_processing/) directory.

### tizen_platform
Tizen platform-specific utilities.
- `gum-utils` - GUM utilities
- `system-info-tool` - System information tool
- `tlm-client` - TLM client
- `tlm-launcher` - TLM launcher
- `tzplatform-get` - Tizen platform get
- `unified-backend` - Unified backend

See [tizen_platform/](tizen_platform/) directory.

## Usage

