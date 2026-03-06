#!/bin/bash
# Test script for IRC server authentication improvements
# Tests RFC 1459 compliance for PASS, NICK, USER commands

PORT=6667
PASSWORD="test123"

echo "=========================================="
echo "IRC Authentication Test Suite"
echo "=========================================="
echo ""

# Test 1: NICK before PASS (should work now)
echo "Test 1: NICK before PASS (flexible order)"
echo "Commands: NICK testuser -> PASS test123 -> USER test 0 * :Real"
echo "------------------------------------------"
(
    sleep 0.5
    echo "NICK testuser"
    sleep 0.5
    echo "PASS test123"
    sleep 0.5
    echo "USER test 0 * :Real Name"
    sleep 1
    echo "QUIT"
) | nc localhost $PORT 2>/dev/null | head -10
echo ""

# Test 2: Valid nickname with special characters
echo "Test 2: Nickname with special characters"
echo "Commands: PASS test123 -> NICK test[bot] -> USER test 0 * :Real"
echo "------------------------------------------"
(
    sleep 0.5
    echo "PASS test123"
    sleep 0.5
    echo "NICK test[bot]"
    sleep 0.5
    echo "USER test 0 * :Real Name"
    sleep 1
    echo "QUIT"
) | nc localhost $PORT 2>/dev/null | head -10
echo ""

# Test 3: Nickname with digits
echo "Test 3: Nickname with digits (user123)"
echo "Commands: PASS test123 -> NICK user123 -> USER test 0 * :Real"
echo "------------------------------------------"
(
    sleep 0.5
    echo "PASS test123"
    sleep 0.5
    echo "NICK user123"
    sleep 0.5
    echo "USER test 0 * :Real Name"
    sleep 1
    echo "QUIT"
) | nc localhost $PORT 2>/dev/null | head -10
echo ""

# Test 4: Invalid - nickname too long
echo "Test 4: Invalid nickname (too long - >9 chars)"
echo "Commands: PASS test123 -> NICK verylongnickname"
echo "------------------------------------------"
(
    sleep 0.5
    echo "PASS test123"
    sleep 0.5
    echo "NICK verylongnickname"
    sleep 1
    echo "QUIT"
) | nc localhost $PORT 2>/dev/null | head -10
echo ""

# Test 5: Invalid - nickname starts with digit
echo "Test 5: Invalid nickname (starts with digit)"
echo "Commands: PASS test123 -> NICK 123user"
echo "------------------------------------------"
(
    sleep 0.5
    echo "PASS test123"
    sleep 0.5
    echo "NICK 123user"
    sleep 1
    echo "QUIT"
) | nc localhost $PORT 2>/dev/null | head -10
echo ""

# Test 6: Duplicate USER command
echo "Test 6: Duplicate USER command (should be rejected)"
echo "Commands: PASS -> NICK -> USER -> USER (again)"
echo "------------------------------------------"
(
    sleep 0.5
    echo "PASS test123"
    sleep 0.5
    echo "NICK testuser"
    sleep 0.5
    echo "USER test 0 * :Real Name"
    sleep 0.5
    echo "USER test2 0 * :Different Name"
    sleep 1
    echo "QUIT"
) | nc localhost $PORT 2>/dev/null | head -15
echo ""

# Test 7: Wrong password
echo "Test 7: Wrong password"
echo "Commands: PASS wrongpass"
echo "------------------------------------------"
(
    sleep 0.5
    echo "PASS wrongpass"
    sleep 1
    echo "QUIT"
) | nc localhost $PORT 2>/dev/null | head -10
echo ""

echo "=========================================="
echo "Test suite completed"
echo "=========================================="
