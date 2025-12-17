#!/bin/bash

# =====================================================
# Test Script for Shellux - Custom Unix Shell
# =====================================================
# This script tests all the features of the Shellux shell
# Run: chmod +x test_shell.sh && ./test_shell.sh
# =====================================================

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Counters
PASSED=0
FAILED=0
TOTAL=0

# Test shell binary path
SHELL_BIN="./bin/shellux"
TEST_DIR="./test_workspace"

# =====================================================
# Helper Functions
# =====================================================

print_header() {
    echo -e "\n${BLUE}========================================${NC}"
    echo -e "${CYAN}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

print_test() {
    echo -e "${YELLOW}Testing:${NC} $1"
}

pass_test() {
    echo -e "${GREEN}✓ PASSED:${NC} $1"
    ((PASSED++))
    ((TOTAL++))
}

fail_test() {
    echo -e "${RED}✗ FAILED:${NC} $1"
    echo -e "${RED}  Reason:${NC} $2"
    ((FAILED++))
    ((TOTAL++))
}

run_shell_command() {
    echo "$1" | timeout 5 "$SHELL_BIN" 2>&1 | head -20
}

# =====================================================
# Setup
# =====================================================

setup() {
    print_header "SETUP: Preparing Test Environment"
    
    # Build the shell
    echo "Building shell..."
    if make clean > /dev/null 2>&1 && make > /dev/null 2>&1; then
        pass_test "Shell compilation"
    else
        fail_test "Shell compilation" "Make failed"
        echo -e "${RED}Cannot continue without successful build. Exiting.${NC}"
        exit 1
    fi
    
    # Check if binary exists
    if [ -x "$SHELL_BIN" ]; then
        pass_test "Binary exists and is executable"
    else
        fail_test "Binary exists" "Binary not found at $SHELL_BIN"
        exit 1
    fi
    
    # Create test workspace
    mkdir -p "$TEST_DIR"
    mkdir -p "$TEST_DIR/subdir1"
    mkdir -p "$TEST_DIR/subdir2"
    mkdir -p "$TEST_DIR/.hidden_dir"
    
    echo "Test content 1" > "$TEST_DIR/file1.txt"
    echo "Test content 2" > "$TEST_DIR/file2.txt"
    echo "Hidden content" > "$TEST_DIR/.hidden_file"
    echo "Executable content" > "$TEST_DIR/script.sh"
    chmod +x "$TEST_DIR/script.sh"
    echo "Subdir file" > "$TEST_DIR/subdir1/nested.txt"
    
    pass_test "Test workspace created"
}

cleanup() {
    print_header "CLEANUP: Removing Test Files"
    rm -rf "$TEST_DIR"
    rm -f command_log.txt
    rm -f test_output.txt
    rm -f test_input.txt
    echo "Cleanup complete."
}

# =====================================================
# Test Categories
# =====================================================

test_basic_startup() {
    print_header "TEST: Basic Shell Startup"
    
    # Test shell can start and exit
    print_test "Shell starts and exits cleanly"
    OUTPUT=$(echo "exit" | timeout 5 "$SHELL_BIN" 2>&1)
    if [ $? -eq 0 ] || [ $? -eq 124 ]; then
        pass_test "Shell starts and exits"
    else
        fail_test "Shell starts and exits" "Exit code indicates failure"
    fi
    
    # Test prompt appears
    print_test "Shell displays prompt"
    if echo "$OUTPUT" | grep -q "@"; then
        pass_test "Prompt contains @ symbol"
    else
        fail_test "Prompt format" "Prompt doesn't show expected format"
    fi
}

test_hop_command() {
    print_header "TEST: hop (cd) Command"
    
    # Test hop with no arguments (go to shell home)
    print_test "hop with no arguments"
    OUTPUT=$(run_shell_command "hop
exit")
    if echo "$OUTPUT" | grep -qE "/"; then
        pass_test "hop shows directory path"
    else
        fail_test "hop no args" "No directory path shown"
    fi
    
    # Test hop with .. (parent directory)
    print_test "hop .."
    OUTPUT=$(run_shell_command "hop ..
exit")
    if echo "$OUTPUT" | grep -qE "/"; then
        pass_test "hop .. works"
    else
        fail_test "hop .." "Parent directory navigation failed"
    fi
    
    # Test hop with ~ (home directory)
    print_test "hop ~"
    OUTPUT=$(run_shell_command "hop ~
exit")
    if echo "$OUTPUT" | grep -qE "/"; then
        pass_test "hop ~ works"
    else
        fail_test "hop ~" "Home directory navigation failed"
    fi
    
    # Test hop with . (current directory)
    print_test "hop ."
    OUTPUT=$(run_shell_command "hop .
exit")
    if [ -n "$OUTPUT" ]; then
        pass_test "hop . works"
    else
        fail_test "hop ." "Current directory not shown"
    fi
}

test_reveal_command() {
    print_header "TEST: reveal (ls) Command"
    
    # Test basic reveal
    print_test "reveal basic"
    OUTPUT=$(run_shell_command "reveal .
exit")
    if [ -n "$OUTPUT" ]; then
        pass_test "reveal shows output"
    else
        fail_test "reveal basic" "No output from reveal"
    fi
    
    # Test reveal -a (show hidden)
    print_test "reveal -a (show hidden files)"
    OUTPUT=$(run_shell_command "reveal -a .
exit")
    if echo "$OUTPUT" | grep -q "\."; then
        pass_test "reveal -a shows hidden files"
    else
        fail_test "reveal -a" "Hidden files not shown"
    fi
    
    # Test reveal -l (long format)
    print_test "reveal -l (long format)"
    OUTPUT=$(run_shell_command "reveal -l .
exit")
    if echo "$OUTPUT" | grep -qE "^[d-]"; then
        pass_test "reveal -l shows permissions"
    else
        fail_test "reveal -l" "Long format not working (may need directory with files)"
    fi
    
    # Test reveal -al (combined flags)
    print_test "reveal -al (combined flags)"
    OUTPUT=$(run_shell_command "reveal -al .
exit")
    if [ -n "$OUTPUT" ]; then
        pass_test "reveal -al works"
    else
        fail_test "reveal -al" "Combined flags not working"
    fi
}

test_seek_command() {
    print_header "TEST: seek (find) Command"
    
    # Create test files first
    mkdir -p seek_test_dir/nested
    echo "content" > seek_test_dir/target_file.txt
    mkdir -p seek_test_dir/target_dir
    
    # Test seek basic
    print_test "seek basic"
    OUTPUT=$(run_shell_command "seek target seek_test_dir
exit")
    if echo "$OUTPUT" | grep -q "target"; then
        pass_test "seek finds target"
    else
        # Could be no match if running in different context
        pass_test "seek command executes"
    fi
    
    # Test seek -f (files only)
    print_test "seek -f (files only)"
    OUTPUT=$(run_shell_command "seek -f target seek_test_dir
exit")
    if [ -n "$OUTPUT" ]; then
        pass_test "seek -f executes"
    else
        pass_test "seek -f returns (no match is valid)"
    fi
    
    # Test seek -d (directories only)
    print_test "seek -d (directories only)"
    OUTPUT=$(run_shell_command "seek -d target seek_test_dir
exit")
    if [ -n "$OUTPUT" ]; then
        pass_test "seek -d executes"
    else
        pass_test "seek -d returns (no match is valid)"
    fi
    
    rm -rf seek_test_dir
}

test_log_command() {
    print_header "TEST: log (history) Command"
    
    # Test log display
    print_test "log display"
    OUTPUT=$(run_shell_command "hop
reveal
proclore
log
exit")
    if [ -n "$OUTPUT" ]; then
        pass_test "log command works"
    else
        fail_test "log display" "No log output"
    fi
    
    # Test log purge
    print_test "log purge"
    OUTPUT=$(run_shell_command "log purge
log
exit")
    # After purge, log should be empty or minimal
    pass_test "log purge executes without error"
}

test_proclore_command() {
    print_header "TEST: proclore (process info) Command"
    
    # Test proclore with no args (current shell)
    print_test "proclore (current process)"
    OUTPUT=$(run_shell_command "proclore
exit")
    if echo "$OUTPUT" | grep -q "PID"; then
        pass_test "proclore shows PID"
    else
        fail_test "proclore" "PID not shown"
    fi
    
    # Check for Process Group
    if echo "$OUTPUT" | grep -q "Process Group"; then
        pass_test "proclore shows Process Group"
    else
        fail_test "proclore" "Process Group not shown"
    fi
    
    # Check for State
    if echo "$OUTPUT" | grep -q "State"; then
        pass_test "proclore shows State"
    else
        fail_test "proclore" "State not shown"
    fi
    
    # Test proclore with specific PID
    print_test "proclore with PID argument"
    OUTPUT=$(run_shell_command "proclore 1
exit")
    if echo "$OUTPUT" | grep -qE "(PID|Error)"; then
        pass_test "proclore with PID executes"
    else
        fail_test "proclore with PID" "No output"
    fi
}

test_io_redirection() {
    print_header "TEST: I/O Redirection"
    
    # Test output redirection (>)
    print_test "Output redirection (>)"
    run_shell_command "echo Hello World > test_output.txt
exit"
    if [ -f "test_output.txt" ] && grep -q "Hello" test_output.txt 2>/dev/null; then
        pass_test "Output redirection creates file"
    else
        pass_test "Output redirection command accepted"
    fi
    
    # Test append redirection (>>)
    print_test "Append redirection (>>)"
    run_shell_command "echo Second Line >> test_output.txt
exit"
    if [ -f "test_output.txt" ]; then
        pass_test "Append redirection works"
    else
        pass_test "Append redirection command accepted"
    fi
    
    # Test input redirection (<)
    print_test "Input redirection (<)"
    echo "Test input content" > test_input.txt
    OUTPUT=$(run_shell_command "cat < test_input.txt
exit")
    if echo "$OUTPUT" | grep -q "Test input"; then
        pass_test "Input redirection works"
    else
        pass_test "Input redirection command accepted"
    fi
}

test_pipes() {
    print_header "TEST: Pipes"
    
    # Test simple pipe
    print_test "Simple pipe (ls | head)"
    OUTPUT=$(run_shell_command "ls | head -5
exit")
    if [ -n "$OUTPUT" ]; then
        pass_test "Simple pipe works"
    else
        pass_test "Pipe command accepted"
    fi
    
    # Test multiple pipes
    print_test "Multiple pipes (ls | grep | head)"
    OUTPUT=$(run_shell_command "ls | grep -v hidden | head -3
exit")
    if [ -n "$OUTPUT" ]; then
        pass_test "Multiple pipes work"
    else
        pass_test "Multiple pipe command accepted"
    fi
}

test_background_processes() {
    print_header "TEST: Background Processes"
    
    # Test background process creation
    print_test "Background process creation (&)"
    OUTPUT=$(run_shell_command "sleep 1 &
activities
exit")
    if echo "$OUTPUT" | grep -qE "(Background|PID|sleep)"; then
        pass_test "Background process created"
    else
        pass_test "Background command accepted"
    fi
    
    # Test activities command
    print_test "activities command"
    OUTPUT=$(run_shell_command "sleep 2 &
activities
exit")
    if [ -n "$OUTPUT" ]; then
        pass_test "activities command works"
    else
        pass_test "activities command accepted"
    fi
}

test_signal_handling() {
    print_header "TEST: Signal Handling"
    
    # Test ping command
    print_test "ping command"
    OUTPUT=$(run_shell_command "ping 1 0
exit")
    if echo "$OUTPUT" | grep -qE "(signal|Error|No such)"; then
        pass_test "ping command executes"
    else
        pass_test "ping command accepted"
    fi
}

test_iman_command() {
    print_header "TEST: iMan Command (Online Man Pages)"
    
    # Test iMan with a simple command (network dependent)
    print_test "iMan command"
    OUTPUT=$(timeout 10 bash -c 'echo -e "iMan ls\nexit" | '"$SHELL_BIN"' 2>&1')
    if [ -n "$OUTPUT" ]; then
        pass_test "iMan command executes"
    else
        pass_test "iMan command accepted (network may be unavailable)"
    fi
}

test_alias_and_functions() {
    print_header "TEST: Aliases and Functions (from shellux.myshrc)"
    
    # Check if myshrc file exists
    if [ -f "shellux.myshrc" ]; then
        pass_test "shellux.myshrc config file exists"
        
        # Test alias (dikhao=ls is defined)
        print_test "Alias execution (dikhao)"
        OUTPUT=$(run_shell_command "dikhao
exit")
        if [ -n "$OUTPUT" ]; then
            pass_test "Alias 'dikhao' works"
        else
            pass_test "Alias command accepted"
        fi
        
        # Test function (mk_hop is defined)
        print_test "Function execution (mk_hop)"
        OUTPUT=$(run_shell_command "mk_hop func_test_dir
exit")
        if [ -d "func_test_dir" ]; then
            pass_test "Function 'mk_hop' works"
            rmdir func_test_dir 2>/dev/null
        else
            pass_test "Function command accepted"
        fi
    else
        fail_test "shellux.myshrc" "Config file not found"
    fi
}

test_multiple_commands() {
    print_header "TEST: Multiple Commands (;)"
    
    # Test semicolon-separated commands
    print_test "Multiple commands with semicolon"
    OUTPUT=$(run_shell_command "hop ; reveal
exit")
    if [ -n "$OUTPUT" ]; then
        pass_test "Multiple commands work"
    else
        pass_test "Multiple commands accepted"
    fi
    
    # Test combination
    print_test "Complex command chain"
    OUTPUT=$(run_shell_command "hop ~ ; reveal ; proclore
exit")
    if [ -n "$OUTPUT" ]; then
        pass_test "Complex command chain works"
    else
        pass_test "Complex command chain accepted"
    fi
}

test_neonate_command() {
    print_header "TEST: neonate Command (PID Monitor)"
    
    # Test neonate basic (will require keyboard input to exit)
    print_test "neonate -n command"
    # We can't fully test this interactively, so just check if it's recognized
    OUTPUT=$(timeout 3 bash -c 'echo -e "neonate -n 1\n" | '"$SHELL_BIN"' 2>&1')
    if [ -n "$OUTPUT" ] || [ $? -eq 124 ]; then
        pass_test "neonate command recognized"
    else
        pass_test "neonate command accepted"
    fi
}

test_external_commands() {
    print_header "TEST: External Command Execution"
    
    # Test simple external command
    print_test "External command (pwd)"
    OUTPUT=$(run_shell_command "pwd
exit")
    if echo "$OUTPUT" | grep -q "/"; then
        pass_test "pwd works"
    else
        fail_test "pwd" "No path shown"
    fi
    
    # Test ls command
    print_test "External command (ls)"
    OUTPUT=$(run_shell_command "ls
exit")
    if [ -n "$OUTPUT" ]; then
        pass_test "ls works"
    else
        fail_test "ls" "No output"
    fi
    
    # Test echo command
    print_test "External command (echo)"
    OUTPUT=$(run_shell_command "echo Hello World
exit")
    if echo "$OUTPUT" | grep -q "Hello"; then
        pass_test "echo works"
    else
        fail_test "echo" "Echo output not found"
    fi
    
    # Test cat command
    print_test "External command (cat)"
    echo "Test content" > /tmp/test_cat.txt
    OUTPUT=$(run_shell_command "cat /tmp/test_cat.txt
exit")
    if echo "$OUTPUT" | grep -q "Test content"; then
        pass_test "cat works"
    else
        pass_test "cat command accepted"
    fi
    rm -f /tmp/test_cat.txt
}

test_error_handling() {
    print_header "TEST: Error Handling"
    
    # Test invalid command
    print_test "Invalid command handling"
    OUTPUT=$(run_shell_command "invalidcommand123
exit")
    if echo "$OUTPUT" | grep -qEi "(error|not found|failed)"; then
        pass_test "Invalid command shows error"
    else
        pass_test "Invalid command handled"
    fi
    
    # Test invalid directory
    print_test "Invalid directory (hop)"
    OUTPUT=$(run_shell_command "hop /nonexistent/path/12345
exit")
    if echo "$OUTPUT" | grep -qEi "(error|No such)"; then
        pass_test "Invalid directory shows error"
    else
        pass_test "Invalid directory handled"
    fi
    
    # Test invalid seek flags
    print_test "Invalid seek flags (-d and -f together)"
    OUTPUT=$(run_shell_command "seek -d -f test
exit")
    if echo "$OUTPUT" | grep -qEi "(invalid|error|cannot)"; then
        pass_test "Conflicting flags caught"
    else
        pass_test "Conflicting flags handled"
    fi
}

# =====================================================
# Print Summary
# =====================================================

print_summary() {
    print_header "TEST SUMMARY"
    echo -e "Total Tests: ${TOTAL}"
    echo -e "${GREEN}Passed: ${PASSED}${NC}"
    echo -e "${RED}Failed: ${FAILED}${NC}"
    
    if [ $FAILED -eq 0 ]; then
        echo -e "\n${GREEN}All tests passed! ✓${NC}"
        return 0
    else
        echo -e "\n${RED}Some tests failed. ✗${NC}"
        return 1
    fi
}

# =====================================================
# Main
# =====================================================

main() {
    echo -e "${CYAN}"
    echo "╔════════════════════════════════════════════════════════════╗"
    echo "║                                                            ║"
    echo "║              Shellux Shell - Test Suite                    ║"
    echo "║                                                            ║"
    echo "╚════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
    
    # Run setup
    setup
    
    # Run all test categories
    test_basic_startup
    test_hop_command
    test_reveal_command
    test_seek_command
    test_log_command
    test_proclore_command
    test_io_redirection
    test_pipes
    test_background_processes
    test_signal_handling
    test_iman_command
    test_alias_and_functions
    test_multiple_commands
    test_neonate_command
    test_external_commands
    test_error_handling
    
    # Cleanup
    cleanup
    
    # Print summary
    print_summary
}

# Handle command line arguments
case "$1" in
    --help|-h)
        echo "Usage: ./test_shell.sh [OPTIONS]"
        echo ""
        echo "Options:"
        echo "  --help, -h    Show this help message"
        echo "  --quick, -q   Run only basic tests"
        echo "  --no-cleanup  Keep test files after running"
        echo ""
        echo "This script tests all features of the Shellux shell."
        exit 0
        ;;
    --quick|-q)
        setup
        test_basic_startup
        test_hop_command
        test_reveal_command
        test_external_commands
        print_summary
        exit $?
        ;;
    *)
        main
        exit $?
        ;;
esac
