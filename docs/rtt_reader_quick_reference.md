# RTT Reader Quick Reference

## Basic Usage

### OpenOCD Backend

```bash
# Basic usage (assumes OpenOCD running on localhost:4444)
python3 scripts/rtt_reader.py --backend openocd

# Custom host/port
python3 scripts/rtt_reader.py --backend openocd --host 192.168.1.100 --port 4444

# Save to file
python3 scripts/rtt_reader.py --backend openocd --output rtt_output.txt

# Read from different channel
python3 scripts/rtt_reader.py --backend openocd --channel 1
```

### J-Link Backend

```bash
# Basic usage (default: STM32F205RB via SWD)
python3 scripts/rtt_reader.py --backend jlink

# Specific device
python3 scripts/rtt_reader.py --backend jlink --device STM32F407VG

# JTAG interface
python3 scripts/rtt_reader.py --backend jlink --device STM32F407VG --interface JTAG

# Custom speed
python3 scripts/rtt_reader.py --backend jlink --speed 8000

# Save to file
python3 scripts/rtt_reader.py --backend jlink --output rtt_output.txt
```

## OpenOCD Setup

### Starting OpenOCD with RTT Support

```bash
# For ST-Link
openocd -f interface/stlink.cfg -f target/stm32f2x.cfg

# For J-Link (via OpenOCD)
openocd -f interface/jlink.cfg -f target/stm32f2x.cfg

# With custom RTT configuration
openocd -f interface/stlink.cfg -f target/stm32f2x.cfg \
  -c "rtt setup 0x20000000 0x10000 \"SEGGER RTT\"" \
  -c "rtt start"
```

### RTT Configuration in OpenOCD

The RTT reader automatically configures RTT with:
- Base address: 0x20000000 (RAM start for STM32F2)
- Search range: 0x10000 (64KB)
- Control block ID: "SEGGER RTT"

For different targets, you may need to adjust the base address.

## J-Link Setup

### Installing pylink

```bash
# Install pylink library
pip install pylink-square

# Verify installation
python3 -c "import pylink; print(pylink.__version__)"
```

### Common Device Names

- STM32F205RB
- STM32F407VG
- STM32F429ZI
- STM32L476RG
- nRF52832_xxAA
- nRF52840_xxAA

Use the exact device name as specified in SEGGER's device list.

## Advanced Usage

### Adjusting Poll Interval

```bash
# Faster polling (more CPU usage)
python3 scripts/rtt_reader.py --backend openocd --poll-interval 0.001

# Slower polling (less CPU usage)
python3 scripts/rtt_reader.py --backend openocd --poll-interval 0.1
```

### Reading Multiple Channels

To read from multiple channels, run multiple instances:

```bash
# Terminal 1: Read channel 0
python3 scripts/rtt_reader.py --backend openocd --channel 0 --output ch0.txt

# Terminal 2: Read channel 1
python3 scripts/rtt_reader.py --backend openocd --channel 1 --output ch1.txt
```

### Combining with Other Tools

```bash
# Pipe output to grep
python3 scripts/rtt_reader.py --backend openocd | grep "ERROR"

# Pipe to tee for both display and save
python3 scripts/rtt_reader.py --backend openocd | tee rtt_output.txt

# Use with watch for continuous monitoring
watch -n 0.1 'cat rtt_output.txt | tail -20'
```

## Troubleshooting

### OpenOCD Connection Issues

**Problem:** "Failed to connect to OpenOCD"

**Solutions:**
1. Verify OpenOCD is running:
   ```bash
   telnet localhost 4444
   ```
2. Check if another process is using the port:
   ```bash
   netstat -an | grep 4444
   ```
3. Check OpenOCD logs for RTT initialization errors

### J-Link Connection Issues

**Problem:** "Failed to connect to J-Link"

**Solutions:**
1. Verify J-Link is connected:
   ```bash
   lsusb | grep "SEGGER"
   ```
2. Check J-Link permissions (Linux):
   ```bash
   sudo chmod 666 /dev/bus/usb/XXX/YYY
   ```
3. Try slower interface speed:
   ```bash
   python3 scripts/rtt_reader.py --backend jlink --speed 1000
   ```
4. Verify device name is correct

### No RTT Data

**Problem:** No output from RTT reader

**Solutions:**
1. Verify target is running (not halted)
2. Check that RTT is initialized in target firmware
3. Verify correct channel number
4. Try resetting the target device
5. Increase search range in OpenOCD (for OpenOCD backend)

### Data Loss or Corruption

**Problem:** Missing or garbled RTT data

**Solutions:**
1. Increase RTT buffer size in target firmware
2. Reduce polling interval:
   ```bash
   python3 scripts/rtt_reader.py --backend openocd --poll-interval 0.005
   ```
3. Use blocking mode in SEGGER RTT configuration
4. Reduce the amount of data being logged

## Performance Tips

### For High-Speed Logging

1. Use J-Link backend (faster than OpenOCD)
2. Reduce poll interval to 0.001 - 0.005 seconds
3. Increase RTT buffer size on target
4. Use binary channel (channel > 0) for non-text data
5. Consider using RTT channel 1 or 2 for high-speed data

### For Low-Power Debugging

1. Use OpenOCD backend
2. Increase poll interval to 0.1 - 1.0 seconds
3. Use lower interface speed
4. Minimize logging in target firmware

## Integration Examples

### With Automated Testing

```bash
#!/bin/bash
# Start RTT reader in background
python3 scripts/rtt_reader.py --backend openocd --output test_log.txt &
RTT_PID=$!

# Flash and run tests
# ... your flash/test commands ...

# Wait for tests to complete
sleep 30

# Stop RTT reader
kill $RTT_PID

# Analyze results
python3 scripts/rtt_analyzer.py test_log.txt --stats-only
```

### With Log Rotation

```bash
# Log with timestamp
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
python3 scripts/rtt_reader.py --backend openocd --output "rtt_log_${TIMESTAMP}.txt"

# Or use logrotate for continuous logging
python3 scripts/rtt_reader.py --backend openocd --output rtt_log.txt
# Configure logrotate to rotate rtt_log.txt
```

## See Also

- `docs/testing_deployment_guide.md` - Testing and deployment guide
- `docs/googletest_size_optimization.md` - GoogleTest size optimization
- `README.md` - General usage and building instructions
