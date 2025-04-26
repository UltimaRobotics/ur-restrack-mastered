# OpenWrt System Monitor Use Cases and Examples

This document provides practical examples and use cases for the OpenWrt System Monitor (sysmon) application.

## Monitoring Use Cases

### 1. Long-term System Health Monitoring

#### Configuration
```json
{
  "output_path": "/mnt/storage/system_stats.json",
  "log_path": "/var/log/sysmon.log",
  "collection_interval": 900,
  "verbose": false,
  "collect_cpu": true,
  "collect_memory": true,
  "collect_load": true,
  "collect_disk": true,
  "collect_network": true,
  "collect_uptime": true,
  "collect_processes": true,
  "collect_swap": true
}
```

#### Command
```bash
./sysmon -c /etc/sysmon_long_term.json
```

#### Notes
- Collects data every 15 minutes (900 seconds)
- Saves to an external storage device to avoid flash wear
- Collects all available metrics for comprehensive analysis

### 2. Network Performance Monitoring

#### Configuration
```json
{
  "output_path": "/var/log/network_stats.json",
  "log_path": "/var/log/sysmon_network.log",
  "collection_interval": 60,
  "verbose": false,
  "collect_cpu": false,
  "collect_memory": false,
  "collect_load": false,
  "collect_disk": false,
  "collect_network": true,
  "collect_uptime": false,
  "collect_processes": false,
  "collect_swap": false
}
```

#### Command
```bash
./sysmon -c /etc/sysmon_network.json
```

#### Notes
- Focuses only on network statistics
- Collects data every minute
- Smaller JSON output since only network data is collected

### 3. Memory Leak Detection

#### Configuration
```json
{
  "output_path": "/var/log/memory_stats.json",
  "log_path": "/var/log/sysmon_memory.log",
  "collection_interval": 300,
  "verbose": true,
  "collect_cpu": false,
  "collect_memory": true,
  "collect_load": false,
  "collect_disk": false,
  "collect_network": false,
  "collect_uptime": false,
  "collect_processes": true,
  "collect_swap": true
}
```

#### Command
```bash
./sysmon -c /etc/sysmon_memory.json
```

#### Notes
- Collects memory, swap, and process information every 5 minutes
- Enables verbose logging for more detailed information
- Useful for identifying memory leaks or processes consuming excessive memory

### 4. Periodic System Snapshot

#### Configuration
```json
{
  "output_path": "/var/log/system_snapshot.json",
  "log_path": "/var/log/sysmon_snapshot.log",
  "collection_interval": 3600,
  "verbose": false,
  "collect_cpu": true,
  "collect_memory": true,
  "collect_load": true,
  "collect_disk": true,
  "collect_network": true,
  "collect_uptime": true,
  "collect_processes": true,
  "collect_swap": true
}
```

#### Command
```bash
./sysmon -c /etc/sysmon_snapshot.json
```

#### Notes
- Takes a comprehensive snapshot every hour
- Provides a daily record with 24 data points
- Balances detail with storage requirements

## Script Examples

### 1. Daily Report Generation

```bash
#!/bin/bash
# daily_report.sh
# Generate a daily system resource report

# Run sysmon once to get current stats
./sysmon -x -o /tmp/current_stats.json

# Extract key metrics using jq (must be installed)
LATEST=$(jq '.history[-1]' /tmp/current_stats.json)

CPU_USAGE=$(echo $LATEST | jq '.cpu_usage.cpus[0].usage_percent')
MEM_TOTAL=$(echo $LATEST | jq '.memory.total_memory')
MEM_USED=$(echo $LATEST | jq '.memory.total_memory - .memory.free_memory')
MEM_PERCENT=$(echo $LATEST | jq '.memory.usage_percent')
UPTIME=$(echo $LATEST | jq -r '.uptime.uptime_pretty')
LOAD=$(echo $LATEST | jq '.load.load_1min')

# Generate report
cat << EOF > /tmp/daily_report.txt
======================================
SYSTEM DAILY REPORT - $(date)
======================================

Uptime: $UPTIME
CPU Usage: $CPU_USAGE%
Memory: $MEM_USED / $MEM_TOTAL bytes ($MEM_PERCENT%)
System Load: $LOAD

EOF

# Send report via email (requires msmtp or similar to be configured)
cat /tmp/daily_report.txt | mail -s "Daily System Report - $(date +%Y-%m-%d)" admin@example.com

# Clean up
rm /tmp/current_stats.json /tmp/daily_report.txt
```

### 2. Resource Threshold Monitoring

