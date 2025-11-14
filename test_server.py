#!/usr/bin/env python3
"""
Test Server for Robot Computer WiFi Management
This server runs on Ubuntu 22.04 robot computer and handles WiFi scanning requests
"""

import socket
import json
import subprocess
import sys
import signal
import os
from typing import List, Dict, Optional

class RobotWiFiServer:
    def __init__(self, host='0.0.0.0', port=8888):
        self.host = host
        self.port = port
        self.server_socket = None
        self.running = False
    
    def get_robot_name(self) -> str:
        """
        Get robot name from environment variable ROM_ROBOT_MODEL
        """
        robot_name = os.environ.get('ROM_ROBOT_MODEL', 'UNKNOWN_ROBOT')
        return robot_name
    
    def get_wifi_status(self) -> Dict[str, any]:
        """
        Get current WiFi connection status
        Returns dict with connected status, SSID, IP address, and robot name
        """
        try:
            # Check if connected to WiFi
            result = subprocess.run(
                ['nmcli', '-t', '-f', 'NAME,TYPE,DEVICE', 'connection', 'show', '--active'],
                capture_output=True,
                text=True,
                timeout=5
            )
            
            connected = False
            ssid = ""
            wifi_device = ""
            
            # Find active WiFi connection
            for line in result.stdout.strip().split('\n'):
                if not line:
                    continue
                parts = line.split(':')
                if len(parts) >= 3 and parts[1] == '802-11-wireless':
                    connected = True
                    ssid = parts[0]
                    wifi_device = parts[2]
                    break
            
            ip_address = ""
            if connected and wifi_device:
                # Get IP address of WiFi device
                ip_result = subprocess.run(
                    ['ip', '-4', 'addr', 'show', wifi_device],
                    capture_output=True,
                    text=True,
                    timeout=5
                )
                
                # Parse IP address from output
                for line in ip_result.stdout.split('\n'):
                    if 'inet ' in line:
                        ip_address = line.strip().split()[1].split('/')[0]
                        break
            
            robot_name = self.get_robot_name()
            
            return {
                'connected': connected,
                'ssid': ssid if connected else '',
                'ip': ip_address if connected else '',
                'robot_name': robot_name
            }
            
        except subprocess.TimeoutExpired:
            print("WiFi status check timeout", file=sys.stderr)
            return {
                'connected': False,
                'ssid': '',
                'ip': '',
                'robot_name': self.get_robot_name()
            }
        except Exception as e:
            print(f"Exception getting WiFi status: {e}", file=sys.stderr)
            return {
                'connected': False,
                'ssid': '',
                'ip': '',
                'robot_name': self.get_robot_name()
            }
        
    def scan_wifi_networks(self) -> List[Dict[str, str]]:
        """
        Scan available WiFi networks using nmcli
        Returns list of dictionaries with SSID and security type
        """
        try:
            # Use nmcli to scan WiFi networks
            result = subprocess.run(
                ['nmcli', '-t', '-f', 'SSID,SECURITY', 'dev', 'wifi', 'list'],
                capture_output=True,
                text=True,
                timeout=10
            )
            
            if result.returncode != 0:
                print(f"Error scanning WiFi: {result.stderr}", file=sys.stderr)
                return []
            
            networks = []
            lines = result.stdout.strip().split('\n')
            
            for line in lines:
                if not line.strip():
                    continue
                    
                parts = line.split(':')
                if len(parts) >= 2:
                    ssid = parts[0].strip()
                    security = parts[1].strip() if parts[1] else 'Open'
                    
                    # Skip empty SSIDs
                    if ssid and ssid != '--':
                        networks.append({
                            'ssid': ssid,
                            'security': security if security else 'Open'
                        })
            
            # Remove duplicates (keep first occurrence)
            seen = set()
            unique_networks = []
            for network in networks:
                if network['ssid'] not in seen:
                    seen.add(network['ssid'])
                    unique_networks.append(network)
            
            print(f"Found {len(unique_networks)} WiFi networks")
            return unique_networks
            
        except subprocess.TimeoutExpired:
            print("WiFi scan timeout", file=sys.stderr)
            return []
        except Exception as e:
            print(f"Exception during WiFi scan: {e}", file=sys.stderr)
            return []
    
    def connect_to_wifi(self, ssid: str, password: str) -> Dict[str, any]:
        """
        Connect to WiFi network
        Returns dict with success status and message
        """
        try:
            print(f"Attempting to connect to WiFi: {ssid}")
            
            # Check if password is needed (for open networks)
            if not password or password == "":
                # Try connecting without password (open network)
                result = subprocess.run(
                    ['nmcli', 'device', 'wifi', 'connect', ssid],
                    capture_output=True,
                    text=True,
                    timeout=30
                )
            else:
                # Connect with password
                result = subprocess.run(
                    ['nmcli', 'device', 'wifi', 'connect', ssid, 'password', password],
                    capture_output=True,
                    text=True,
                    timeout=30
                )
            
            if result.returncode == 0:
                print(f"Successfully connected to {ssid}")
                return {
                    'success': True,
                    'message': f'Successfully connected to {ssid}'
                }
            else:
                error_msg = result.stderr.strip() if result.stderr else result.stdout.strip()
                print(f"Failed to connect to {ssid}: {error_msg}", file=sys.stderr)
                return {
                    'success': False,
                    'message': f'Failed to connect: {error_msg}'
                }
                
        except subprocess.TimeoutExpired:
            print(f"WiFi connection timeout for {ssid}", file=sys.stderr)
            return {
                'success': False,
                'message': 'Connection timeout'
            }
        except Exception as e:
            print(f"Exception during WiFi connection: {e}", file=sys.stderr)
            return {
                'success': False,
                'message': f'Connection error: {str(e)}'
            }
    
    def handle_command(self, command_data: dict) -> dict:
        """
        Handle incoming command from client
        """
        command = command_data.get('command', '')
        params = command_data.get('params', {})
        
        print(f"Received command: {command}")
        
        if command == 'scan_wifi':
            networks = self.scan_wifi_networks()
            return {
                'type': 'wifi_list',
                'data': networks
            }
        elif command == 'get_wifi_status':
            status = self.get_wifi_status()
            return {
                'type': 'wifi_status',
                'data': status
            }
        elif command == 'connect_wifi':
            ssid = params.get('ssid', '')
            password = params.get('password', '')
            
            if not ssid:
                return {
                    'type': 'wifi_connection_result',
                    'success': False,
                    'message': 'SSID is required'
                }
            
            result = self.connect_to_wifi(ssid, password)
            return {
                'type': 'wifi_connection_result',
                'success': result['success'],
                'message': result['message']
            }
        else:
            return {
                'type': 'error',
                'message': f'Unknown command: {command}'
            }
    
    def handle_client(self, client_socket, client_address):
        """
        Handle client connection
        """
        print(f"Client connected from {client_address}")
        
        try:
            buffer = b''
            
            while self.running:
                # Receive data
                data = client_socket.recv(1024)
                
                if not data:
                    print(f"Client {client_address} disconnected")
                    break
                
                buffer += data
                
                # Process complete messages (newline-delimited JSON)
                while b'\n' in buffer:
                    newline_index = buffer.index(b'\n')
                    message = buffer[:newline_index]
                    buffer = buffer[newline_index + 1:]
                    
                    if message:
                        try:
                            # Parse JSON command
                            command_data = json.loads(message.decode('utf-8'))
                            print(f"Received: {command_data}")
                            
                            # Handle command
                            response = self.handle_command(command_data)
                            
                            # Send response
                            response_json = json.dumps(response) + '\n'
                            client_socket.sendall(response_json.encode('utf-8'))
                            print(f"Sent response: {response_json.strip()}")
                            
                        except json.JSONDecodeError as e:
                            error_response = {
                                'type': 'error',
                                'message': f'Invalid JSON: {str(e)}'
                            }
                            client_socket.sendall((json.dumps(error_response) + '\n').encode('utf-8'))
                        except Exception as e:
                            error_response = {
                                'type': 'error',
                                'message': f'Error processing command: {str(e)}'
                            }
                            client_socket.sendall((json.dumps(error_response) + '\n').encode('utf-8'))
        
        except Exception as e:
            print(f"Error handling client {client_address}: {e}", file=sys.stderr)
        finally:
            client_socket.close()
            print(f"Client {client_address} connection closed")
    
    def start(self):
        """
        Start the server
        """
        try:
            # Create socket
            self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            
            # Bind and listen
            self.server_socket.bind((self.host, self.port))
            self.server_socket.listen(5)
            
            self.running = True
            
            print(f"Robot WiFi Server started on {self.host}:{self.port}")
            print("Waiting for connections...")
            
            while self.running:
                try:
                    # Accept client connection
                    client_socket, client_address = self.server_socket.accept()
                    
                    # Handle client (blocking - one client at a time for simplicity)
                    self.handle_client(client_socket, client_address)
                    
                except KeyboardInterrupt:
                    print("\nShutting down server...")
                    self.running = False
                    break
                except Exception as e:
                    print(f"Error accepting connection: {e}", file=sys.stderr)
        
        finally:
            if self.server_socket:
                self.server_socket.close()
            print("Server stopped")
    
    def stop(self):
        """
        Stop the server
        """
        self.running = False
        if self.server_socket:
            self.server_socket.close()


def signal_handler(sig, frame):
    """Handle Ctrl+C gracefully"""
    print("\nReceived signal to terminate. Shutting down...")
    sys.exit(0)


if __name__ == '__main__':
    # Set up signal handler for graceful shutdown
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    # Create and start server
    server = RobotWiFiServer(host='10.0.0.5', port=8888)
    # server = RobotWiFiServer(host='0.0.0.0', port=8888)
    # server = RobotWiFiServer(host='0.0.0.0', port=8888)
    
    try:
        server.start()
    except Exception as e:
        print(f"Fatal error: {e}", file=sys.stderr)
        sys.exit(1)
