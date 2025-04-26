# OpenWrt System Monitor Usage Guide

## Overview

The OpenWrt System Monitor (`sysmon`) is a lightweight, configurable utility that collects system resource information on OpenWrt-based systems. It stores this data in a structured JSON format that can be used for monitoring, analysis, or visualization.

## Quick Start

### Building the Application

1. Clone the repository or download the source code
2. Make the build script executable:
   ```
   chmod +x build.sh
   ```
3. Run the build script:
   ```
   ./build.sh
   ```

### Running the Application

#### Basic Usage
```
./sysmon
```
This runs the monitor with default settings, reading from `/etc/sysmon_config.json` if available.

#### Common Options
```
./sysmon -c config.json -v
```
This runs the monitor with a custom config file and verbose logging.

#### Run Once
```
./sysmon -x
```
This collects data once and exits immediately.

#### All Options
```
Usage: sysmon [options]
Options:
  -c <config_file>   Specify configuration file (default: /etc/sysmon_config.json)
  -o <output_file>   Specify output file (default: from config or /var/log/sysmon_data.json)
  -l <log_file>      Specify log file (default: from config or /var/log/sysmon.log)
  -i <interval>      Specify collection interval in seconds (default: from config or 60)
  -v                 Enable verbose output
  -x                 Run once and exit
  -h                 Display this help message
```

## Configuration File

The application uses a JSON configuration file. Here's the default configuration:

```json
{
  "output_path": "/var/log/sysmon_data.json",
  "log_path": "/var/log/sysmon.log",
  "collection_interval": 60,
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

You can customize this configuration to:
- Change the output file path
- Adjust the collection interval
- Enable/disable specific resource collection types
- Enable verbose logging

## Output Format

The application generates a JSON file with the following structure:

```json
{
  "history": [
    {
      "timestamp": "2025-04-24 23:03:21",
      "cpu_usage": { ... },
      "memory": { ... },
      "load": { ... },
      "disk": { ... },
      "network": { ... },
      "uptime": { ... },
      "processes": { ... },
      "swap": { ... }
    },
    ...
  ]
}
```

Each data collection run adds a new entry to the `history` array with a timestamp, ensuring that you have a time-series record of your system's performance.

## Resource Types

The application collects the following resource types:

1. **CPU Usage**: Per-core and overall CPU utilization
2. **Memory Usage**: Total, free, shared, and buffer memory statistics
3. **System Load**: 1, 5, and 15-minute load averages
4. **Disk Usage**: Filesystem capacity, used space, and availability
5. **Network Statistics**: Interface traffic, packets, and errors
6. **System Uptime**: Total uptime in seconds and human-readable format
7. **Process Information**: Total, running, blocked, and sleeping processes
8. **Swap Usage**: Total, used, and free swap space

## Logging

The application logs its activity to the configured log file. The log format is:

```
[YYYY-MM-DD HH:MM:SS] [LEVEL] Message
```

Log levels include:
- `ERROR`: Critical errors that prevent functionality
- `WARNING`: Non-critical issues that might affect operation
- `INFO`: General operational information
- `DEBUG`: Detailed information for troubleshooting (only with verbose mode)

## Use Cases

1. **System Health Monitoring**: Track resource usage over time to identify trends and potential issues
2. **Performance Benchmarking**: Compare resource usage before and after system changes
3. **Capacity Planning**: Monitor resource utilization to plan for upgrades
4. **Automated Alerting**: Use collected data to trigger alerts when resources reach critical levels
5. **Custom Dashboards**: Visualize the data with tools like the included web dashboard

## Troubleshooting

### Common Issues

1. **Configuration File Not Found**:
   ```
   [ERROR] Failed to open file /etc/sysmon_config.json: No such file or directory
   [WARNING] Could not read config file, using defaults
   ```
   Solution: Specify a configuration file with `-c` or create one at the default location.

2. **Output File Permission Denied**:
   ```
   [ERROR] Failed to open file /var/log/sysmon_data.json: Permission denied
   ```
   Solution: Run with appropriate permissions or specify an output file in a writable location.

3. **JSON Parse Error**:
   ```
   [ERROR] Failed to parse JSON file
   ```
   Solution: Ensure your configuration file is valid JSON.

### Getting Help

For more information, run the application with the `-h` flag:
```
./sysmon -h
```

## Examples

### Collect Data Every 5 Minutes

```
./sysmon -i 300
```

### Use Custom Configuration and Output Files

```
./sysmon -c /path/to/custom_config.json -o /path/to/output.json
```

### Collect Data Once for Testing

```
./sysmon -x -v
```

### Create a Minimal Configuration File

```json
{
  "output_path": "system_data.json",
  "collection_interval": 10,
  "collect_cpu": true,
  "collect_memory": true,
  "collect_load": false,
  "collect_disk": false,
  "collect_network": false,
  "collect_uptime": true,
  "collect_processes": false,
  "collect_swap": false
}
```

This configuration only collects CPU, memory, and uptime data every 10 seconds.