```bash
#!/bin/bash
# threshold_monitor.sh
# Monitor system resources and alert if thresholds are exceeded

# Run sysmon once
./sysmon -x -o /tmp/current_stats.json

# Set thresholds
CPU_THRESHOLD=80
MEM_THRESHOLD=85
DISK_THRESHOLD=90

# Extract metrics
LATEST=$(jq '.history[-1]' /tmp/current_stats.json)
CPU_USAGE=$(echo $LATEST | jq '.cpu_usage.cpus[0].usage_percent')
MEM_PERCENT=$(echo $LATEST | jq '.memory.usage_percent')
DISK_PERCENT=$(echo $LATEST | jq '.disk.filesystems[0].usage_percent')

# Check CPU
if (( $(echo "$CPU_USAGE > $CPU_THRESHOLD" | bc -l) )); then
    echo "WARNING: CPU usage is $CPU_USAGE% (threshold: $CPU_THRESHOLD%)" >> /var/log/threshold_alerts.log
    # Send alert via your preferred method (email, SMS, etc.)
fi

# Check Memory
if (( $(echo "$MEM_PERCENT > $MEM_THRESHOLD" | bc -l) )); then
    echo "WARNING: Memory usage is $MEM_PERCENT% (threshold: $MEM_THRESHOLD%)" >> /var/log/threshold_alerts.log
    # Send alert
fi

# Check Disk
if (( $(echo "$DISK_PERCENT > $DISK_THRESHOLD" | bc -l) )); then
    echo "WARNING: Disk usage is $DISK_PERCENT% (threshold: $DISK_THRESHOLD%)" >> /var/log/threshold_alerts.log
    # Send alert
fi

# Clean up
rm /tmp/current_stats.json
```

### 3. Weekly Log Rotation

```bash
#!/bin/bash
# rotate_sysmon_logs.sh
# Rotate sysmon log files weekly

# Stop sysmon
killall sysmon

# Rotate log file
if [ -f /var/log/sysmon.log ]; then
    mv /var/log/sysmon.log /var/log/sysmon.log.$(date +%Y%m%d)
    
    # Compress logs older than a week
    find /var/log -name "sysmon.log.*" -mtime +7 -exec gzip {} \;
    
    # Delete logs older than a month
    find /var/log -name "sysmon.log.*.gz" -mtime +30 -delete
fi

# Rotate data file if needed
if [ -f /var/log/sysmon_data.json ]; then
    # Create weekly snapshot
    cp /var/log/sysmon_data.json /var/log/sysmon_data.$(date +%Y%m%d).json
    
    # Optionally reset the main data file to avoid it growing too large
    echo '{"history":[]}' > /var/log/sysmon_data.json
    
    # Compress snapshots older than a week
    find /var/log -name "sysmon_data.*.json" -mtime +7 -exec gzip {} \;
    
    # Delete snapshots older than a month
    find /var/log -name "sysmon_data.*.json.gz" -mtime +30 -delete
fi

# Restart sysmon
/usr/local/bin/sysmon &
```

## Integration Examples

### 1. Add to OpenWrt Init System

Create an init script at `/etc/init.d/sysmon`:

```bash
#!/bin/sh /etc/rc.common

START=90
STOP=10
USE_PROCD=1

start_service() {
    procd_open_instance
    procd_set_param command /usr/local/bin/sysmon -c /etc/sysmon_config.json
    procd_set_param stdout 1
    procd_set_param stderr 1
    procd_set_param respawn ${respawn_threshold:-3600} ${respawn_timeout:-5} ${respawn_retry:-5}
    procd_close_instance
}
```

Make it executable and enable:
```bash
chmod +x /etc/init.d/sysmon
/etc/init.d/sysmon enable
/etc/init.d/sysmon start
```

### 2. Incorporate with LuCI Web Interface

Create a simple LuCI app to display sysmon data:

```lua
-- /usr/lib/lua/luci/controller/sysmon.lua
module("luci.controller.sysmon", package.seeall)

function index()
    entry({"admin", "status", "sysmon"}, template("sysmon/overview"), _("System Monitor"), 90)
    entry({"admin", "status", "sysmon", "data"}, call("action_get_data"), nil)
end

function action_get_data()
    local fs = require "nixio.fs"
    local json = require "luci.jsonc"
    
    local data = fs.readfile("/var/log/sysmon_data.json")
    if data then
        luci.http.prepare_content("application/json")
        luci.http.write(data)
    else
        luci.http.status(404, "Data file not found")
    end
end
```

### 3. Export to Time-Series Database

Script to export sysmon data to InfluxDB:

