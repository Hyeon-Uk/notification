#!/bin/bash
# System CLI Filter Script
# Filters out documentation for CLI tools that don't exist on the target device
# and updates the index.md file accordingly

set -euo pipefail

# Configuration
TOOLS_DIR="/opt/usr/share/tizen-tools/system_cli"
INDEX_FILE="${TOOLS_DIR}/index.md"
TEMP_INDEX="${TOOLS_DIR}/index.md.tmp"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
CYAN='\033[0;36m'
NC='\033[0m'

log()    { echo -e "${CYAN}[FILTER]${NC} $*"; }
ok()     { echo -e "${GREEN}[  OK  ]${NC} $*"; }
warn()   { echo -e "${YELLOW}[ WARN ]${NC} $*"; }
fail()   { echo -e "${RED}[ FAIL ]${NC} $*"; }

# Check if a command exists on the system
command_exists() {
    command -v "$1" &>/dev/null
}

# Get tool name from markdown filename
get_tool_name_from_md() {
    basename "$1" .md
}

# Filter system CLI documentation
filter_system_cli_docs() {
    log "Starting system CLI documentation filtering..."
    
    if [ ! -d "${TOOLS_DIR}" ]; then
        fail "Tools directory not found: ${TOOLS_DIR}"
        return 1
    fi
    
    if [ ! -f "${INDEX_FILE}" ]; then
        fail "Index file not found: ${INDEX_FILE}"
        return 1
    fi
    
    # Count initial files
    local total_files=0
    local removed_files=0
    local remaining_files=0
    
    # Count total markdown files (excluding index.md and this script)
    total_files=$(find "${TOOLS_DIR}" -name "*.md" -not -name "index.md" -not -name "filter_system_cli.sh.md" | wc -l)
    log "Found ${total_files} system CLI documentation files"
    
    # Process each markdown file to check if corresponding CLI tool exists
    log "Checking CLI tool availability..."
    
    local available_tools=()
    local removed_tools=()
    
    # Find all markdown files except index.md
    while IFS= read -r -d '' md_file; do
        local md_filename=$(basename "${md_file}")
        local tool_name=$(get_tool_name_from_md "${md_filename}")
        
        # Skip special files
        if [[ "${md_filename}" == "index.md" ]] || [[ "${md_filename}" == "filter_system_cli.sh.md" ]]; then
            continue
        fi
        
        # Check if the tool exists on the system
        if command_exists "${tool_name}"; then
            available_tools+=("${tool_name}")
            log "  ${tool_name}: ${GREEN}AVAILABLE${NC}"
        else
            # Remove the documentation file for non-existent tool
            if [ -f "${md_file}" ]; then
                rm -f "${md_file}"
                removed_tools+=("${tool_name}")
                ((removed_files++))
                log "  ${tool_name}: ${RED}REMOVED${NC} (tool not found)"
            fi
        fi
    done < <(find "${TOOLS_DIR}" -name "*.md" -not -name "index.md" -not -name "filter_system_cli.sh.md" -print0 2>/dev/null)
    
    remaining_files=$((${#available_tools[@]}))
    log "Filtering complete: ${remaining_files} available, ${removed_files} removed"
    
    # Update index.md file
    update_index_file "${INDEX_FILE}" "${TEMP_INDEX}" available_tools removed_tools
    
    # Print final summary
    print_final_summary available_tools removed_tools
    
    return 0
}

# Update index.md file by removing entries for removed tools
update_index_file() {
    local index_file="$1"
    local temp_index="$2"
    local -n available_tools_ref=$3
    local -n removed_tools_ref=$4
    
    log "Updating index.md..."
    
    if [ ! -f "${index_file}" ]; then
        fail "Index file not found: ${index_file}"
        return 1
    fi
    
    # Create temporary file for processing
    cp "${index_file}" "${temp_index}"
    
    # Remove table rows for removed tools
    # Process the Available System CLI Tools section
    local in_tools_table=false
    local line_num=0
    local lines_to_remove=()
    
    while IFS= read -r line; do
        ((line_num++))
        
        # Detect start of tools table
        if [[ "${line}" == "| Tool Name | Description | Category | Documentation |" ]]; then
            in_tools_table=true
            continue
        fi
        
        # Process table rows
        if [ "${in_tools_table}" = true ] && [[ "${line}" == "| "*" | "*" | "*" | "*".md)"* ]]; then
            # Extract tool name from the table row
            local table_tool_name=$(echo "${line}" | cut -d'|' -f2 | xargs)
            
            # Check if this tool was removed
            local tool_removed=false
            for removed_tool in "${removed_tools_ref[@]}"; do
                if [[ "${removed_tool}" == "${table_tool_name}" ]]; then
                    tool_removed=true
                    lines_to_remove+=("${line_num}")
                    break
                fi
            done
        elif [ "${in_tools_table}" = true ] && [[ "${line}" != "| "* ]]; then
            # End of table
            in_tools_table=false
        fi
    done < "${temp_index}"
    
    # Remove lines in reverse order to maintain line numbers
    if [ ${#lines_to_remove[@]} -gt 0 ]; then
        log "Removing ${#lines_to_remove[@]} entries from index.md table"
        for ((i=${#lines_to_remove[@]}-1; i>=0; i--)); do
            local line_to_remove=${lines_to_remove[i]}
            sed -i "${line_to_remove}d" "${temp_index}"
        done
    fi
    
    # Update quick reference sections
    log "Updating quick reference sections..."
    update_quick_reference_sections "${temp_index}" available_tools_ref removed_tools_ref
    
    # Replace original index file
    mv "${temp_index}" "${index_file}"
    ok "index.md updated successfully"
}

# Update quick reference sections by filtering available tools
update_quick_reference_sections() {
    local temp_index="$1"
    local -n available_tools_ref=$2
    local -n removed_tools_ref=$3
    
    # Create arrays for each category
    local audio_tools=()
    local network_tools=()
    local storage_tools=()
    local system_tools=()
    local security_tools=()
    local debug_tools=()
    local input_tools=()
    local text_tools=()
    local ui_tools=()
    local media_tools=()
    local dev_tools=()
    local compression_tools=()
    
    # Categorize available tools (this would need to be enhanced with actual category data)
    # For now, we'll use a simplified approach by parsing the existing index
    
    # Read the current index to extract categories and tools
    local current_category=""
    while IFS= read -r line; do
        # Detect category headers
        if [[ "${line}" == "### "* ]]; then
            current_category=$(echo "${line}" | sed 's/### //')
        elif [[ "${line}" == "- "* ]]; then
            # Extract tools from list
            local tools_list=$(echo "${line}" | sed 's/- //' | tr -d '`')
            IFS=', ' read -ra TOOLS <<< "${tools_list}"
            
            # Filter tools based on availability
            local filtered_tools=()
            for tool in "${TOOLS[@]}"; do
                tool=$(echo "${tool}" | xargs)  # trim whitespace
                local is_available=false
                for available_tool in "${available_tools_ref[@]}"; do
                    if [[ "${available_tool}" == "${tool}" ]]; then
                        is_available=true
                        break
                    fi
                done
                
                if [ "${is_available}" = true ]; then
                    filtered_tools+=("${tool}")
                fi
            done
            
            # Update the line with filtered tools if any remain
            if [ ${#filtered_tools[@]} -gt 0 ]; then
                local tool_list_str=$(IFS=', '; echo "${filtered_tools[*]}")
                # We'll update this during the actual file processing
            fi
        fi
    done < "${temp_index}"
    
    # For now, we'll do a simpler approach: regenerate the quick reference sections
    regenerate_quick_reference_sections "${temp_index}" available_tools_ref
}

# Regenerate quick reference sections based on available tools
regenerate_quick_reference_sections() {
    local temp_index="$1"
    local -n available_tools_ref=$2
    
    # Find the section after "Quick Reference by Category" and replace it
    local temp_file="${temp_index}.quickref"
    local in_quick_ref=false
    local quick_ref_done=false
    
    > "${temp_file}"  # Clear temp file
    
    while IFS= read -r line; do
        if [ "${quick_ref_done}" = false ] && [[ "${line}" == "## Quick Reference by Category" ]]; then
            in_quick_ref=true
            echo "${line}" >> "${temp_file}"
            # Add new quick reference sections
            add_new_quick_reference_sections "${temp_file}" available_tools_ref
            quick_ref_done=true
            continue
        elif [ "${in_quick_ref}" = true ] && [[ "${line}" == "## "* ]] && [[ "${line}" != "## Quick Reference by Category" ]]; then
            # End of quick reference section
            in_quick_ref=false
        fi
        
        if [ "${in_quick_ref}" = false ] || [ "${quick_ref_done}" = true ]; then
            echo "${line}" >> "${temp_file}"
        fi
    done < "${temp_index}"
    
    mv "${temp_file}" "${temp_index}"
}

# Add new quick reference sections based on actual tool availability
add_new_quick_reference_sections() {
    local temp_file="$1"
    local -n available_tools_ref=$2
    
    # Define tool categories (simplified version)
    local audio_category_tools=(
        "aconnect" "alsabat" "alsactl" "alsaloop" "alsaucm" "altonegen" "amidi" "amixer" 
        "aplay" "aplaymidi" "arecord" "arecordmidi" "aseqdump" "aseqnet" "aseqsend" "axfer" 
        "iecset" "nhlt-dmic-info" "openal-info"
    )
    
    local network_category_tools=(
        "arp" "arpaname" "bridge" "ddns-confgen" "delv" "dig" "genl" "host" "ifcfg" "ifconfig" 
        "ip" "ip6tables" "ip6tables-restore" "ip6tables-save" "ipmaddr" "ipsec" "iptables" 
        "iptables-restore" "iptables-save" "iptables-xml" "iptunnel" "mdig" "mii-tool" "nameif" 
        "named" "named-checkconf" "named-checkzone" "named-journalprint" "named-rrchecker" 
        "nbd-client" "net-cls-release" "netstat" "newfs_msdos" "nsec3hash" "nslookup" "nsupdate" "openvpn"
    )
    
    local storage_category_tools=(
        "blkdeactivate" "btrfs" "btrfs-convert" "btrfs-find-root" "btrfs-map-logical"
        "btrfs-select-super" "btrfsck" "btrfstune" "defrag.f2fs" "dmsetup" "dmstats" "dump.f2fs"
        "f2fs_io" "f2fscrypt" "f2fslabel" "fibmap.f2fs" "fsck.btrfs" "fsck.f2fs" "mkfs.btrfs" 
        "mkfs.f2fs" "parse-dynparts"
    )
    
    local system_category_tools=(
        "actd" "actdctl" "action-db-recovery" "action-tool" "action-upgrade" "alarmmgr_tool" 
        "app_launcher" "appgroup_info" "appid2pid" "appmgr_tool" "appsvc-db-recovery" "at_spi2_tool" 
        "aul_test" "aul_window" "aulctl" "boot_status" "buxton2ctl" "cgroup_remount" "compmgr_tool" 
        "component-db-recovery" "crash-json2info" "cyad" "cynara" "cynara-db-chsgen" "cynara-db-migration" 
        "dbuspolicy-checker" "depmod" "devicectl" "dpm-admin-cli" "dpm-storage-builder" "dump_systemstate" 
        "glibc_post_upgrade" "hal-compatibility-checker" "isu" "isu_pkgs_maker.py" "isu_pkgs_maker_py2.py" 
        "isu-sandbox" "isf" "keymap_update.sh" "kmod" "launch_app" "launch_debug" "ld.so" "ldconfig" 
        "ldd" "locale" "localedef" "lshal" "lsmod" "make-update-control-config.sh" 
        "mediacontroller" "mediadb-update" "memps" "minicoredumper" "mobileap-agent-cmd.sh" 
        "model-config-cli" "modinfo" "modprobe" "notification_init" "nsjail" "nsjail.sh" "p2p_supp.sh" 
        "pam_tally" "pam_tally2"
    )
    
    local security_category_tools=(
        "capsh" "chsmack" "ckm_tool" "cryptsetup" "cyad" "cynara" "cynara-db-chsgen" "cynara-db-migration" 
        "getcap" "getpcaps" "nsjail" "nsjail.sh" "pam_tally" "pam_tally2"
    )
    
    local debug_category_tools=(
        "atrace" "atrace-1.1" "atrace-bootup.sh" "atrace-helper" "atrace-helper-1.0" "crash-json2info" 
        "dalireslog.sh" "dlog_redirect_stdout" "dlogctl" "dlogsend" "dlogutil" "dump_systemstate" 
        "launch_debug" "memps" "minicoredumper"
    )
    
    local input_category_tools=(
        "anthy-dic-tool" "anthy-morphological-analyzer" "hangul" "input_keyevent" "keymap_update.sh"
    )
    
    local text_category_tools=(
        "awk" "gawk" "gawk-3.1.5" "iconv" "iconvconfig" "igawk" "json-glib-format" "json-glib-validate"
    )
    
    local ui_category_tools=(
        "enlightenment_info" "enlightenment_input_key" "enlightenment_mon.sh"
    )
    
    local media_category_tools=(
        "ges-launch-1.0" "gst-inspect-1.0" "gst-launch-1.0" "gst-stats-1.0" "gst-typefind-1.0" 
        "mediacontroller" "mediadb-update"
    )
    
    local dev_category_tools=("dotnettool")
    local compression_category_tools=("funzip")
    
    # Generate sections for each category
    generate_category_section "${temp_file}" "Audio" audio_category_tools available_tools_ref
    generate_category_section "${temp_file}" "Network" network_category_tools available_tools_ref
    generate_category_section "${temp_file}" "Storage" storage_category_tools available_tools_ref
    generate_category_section "${temp_file}" "System" system_category_tools available_tools_ref
    generate_category_section "${temp_file}" "Security" security_category_tools available_tools_ref
    generate_category_section "${temp_file}" "Debug" debug_category_tools available_tools_ref
    generate_category_section "${temp_file}" "Input" input_category_tools available_tools_ref
    generate_category_section "${temp_file}" "Text Processing" text_category_tools available_tools_ref
    generate_category_section "${temp_file}" "UI" ui_category_tools available_tools_ref
    generate_category_section "${temp_file}" "Media" media_category_tools available_tools_ref
    generate_category_section "${temp_file}" "Development" dev_category_tools available_tools_ref
    generate_category_section "${temp_file}" "Compression" compression_category_tools available_tools_ref
    
    # Add the final note
    cat >> "${temp_file}" << 'EOF'

For detailed usage of each tool, refer to the individual tool documentation linked in the table above.
EOF
}

# Generate category section with available tools only
generate_category_section() {
    local temp_file="$1"
    local category_name="$2"
    local -n category_tools_ref=$3
    local -n available_tools_ref=$4
    
    # Filter available tools in this category
    local available_in_category=()
    for tool in "${category_tools_ref[@]}"; do
        for available_tool in "${available_tools_ref[@]}"; do
            if [[ "${available_tool}" == "${tool}" ]]; then
                available_in_category+=("${tool}")
                break
            fi
        done
    done
    
    # Only add section if there are available tools
    if [ ${#available_in_category[@]} -gt 0 ]; then
        echo "" >> "${temp_file}"
        echo "### ${category_name}" >> "${temp_file}"
        local tool_list=$(IFS=', '; echo "${available_in_category[*]}")
        echo "- \`${tool_list}\`" >> "${temp_file}"
    fi
}

# Print final summary of available tools
print_final_summary() {
    local -n available_tools_ref=$1
    local -n removed_tools_ref=$2
    
    echo ""
    echo "=========================================="
    echo "  System CLI Tools - Final Status"
    echo "=========================================="
    echo ""
    
    if [ ${#available_tools_ref[@]} -gt 0 ]; then
        echo "Available System CLI Tools (${#available_tools_ref[@]} total):"
        for tool in "${available_tools_ref[@]}"; do
            echo "  ✓ ${tool}"
        done
    else
        echo "No system CLI tools are available on this device."
    fi
    
    if [ ${#removed_tools_ref[@]} -gt 0 ]; then
        echo ""
        echo "Removed Documentation for Unavailable Tools (${#removed_tools_ref[@]} total):"
        for tool in "${removed_tools_ref[@]}"; do
            echo "  ✗ ${tool}"
        done
    fi
    
    echo ""
    echo "Documentation filtering completed successfully."
    echo ""
}

# Main execution
main() {
    log "System CLI Documentation Filter Script"
    log "====================================="
    
    filter_system_cli_docs
    
    ok "System CLI documentation filtering completed"
}

main "$@"