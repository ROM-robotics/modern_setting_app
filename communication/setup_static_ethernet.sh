#!/bin/bash
# Setup static IP for ethernet and DHCP server for tablet connection
# Ubuntu 22.04 - NetworkManager configuration

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root (use sudo)"
    exit 1
fi

# Configuration
INTERFACE="eth0"  # Change this to your ethernet interface name (eth0, enp0s3, etc.)
STATIC_IP="10.0.0.5"
NETMASK="24"
TABLET_IP="10.0.0.6"
CONNECTION_NAME="Ethernet-Static-Tablet"

echo "==================================="
echo "Ethernet Static IP Setup"
echo "==================================="
echo "Interface: $INTERFACE"
echo "Robot IP: $STATIC_IP/$NETMASK"
echo "Tablet IP: $TABLET_IP (via DHCP)"
echo "==================================="

# Function to detect ethernet interface
detect_ethernet_interface() {
    echo "Detecting ethernet interfaces..."
    
    # List all ethernet interfaces
    ETH_INTERFACES=$(ip link show | grep -E '^[0-9]+: (eth|enp|eno)' | awk -F: '{print $2}' | tr -d ' ')
    
    if [ -z "$ETH_INTERFACES" ]; then
        echo "ERROR: No ethernet interface found!"
        echo "Available interfaces:"
        ip link show
        exit 1
    fi
    
    echo "Found ethernet interface(s):"
    echo "$ETH_INTERFACES"
    
    # Use first detected interface
    INTERFACE=$(echo "$ETH_INTERFACES" | head -n 1)
    echo "Using interface: $INTERFACE"
}

# Auto-detect ethernet interface
detect_ethernet_interface

# Stop and disable systemd-networkd if running (we'll use NetworkManager)
systemctl stop systemd-networkd 2>/dev/null
systemctl disable systemd-networkd 2>/dev/null

# Install required packages
echo ""
echo "Installing required packages..."
apt-get update
apt-get install -y network-manager dnsmasq

# Stop dnsmasq (we'll configure it later)
systemctl stop dnsmasq
systemctl disable dnsmasq

# Enable and start NetworkManager
systemctl enable NetworkManager
systemctl start NetworkManager

# Wait for NetworkManager to start
sleep 2

# Delete existing connection if it exists
echo ""
echo "Removing existing connection configuration..."
nmcli connection delete "$CONNECTION_NAME" 2>/dev/null
nmcli connection delete "Wired connection 1" 2>/dev/null
nmcli connection delete "Ethernet" 2>/dev/null

# Create new connection with static IP and shared mode
echo ""
echo "Creating static IP configuration..."
nmcli connection add \
    type ethernet \
    con-name "$CONNECTION_NAME" \
    ifname "$INTERFACE" \
    ipv4.method shared \
    ipv4.addresses "$STATIC_IP/$NETMASK"

# Configure connection to auto-connect
nmcli connection modify "$CONNECTION_NAME" \
    connection.autoconnect yes \
    connection.autoconnect-priority 10

# Bring up the connection
echo ""
echo "Activating connection..."
nmcli connection up "$CONNECTION_NAME"

# Configure dnsmasq for specific DHCP range (only tablet IP)
echo ""
echo "Configuring DHCP server for tablet..."

cat > /etc/NetworkManager/dnsmasq-shared.d/tablet-dhcp.conf <<EOF
# DHCP configuration for tablet connection
# Only assign IP to one device (tablet)

interface=$INTERFACE
bind-interfaces

# DHCP range - only one IP for tablet
dhcp-range=$TABLET_IP,$TABLET_IP,12h

# Set gateway (robot IP)
dhcp-option=3,$STATIC_IP

# Set DNS server (robot IP)
dhcp-option=6,$STATIC_IP

# Disable DNS server functionality
port=0

# Log DHCP requests
log-dhcp
log-facility=/var/log/dnsmasq-tablet.log

# Static lease for tablet (optional - add tablet MAC address if known)
# dhcp-host=AA:BB:CC:DD:EE:FF,$TABLET_IP,tablet,12h
EOF

# Create dnsmasq config directory if not exists
mkdir -p /etc/NetworkManager/dnsmasq-shared.d/

# Enable dnsmasq for NetworkManager shared connections
cat > /etc/NetworkManager/conf.d/shared-dnsmasq.conf <<EOF
[main]
dns=dnsmasq

[connection]
ipv4.dhcp-timeout=30
EOF

# Restart NetworkManager to apply dnsmasq configuration
echo ""
echo "Restarting NetworkManager..."
systemctl restart NetworkManager

# Wait for interface to come up
sleep 3

# Bring up connection again after restart
nmcli connection up "$CONNECTION_NAME"

# Verify configuration
echo ""
echo "==================================="
echo "Configuration Complete!"
echo "==================================="
echo ""
echo "Current IP configuration:"
ip addr show "$INTERFACE"
echo ""
echo "Connection status:"
nmcli connection show "$CONNECTION_NAME"
echo ""
echo "==================================="
echo "Testing Instructions:"
echo "==================================="
echo "1. Connect tablet to robot via ethernet cable"
echo "2. Tablet should automatically receive IP: $TABLET_IP"
echo "3. From tablet, ping robot: ping $STATIC_IP"
echo "4. From robot, check DHCP log: tail -f /var/log/dnsmasq-tablet.log"
echo ""
echo "To check tablet MAC address (after connection):"
echo "  cat /var/lib/NetworkManager/dnsmasq-$INTERFACE.leases"
echo ""
echo "To add static lease for tablet (optional):"
echo "  1. Find tablet MAC from leases file"
echo "  2. Edit /etc/NetworkManager/dnsmasq-shared.d/tablet-dhcp.conf"
echo "  3. Uncomment and update: dhcp-host=<TABLET_MAC>,$TABLET_IP,tablet,12h"
echo "  4. Restart: systemctl restart NetworkManager"
echo "==================================="
