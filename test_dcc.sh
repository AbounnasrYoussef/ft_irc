#!/bin/bash
# DCC Test Script for ft_irc server
# Tests PRIVMSG and CTCP message forwarding

SERVER_HOST="127.0.0.1"
SERVER_PORT="6667"
SERVER_PASS="123"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=== FT_IRC DCC Test Script ==="
echo ""
echo "This script tests PRIVMSG forwarding including CTCP DCC messages"
echo ""

# Create named pipes for communication
FIFO_SENDER="/tmp/irc_sender_$$"
FIFO_RECEIVER="/tmp/irc_receiver_$$"

mkfifo "$FIFO_SENDER" "$FIFO_RECEIVER" 2>/dev/null

# Cleanup function
cleanup() {
    rm -f "$FIFO_SENDER" "$FIFO_RECEIVER"
    rm -f /tmp/sender_out_$$ /tmp/receiver_out_$$
    jobs -p | xargs kill 2>/dev/null
    wait 2>/dev/null
}
trap cleanup EXIT

# Start background netcat processes
nc "$SERVER_HOST" "$SERVER_PORT" < "$FIFO_SENDER" > /tmp/sender_out_$$ 2>&1 &
SENDER_PID=$!
nc "$SERVER_HOST" "$SERVER_PORT" < "$FIFO_RECEIVER" > /tmp/receiver_out_$$ 2>&1 &
RECEIVER_PID=$!

sleep 0.5

# Check if connections succeeded
if ! kill -0 $SENDER_PID 2>/dev/null || ! kill -0 $RECEIVER_PID 2>/dev/null; then
    echo -e "${RED}✗ ERROR: Cannot connect to server at ${SERVER_HOST}:${SERVER_PORT}${NC}"
    echo "Make sure the server is running: ./ircserv ${SERVER_PORT} ${SERVER_PASS}"
    exit 1
fi

echo -e "${GREEN}✓ Both clients connected${NC}"

# Open FIFOs for writing
exec 3>"$FIFO_SENDER"
exec 4>"$FIFO_RECEIVER"

# Register sender
echo -e "PASS ${SERVER_PASS}\r" >&3
echo -e "NICK sender\r" >&3
echo -e "USER sender 0 * :Sender User\r" >&3

# Register receiver
echo -e "PASS ${SERVER_PASS}\r" >&4
echo -e "NICK receiver\r" >&4
echo -e "USER receiver 0 * :Receiver User\r" >&4

# Wait for registration
sleep 1

# Check welcome messages
if grep -q "Welcome" /tmp/sender_out_$$ && grep -q "Welcome" /tmp/receiver_out_$$; then
    echo -e "${GREEN}✓ Both clients registered${NC}"
else
    echo -e "${RED}✗ Registration failed${NC}"
    echo "Sender output:"
    cat /tmp/sender_out_$$
    echo "Receiver output:"
    cat /tmp/receiver_out_$$
    exit 1
fi

echo ""
echo "Test 1: Simple PRIVMSG forwarding"
echo "-----------------------------------"

# Clear receiver output
> /tmp/receiver_out_$$

echo "Sending: PRIVMSG receiver :Hello World"
echo -e "PRIVMSG receiver :Hello World\r" >&3
sleep 0.5

RESPONSE=$(cat /tmp/receiver_out_$$)
echo "Received: $RESPONSE"

if echo "$RESPONSE" | grep -q "sender.*PRIVMSG receiver :Hello World"; then
    echo -e "${GREEN}✓ Simple PRIVMSG works correctly${NC}"
else
    echo -e "${RED}✗ Simple PRIVMSG failed${NC}"
fi

echo ""
echo "Test 2: CTCP VERSION"
echo "-----------------------------------"

# Clear receiver output
> /tmp/receiver_out_$$

echo "Sending: PRIVMSG receiver :\x01VERSION\x01"
printf "PRIVMSG receiver :\x01VERSION\x01\r\n" >&3
sleep 0.5

RESPONSE=$(cat /tmp/receiver_out_$$)
echo "Received (hex): $(echo "$RESPONSE" | od -An -tx1 | head -1)"
echo "Received (text): $RESPONSE"
echo "$RESPONSE" | hexdump -C

if echo "$RESPONSE" | grep -q "VERSION"; then
    echo -e "${GREEN}✓ CTCP VERSION forwarded${NC}"
    if echo "$RESPONSE" | od -An -tx1 | grep -q "01"; then
        echo -e "${GREEN}✓ CTCP delimiters (\\x01) preserved${NC}"
    else
        echo -e "${RED}✗ CTCP delimiters may be missing${NC}"
    fi
else
    echo -e "${RED}✗ CTCP VERSION failed${NC}"
fi

echo ""
echo "Test 3: CTCP DCC SEND (simulated)"
echo "-----------------------------------"

# Clear receiver output
> /tmp/receiver_out_$$

# DCC SEND format: \x01DCC SEND filename ip port size\x01
# IP: 127.0.0.1 = 2130706433 in decimal
echo "Sending: PRIVMSG receiver :\x01DCC SEND test.txt 2130706433 12345 1024\x01"
printf "PRIVMSG receiver :\x01DCC SEND test.txt 2130706433 12345 1024\x01\r\n" >&3
sleep 0.5

RESPONSE=$(cat /tmp/receiver_out_$$)
echo "Received (hex): $(echo "$RESPONSE" | od -An -tx1 | head -1)"
echo "Received (text): $RESPONSE"

if echo "$RESPONSE" | grep -q "DCC SEND"; then
    echo -e "${GREEN}✓ DCC SEND message forwarded${NC}"
    
    if echo "$RESPONSE" | od -An -tx1 | grep -q "01.*44 43 43.*01"; then
        echo -e "${GREEN}✓ CTCP delimiters (\\x01) preserved${NC}"
    else
        echo -e "${YELLOW}⚠ CTCP delimiters may be missing (check hex output above)${NC}"
    fi
    
    if echo "$RESPONSE" | grep -q "2130706433" && echo "$RESPONSE" | grep -q "12345"; then
        echo -e "${GREEN}✓ DCC parameters present${NC}"
    else
        echo -e "${RED}✗ Some DCC parameters missing${NC}"
    fi
else
    echo -e "${RED}✗ DCC SEND failed${NC}"
fi

echo ""
echo "Test 4: Special characters in message"
echo "-----------------------------------"

# Clear receiver output
> /tmp/receiver_out_$$

echo "Sending: PRIVMSG receiver :Test with special chars: !@#\$%^&*()"
echo -e "PRIVMSG receiver :Test with special chars: !@#\$%^&*()\r" >&3
sleep 0.5

RESPONSE=$(cat /tmp/receiver_out_$$)
echo "Received: $RESPONSE"

if echo "$RESPONSE" | grep -q "special chars"; then
    echo -e "${GREEN}✓ Special characters preserved${NC}"
else
    echo -e "${RED}✗ Special characters test failed${NC}"
fi

# Cleanup
echo -e "QUIT :Goodbye\r" >&3
echo -e "QUIT :Goodbye\r" >&4
sleep 0.2

exec 3>&-
exec 4>&-

echo ""
echo "=== Test Complete ==="
echo ""
echo "Check server output for debug logs showing:"
echo "  - Raw message received"
echo "  - Formatted message sent"
echo "  - CTCP delimiters (<CTCP> markers)"
echo "  - DCC detection messages"
echo ""
echo -e "${YELLOW}Note: The Python test script (test_dcc_python.py) is more reliable${NC}"