```python
#!/usr/bin/env python3
# sysmon_to_influxdb.py

import json
import time
import sys
import requests
from datetime import datetime

# Configuration
INFLUXDB_URL = "http://influxdb.example.com:8086/write"
INFLUXDB_DB = "sysmon"
INFLUXDB_USER = "sysmon"
INFLUXDB_PASS = "password"
SYSMON_DATA = "/var/log/sysmon_data.json"

# Read sysmon data
try:
    with open(SYSMON_DATA, 'r') as f:
        data = json.load(f)
except Exception as e:
    print(f"Error reading sysmon data: {e}", file=sys.stderr)
    sys.exit(1)

# Process latest entry
if 'history' in data and len(data['history']) > 0:
    latest = data['history'][-1]
    
    # CPU metrics
    if 'cpu_usage' in latest:
        for cpu in latest['cpu_usage']['cpus']:
            influx_line = f"cpu,host=openwrt,cpu={cpu['name']} usage_percent={cpu['usage_percent']}"
            requests.post(
                INFLUXDB_URL,
                params={"db": INFLUXDB_DB, "u": INFLUXDB_USER, "p": INFLUXDB_PASS},
                data=influx_line
            )
    
    # Memory metrics
    if 'memory' in latest:
        mem = latest['memory']
        influx_line = f"memory,host=openwrt total={mem['total_memory']},free={mem['free_memory']},usage_percent={mem['usage_percent']}"
        requests.post(
            INFLUXDB_URL,
            params={"db": INFLUXDB_DB, "u": INFLUXDB_USER, "p": INFLUXDB_PASS},
            data=influx_line
        )
    
    # Handle other metrics similarly
    
    print("Data exported to InfluxDB successfully")
else:
    print("No data available to export", file=sys.stderr)
    sys.exit(1)
```

## Visualization Examples

### 1. Command-line Graphs with gnuplot

```bash
#!/bin/bash
# plot_cpu_usage.sh
# Create a CPU usage graph using gnuplot

# Extract CPU data points from sysmon output
jq -r '.history[] | [.timestamp, (.cpu_usage.cpus[0].usage_percent // 0)] | @csv' /var/log/sysmon_data.json > /tmp/cpu_data.csv

# Create gnuplot script
cat << EOF > /tmp/cpu_plot.gnuplot
set terminal png size 800,400
set output '/tmp/cpu_usage.png'
set title 'CPU Usage Over Time'
set xlabel 'Time'
set ylabel 'CPU Usage (%)'
set yrange [0:100]
set datafile separator ','
set xdata time
set timefmt '"%Y-%m-%d %H:%M:%S"'
set format x '%H:%M'
set grid
plot '/tmp/cpu_data.csv' using 1:2 with lines title 'CPU Usage'
EOF

# Generate the plot
gnuplot /tmp/cpu_plot.gnuplot

echo "CPU usage graph generated at /tmp/cpu_usage.png"
```

### 2. Integrated Web Dashboard Example

This is a simple example of how to create a web dashboard to view the collected data using a basic HTML/JavaScript frontend. First, you need to have a web server installed on your OpenWrt device.

Create `/www/sysmon/index.html`:

