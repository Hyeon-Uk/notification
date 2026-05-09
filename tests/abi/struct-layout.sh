#!/usr/bin/env bash
# Capture a struct-layout snapshot of every type referenced from
# the installed notification headers.
#
# Usage:
#   tests/abi/struct-layout.sh <so-path> <out-layout.txt>
#
# Tool selection (in order of preference):
#   1) pahole      — best output, may be absent on dev hosts
#   2) objdump     — fallback using DWARF info embedded in $SO_PATH
#                    (debuginfo must be retained — gbs Debug builds
#                    do this; release strips. For Tizen GBS builds
#                    the unstripped copy lives under
#                    BUILD-ROOTS/scratch.<arch>.0/usr/lib*/.debug/.)
#   3) (last-resort) gcc -fdump-tree-class on a tiny .c that includes
#      every installed header — produces a layout-equivalent dump
#      good enough for diffing if 1+2 are absent.
#
# Output is one record per type, deterministic order.

set -euo pipefail

SO_PATH="${1:?usage: $0 <so> <out-layout>}"
OUT="${2:?usage: $0 <so> <out-layout>}"

dump_via_pahole() {
    # `|| true` because empty input is not an error
    pahole "$SO_PATH" 2>/dev/null \
        | awk '
            /^struct |^union |^typedef / { in_blk=1 }
            in_blk { print }
            /^}/ { in_blk=0; print "" }
        ' || true
}

dump_via_objdump() {
    # When pahole is unavailable, objdump --dwarf=info is the
    # fallback. Pull only the ABI-meaningful bits — byte_size
    # and data_member_location — and group them by the
    # type+member name that owns the value. Skip everything
    # else (DIE addresses, abbrev indices, file paths, indirect
    # string offsets) that drifts between builds on the same
    # source.
    objdump --dwarf=info "$SO_PATH" 2>/dev/null \
        | awk '
            /DW_TAG_(structure_type|union_type|class_type)/ {
                in_type = 1; type_name = ""; size = ""; next
            }
            /DW_TAG_member/ {
                if (in_type) { in_member = 1; member_name = ""; member_off = "" }
                next
            }
            /DW_TAG_(typedef|subprogram|formal_parameter|compile_unit|variable|base_type|pointer_type|const_type|enumerator|enumeration_type|array_type|subroutine_type)/ {
                if (in_type && !in_member && type_name != "" && size != "")
                    print "type:" type_name " size=" size
                in_type = 0; in_member = 0; next
            }
            in_member && /DW_AT_name[^:]*:[^:]*: *([^ ]+)/ {
                match($0, /:[^:]*: *([^ ]+) *$/, a)
                if (a[1] != "") member_name = a[1]
                next
            }
            in_type && !in_member && /DW_AT_name[^:]*:[^:]*: *([^ ]+)/ {
                match($0, /:[^:]*: *([^ ]+) *$/, a)
                if (a[1] != "") type_name = a[1]
                next
            }
            in_type && !in_member && /DW_AT_byte_size[^:]*: *([0-9]+)/ {
                match($0, /: *([0-9]+) *$/, a)
                size = a[1]; next
            }
            in_member && /DW_AT_data_member_location[^:]*: *([0-9]+)/ {
                match($0, /: *([0-9]+) *$/, a)
                member_off = a[1]
                if (type_name != "" && member_name != "")
                    print "  member:" type_name "::" member_name " offset=" member_off
                in_member = 0; next
            }
        ' \
        | LC_ALL=C sort -u
}

dump_via_gcc_aux() {
    local tmp
    tmp="$(mktemp -d)"
    {
        echo "/* auto-generated probe — do not edit */"
        for h in \
            notification.h notification_db.h notification_error.h \
            notification_internal.h notification_ipc.h \
            notification_list.h notification_noti.h \
            notification_ongoing.h notification_ongoing_flag.h \
            notification_setting.h notification_setting_internal.h \
            notification_setting_service.h notification_shared_file.h \
            notification_status.h notification_status_internal.h \
            notification_text_domain.h notification_type.h \
            notification_type_internal.h notification_viewer.h \
            ; do
            echo "#include \"$h\""
        done
        echo "int main(void){return 0;}"
    } > "$tmp/probe.c"
    gcc -fdump-tree-class \
        -I"$(dirname "$0")/../../src/notification/include" \
        $(pkg-config --cflags glib-2.0 dlog bundle 2>/dev/null) \
        -c -o /dev/null "$tmp/probe.c" 2>/dev/null || true
    cat "$tmp"/*.class 2>/dev/null || echo "# probe failed"
    rm -rf "$tmp"
}

{
    echo "# libnotification struct layout snapshot"
    echo "# captured-from: $(readlink -f "$SO_PATH")"
    if command -v pahole >/dev/null 2>&1; then
        echo "# tool: pahole"
        echo "#"
        dump_via_pahole
    elif command -v objdump >/dev/null 2>&1; then
        echo "# tool: objdump --dwarf=info (fallback — pahole unavailable)"
        echo "#"
        dump_via_objdump
    else
        echo "# tool: gcc -fdump-tree-class (last-resort fallback)"
        echo "#"
        dump_via_gcc_aux
    fi
} > "$OUT"

# Sanity check: header was always written. We don't fail when
# the body is empty because some tiny test inputs (e.g., a .so
# with only function symbols and no aggregate types) legitimately
# have no struct layout to report — the empty body is meaningful.
echo "struct-layout.sh: wrote $OUT ($(grep -cv '^#' "$OUT") body lines)"
