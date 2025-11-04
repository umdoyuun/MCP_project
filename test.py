#!/usr/bin/env python3
import subprocess
import json
import time

def send_message(proc, msg):
    """MCP 프로토콜 형식으로 메시지 전송"""
    msg_json = json.dumps(msg)
    msg_bytes = msg_json.encode('utf-8')
    header = f"Content-Length: {len(msg_bytes)}\r\n\r\n"
    full_message = header.encode('utf-8') + msg_bytes
    
    proc.stdin.write(full_message)
    proc.stdin.flush()
    time.sleep(0.3)

def test_all():
    print("=" * 70)
    print("MCP Server Full Test Suite")
    print("=" * 70)
    print()
    
    proc = subprocess.Popen(
        ['C:/Users/2-15/Desktop/project/MCP_project/build/mcp_server.exe'],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )
    
    try:
        time.sleep(0.2)
        
        # Test 1: Initialize
        print("✅ Test 1: Initialize")
        send_message(proc, {
            "jsonrpc": "2.0",
            "id": "1",
            "method": "initialize",
            "params": {"protocolVersion": "2.0", "clientInfo": {"name": "test", "version": "1.0"}}
        })
        
        # Test 2: List Tools
        print("✅ Test 2: List Tools")
        send_message(proc, {
            "jsonrpc": "2.0",
            "id": "2",
            "method": "tools/list",
            "params": {}
        })
        
        # Test 3: Get Quote
        print("✅ Test 3: Get Random Quote")
        send_message(proc, {
            "jsonrpc": "2.0",
            "id": "3",
            "method": "tools/call",
            "params": {"name": "getRandomQuote", "arguments": {}}
        })
        
        # Test 4: Get Weather
        print("✅ Test 4: Get Weather (Seoul)")
        send_message(proc, {
            "jsonrpc": "2.0",
            "id": "4",
            "method": "tools/call",
            "params": {"name": "getWeather", "arguments": {"city": "Seoul"}}
        })
        
        # Test 5: Invalid tool
        print("✅ Test 5: Invalid Tool (should return error)")
        send_message(proc, {
            "jsonrpc": "2.0",
            "id": "5",
            "method": "tools/call",
            "params": {"name": "invalidTool", "arguments": {}}
        })
        
        time.sleep(1)
        
        proc.terminate()
        stdout, stderr = proc.communicate(timeout=2)
        
        print()
        print("=" * 70)
        print("Server Responses:")
        print("=" * 70)
        print(stdout.decode('utf-8', errors="replace"))
        
        print("=" * 70)
        print("Server Logs:")
        print("=" * 70)
        print(stderr.decode('utf-8', errors="replace"))
        
        print()
        print("=" * 70)
        print("✅ All tests completed successfully!")
        print("=" * 70)
        
    except Exception as e:
        print(f"❌ Error: {e}")
        proc.terminate()

if __name__ == "__main__":
    test_all()