```html
<!DOCTYPE html>
<html>
<head>
    <title>System Monitor Dashboard</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background: #f5f5f5; }
        .dashboard { display: grid; grid-template-columns: repeat(auto-fill, minmax(500px, 1fr)); gap: 20px; }
        .card { background: white; border-radius: 8px; padding: 15px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .card-header { font-size: 18px; font-weight: bold; margin-bottom: 10px; }
        .metric { font-size: 36px; color: #0066cc; text-align: center; margin: 15px 0; }
        canvas { width: 100% !important; }
    </style>
</head>
<body>
    <h1>System Monitor Dashboard</h1>
    
    <div class="dashboard">
        <div class="card">
            <div class="card-header">System Uptime</div>
            <div class="metric" id="uptime">--</div>
        </div>
        
        <div class="card">
            <div class="card-header">CPU Usage</div>
            <canvas id="cpuChart"></canvas>
        </div>
        
        <div class="card">
            <div class="card-header">Memory Usage</div>
            <canvas id="memoryChart"></canvas>
        </div>
        
        <div class="card">
            <div class="card-header">Network Traffic</div>
            <canvas id="networkChart"></canvas>
        </div>
    </div>
    
    <script>
        // Chart configurations and data fetching
        let cpuChart, memoryChart, networkChart;
        
        // Initialize charts
        function initCharts() {
            // CPU Chart
            const cpuCtx = document.getElementById('cpuChart').getContext('2d');
            cpuChart = new Chart(cpuCtx, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [{
                        label: 'CPU Usage (%)',
                        data: [],
                        borderColor: '#0066cc',
                        backgroundColor: 'rgba(0, 102, 204, 0.1)',
                        fill: true
                    }]
                },
                options: {
                    scales: { y: { min: 0, max: 100 } }
                }
            });
            
            // Memory Chart
            const memoryCtx = document.getElementById('memoryChart').getContext('2d');
            memoryChart = new Chart(memoryCtx, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [{
                        label: 'Memory Usage (%)',
                        data: [],
                        borderColor: '#cc6600',
                        backgroundColor: 'rgba(204, 102, 0, 0.1)',
                        fill: true
                    }]
                },
                options: {
                    scales: { y: { min: 0, max: 100 } }
                }
            });
            
            // Network Chart
            const networkCtx = document.getElementById('networkChart').getContext('2d');
            networkChart = new Chart(networkCtx, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [
                        {
                            label: 'RX (KB)',
                            data: [],
                            borderColor: '#00cc66',
                            backgroundColor: 'transparent'
                        },
                        {
                            label: 'TX (KB)',
                            data: [],
                            borderColor: '#cc0066',
                            backgroundColor: 'transparent'
                        }
                    ]
                }
            });
        }
        
        // Fetch data from sysmon JSON file
        async function fetchData() {
            try {
                const response = await fetch('/sysmon/data.json');
                const data = await response.json();
                
                if (data.history && data.history.length > 0) {
                    updateDashboard(data);
                }
            } catch (error) {
                console.error('Error fetching data:', error);
            }
        }
        
        // Update dashboard with new data
        function updateDashboard(data) {
            // Get last 10 history entries
            const history = data.history.slice(-10);
            
            // Extract timestamps
            const timestamps = history.map(entry => {
                const ts = entry.timestamp;
                return ts.split(' ')[1]; // Return only the time part
            });
            
            // Update uptime
            const latest = history[history.length - 1];
            if (latest.uptime && latest.uptime.uptime_pretty) {
                document.getElementById('uptime').textContent = latest.uptime.uptime_pretty;
            }
            
            // Update CPU chart
            if (cpuChart) {
                cpuChart.data.labels = timestamps;
                cpuChart.data.datasets[0].data = history.map(entry => {
                    if (entry.cpu_usage && entry.cpu_usage.cpus && entry.cpu_usage.cpus.length > 0) {
                        // Calculate average CPU usage across all cores
                        const totalCores = entry.cpu_usage.cpus.length;
                        const totalUsage = entry.cpu_usage.cpus.reduce((sum, cpu) => sum + cpu.usage_percent, 0);
                        return totalUsage / totalCores;
                    }
                    return null;
                });
                cpuChart.update();
            }
            
            // Update Memory chart
            if (memoryChart) {
                memoryChart.data.labels = timestamps;
                memoryChart.data.datasets[0].data = history.map(entry => {
                    if (entry.memory) {
                        return entry.memory.usage_percent;
                    }
                    return null;
                });
                memoryChart.update();
            }
            
            // Update Network chart
            if (networkChart) {
                networkChart.data.labels = timestamps;
                
                // RX data
                networkChart.data.datasets[0].data = history.map(entry => {
                    if (entry.network && entry.network.interfaces && entry.network.interfaces.length > 0) {
                        // Sum RX bytes from all interfaces and convert to KB
                        return entry.network.interfaces.reduce((sum, iface) => sum + iface.rx_bytes, 0) / 1024;
                    }
                    return null;
                });
                
                // TX data
                networkChart.data.datasets[1].data = history.map(entry => {
                    if (entry.network && entry.network.interfaces && entry.network.interfaces.length > 0) {
                        // Sum TX bytes from all interfaces and convert to KB
                        return entry.network.interfaces.reduce((sum, iface) => sum + iface.tx_bytes, 0) / 1024;
                    }
                    return null;
                });
                
                networkChart.update();
            }
        }
        
        // Initialize charts and start data fetching
        document.addEventListener('DOMContentLoaded', () => {
            initCharts();
            fetchData();
            
            // Refresh data every 60 seconds
            setInterval(fetchData, 60000);
        });
    </script>
</body>
</html>
```

Create a simple script to copy the latest data to the web directory:

```bash
#!/bin/sh
# /usr/local/bin/update_sysmon_web.sh

# Copy the latest data to the web directory
cp /var/log/sysmon_data.json /www/sysmon/data.json

# Set proper permissions
chmod 644 /www/sysmon/data.json
```

Add this to cron to run every minute:
```
* * * * * /usr/local/bin/update_sysmon_web.sh
```

After setting up the web server (like uhttpd), you can access the dashboard at http://router-ip/sysmon/